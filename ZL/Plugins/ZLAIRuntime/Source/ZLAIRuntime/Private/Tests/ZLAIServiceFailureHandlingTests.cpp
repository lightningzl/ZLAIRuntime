#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "ZLAIServiceSettings.h"
#include "ZLAIServiceSubsystem.h"

namespace
{
	struct FFailureCompletionResult
	{
		bool bFailureCalled = false;
		bool bSuccessCalled = false;
		int32 FailureCallCount = 0;
		int32 SuccessCallCount = 0;
		FZLServiceError Error;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLAIServiceSettingsTest,
	"ZLAIRuntime.Configuration.DefaultSettings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLAIServiceSettingsTest::RunTest(const FString& Parameters)
{
	const UZLAIServiceSettings* Settings = GetDefault<UZLAIServiceSettings>();
	TestEqual(TEXT("Default Service URL comes from Game config"), Settings->ServiceBaseUrl, FString(TEXT("http://127.0.0.1:8000")));
	TestEqual(TEXT("Default request timeout comes from Game config"), Settings->RequestTimeoutSeconds, 30.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLAIServiceFailureHandlingTest,
	"ZLAIRuntime.Failures.Classification",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLAIServiceFailureHandlingTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UZLAIServiceSubsystem* Subsystem = NewObject<UZLAIServiceSubsystem>(GameInstance);

	auto CompleteFailure = [Subsystem](
		const FString& RequestId,
		const bool bTransportSucceeded,
		const bool bTimedOut,
		const int32 HttpStatusCode,
		const FString& ResponseBody)
	{
		FFailureCompletionResult Result;
		Subsystem->CompleteRequest(
			RequestId,
			TEXT("npc_guard_01"),
			bTransportSucceeded,
			bTimedOut,
			HttpStatusCode,
			ResponseBody,
			FZLDialogueSuccessDelegate::CreateLambda([&Result](const FZLDialogueResponse&)
			{
				Result.bSuccessCalled = true;
				++Result.SuccessCallCount;
			}),
			FZLDialogueFailureDelegate::CreateLambda([&Result](const FZLServiceError& Error)
			{
				Result.Error = Error;
				Result.bFailureCalled = true;
				++Result.FailureCallCount;
			}));
		return Result;
	};

	const FFailureCompletionResult NetworkResult = CompleteFailure(TEXT("network-request"), false, false, 0, FString());
	TestTrue(TEXT("Network failure delegate is called"), NetworkResult.bFailureCalled);
	TestFalse(TEXT("Network failure does not call success"), NetworkResult.bSuccessCalled);
	TestEqual(TEXT("Service unavailable is a network failure"), NetworkResult.Error.Category, EZLServiceErrorCategory::Network);
	TestEqual(TEXT("Network failure has a stable code"), NetworkResult.Error.Code, FString(TEXT("network_error")));

	const FFailureCompletionResult TimeoutResult = CompleteFailure(TEXT("timeout-request"), false, true, 0, FString());
	TestTrue(TEXT("Timeout failure delegate is called"), TimeoutResult.bFailureCalled);
	TestFalse(TEXT("Timeout does not call success"), TimeoutResult.bSuccessCalled);
	TestEqual(TEXT("Timed out request has a distinct category"), TimeoutResult.Error.Category, EZLServiceErrorCategory::Timeout);
	TestEqual(TEXT("Timeout has a stable code"), TimeoutResult.Error.Code, FString(TEXT("timeout")));

	const FFailureCompletionResult HttpResult = CompleteFailure(
		TEXT("http-request"),
		true,
		false,
		400,
		TEXT("{\"request_id\":\"http-request\",\"error\":{\"code\":\"invalid_request\",\"message\":\"invalid input\"}}"));
	TestTrue(TEXT("HTTP failure delegate is called"), HttpResult.bFailureCalled);
	TestFalse(TEXT("HTTP failure does not call success"), HttpResult.bSuccessCalled);
	TestEqual(TEXT("Non-2xx response is an HTTP failure"), HttpResult.Error.Category, EZLServiceErrorCategory::Http);
	TestEqual(TEXT("HTTP failure preserves the Service code"), HttpResult.Error.Code, FString(TEXT("invalid_request")));
	TestEqual(TEXT("HTTP failure includes its status"), HttpResult.Error.HttpStatusCode, 400);

	auto TestProviderHttpFailure = [this, &CompleteFailure](
		const int32 HttpStatusCode,
		const TCHAR* ErrorCode)
	{
		const FString RequestId = FString::Printf(TEXT("provider-http-%d"), HttpStatusCode);
		const FString ResponseBody = FString::Printf(
			TEXT("{\"request_id\":\"%s\",\"error\":{\"code\":\"%s\",\"message\":\"provider failure\"}}"),
			*RequestId,
			ErrorCode);
		const FFailureCompletionResult Result = CompleteFailure(
			RequestId,
			true,
			false,
			HttpStatusCode,
			ResponseBody);

		TestEqual(TEXT("Provider HTTP failure calls failure exactly once"), Result.FailureCallCount, 1);
		TestEqual(TEXT("Provider HTTP failure never calls success"), Result.SuccessCallCount, 0);
		TestEqual(TEXT("Provider HTTP failure remains in HTTP category"), Result.Error.Category, EZLServiceErrorCategory::Http);
		TestEqual(TEXT("Provider HTTP failure preserves status"), Result.Error.HttpStatusCode, HttpStatusCode);
		TestEqual(TEXT("Provider HTTP failure preserves protocol code"), Result.Error.Code, FString(ErrorCode));
	};

	TestProviderHttpFailure(429, TEXT("provider_rate_limited"));
	TestProviderHttpFailure(502, TEXT("provider_error"));
	TestProviderHttpFailure(503, TEXT("provider_unavailable"));
	TestProviderHttpFailure(504, TEXT("provider_timeout"));

	const FFailureCompletionResult ParseResult = CompleteFailure(TEXT("parse-request"), true, false, 200, TEXT("not-json"));
	TestTrue(TEXT("Parse failure delegate is called"), ParseResult.bFailureCalled);
	TestFalse(TEXT("Parse failure does not call success"), ParseResult.bSuccessCalled);
	TestEqual(TEXT("Invalid success body is a parse failure"), ParseResult.Error.Category, EZLServiceErrorCategory::Parse);
	TestEqual(TEXT("Parse failure has a stable code"), ParseResult.Error.Code, FString(TEXT("parse_error")));
	TestEqual(TEXT("Parse failure includes its HTTP status"), ParseResult.Error.HttpStatusCode, 200);

	return true;
}

#endif
