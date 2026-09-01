#if WITH_DEV_AUTOMATION_TESTS

#include "HAL/PlatformTime.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "ZLAIServiceSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLDecisionClientCompletionTest,
	"ZLAIRuntime.DecisionClient.CompletionAndStaleness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLDecisionClientCompletionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UZLAIServiceSubsystem* Subsystem = NewObject<UZLAIServiceSubsystem>(GameInstance);
	const FString ValidJson = TEXT(R"({
		"request_id":"request-1","npc_id":"npc_guard","state_version":12,
		"decision_id":"decision-1","intent":"disengage",
		"speech":{"text":"Keep your distance."},
		"tool_call":{"call_id":"tool-1","name":"move_away","target_id":"player"},
		"confidence":0.8,"provider":"stub"})");

	int32 SuccessCount = 0;
	int32 FailureCount = 0;
	FZLDecisionResponse CapturedResponse;
	FZLServiceError CapturedError;
	Subsystem->CompleteDecisionRequest(
		TEXT("request-1"),
		TEXT("npc_guard"),
		12,
		FPlatformTime::Seconds(),
		30000,
		true,
		false,
		200,
		ValidJson,
		FZLDecisionSuccessDelegate::CreateLambda([&](const FZLDecisionResponse& Response)
		{
			++SuccessCount;
			CapturedResponse = Response;
		}),
		FZLDecisionFailureDelegate::CreateLambda([&](const FZLServiceError& Error)
		{
			++FailureCount;
			CapturedError = Error;
		}));
	TestEqual(TEXT("Valid response completes once"), SuccessCount, 1);
	TestEqual(TEXT("Valid response has no failure"), FailureCount, 0);
	TestEqual(TEXT("Response remains correlated"), CapturedResponse.DecisionId, FString(TEXT("decision-1")));

	SuccessCount = 0;
	FailureCount = 0;
	Subsystem->CompleteDecisionRequest(
		TEXT("request-1"),
		TEXT("npc_guard"),
		12,
		FPlatformTime::Seconds() - 31.0,
		30000,
		true,
		false,
		200,
		ValidJson,
		FZLDecisionSuccessDelegate::CreateLambda([&](const FZLDecisionResponse&) { ++SuccessCount; }),
		FZLDecisionFailureDelegate::CreateLambda([&](const FZLServiceError& Error)
		{
			++FailureCount;
			CapturedError = Error;
		}));
	TestEqual(TEXT("Expired response has no success"), SuccessCount, 0);
	TestEqual(TEXT("Expired response fails once"), FailureCount, 1);
	TestEqual(TEXT("Expired response is classified"), CapturedError.Code, FString(TEXT("stale_response")));

	const FString MismatchedVersionJson = ValidJson.Replace(TEXT("\"state_version\":12"), TEXT("\"state_version\":13"));
	FailureCount = 0;
	Subsystem->CompleteDecisionRequest(
		TEXT("request-1"),
		TEXT("npc_guard"),
		12,
		FPlatformTime::Seconds(),
		30000,
		true,
		false,
		200,
		MismatchedVersionJson,
		FZLDecisionSuccessDelegate(),
		FZLDecisionFailureDelegate::CreateLambda([&](const FZLServiceError& Error)
		{
			++FailureCount;
			CapturedError = Error;
		}));
	TestEqual(TEXT("Mismatched version fails once"), FailureCount, 1);
	TestEqual(TEXT("Mismatched version is parse failure"), CapturedError.Code, FString(TEXT("parse_error")));
	return true;
}

#endif
