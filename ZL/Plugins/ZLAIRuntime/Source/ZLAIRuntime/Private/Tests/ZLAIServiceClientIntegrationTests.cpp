#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "ZLAIServiceSubsystem.h"

namespace
{
	struct FDialogueCallState
	{
		bool bCompleted = false;
		bool bSucceeded = false;
		bool bCallbackOnGameThread = false;
		FString ExpectedRequestId;
		FZLDialogueResponse Response;
		FZLServiceError Error;
	};

	struct FDialogueIntegrationState
	{
		UGameInstance* GameInstance = nullptr;
		UZLAIServiceSubsystem* Subsystem = nullptr;
		TSharedRef<FDialogueCallState> SuccessCall = MakeShared<FDialogueCallState>();
		TSharedRef<FDialogueCallState> FailureCall = MakeShared<FDialogueCallState>();
		double DeadlineSeconds = 0.0;
	};
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FWaitForDialogueIntegrationCalls,
	TSharedRef<FDialogueIntegrationState>,
	State,
	FAutomationTestBase*,
	Test);

bool FWaitForDialogueIntegrationCalls::Update()
{
	const bool bTimedOut = FPlatformTime::Seconds() >= State->DeadlineSeconds;
	if ((!State->SuccessCall->bCompleted || !State->FailureCall->bCompleted) && !bTimedOut)
	{
		return false;
	}

	Test->TestTrue(TEXT("Success request completed before timeout"), State->SuccessCall->bCompleted);
	Test->TestTrue(TEXT("Valid request used the success delegate"), State->SuccessCall->bSucceeded);
	Test->TestTrue(TEXT("Success delegate ran on the Game Thread"), State->SuccessCall->bCallbackOnGameThread);
	Test->TestEqual(
		TEXT("Success response preserves request ID"),
		State->SuccessCall->Response.RequestId,
		State->SuccessCall->ExpectedRequestId);
	Test->TestEqual(
		TEXT("Success response preserves NPC ID"),
		State->SuccessCall->Response.NpcId,
		FString(TEXT("npc_guard_01")));
	Test->TestEqual(TEXT("Success response uses Stub provider"), State->SuccessCall->Response.Provider, FString(TEXT("stub")));
	Test->TestFalse(TEXT("Success response contains a reply"), State->SuccessCall->Response.Reply.IsEmpty());

	Test->TestTrue(TEXT("Invalid request completed before timeout"), State->FailureCall->bCompleted);
	Test->TestFalse(TEXT("Invalid request used the failure delegate"), State->FailureCall->bSucceeded);
	Test->TestTrue(TEXT("Failure delegate ran on the Game Thread"), State->FailureCall->bCallbackOnGameThread);
	Test->TestEqual(
		TEXT("Failure response preserves request ID"),
		State->FailureCall->Error.RequestId,
		State->FailureCall->ExpectedRequestId);
	Test->TestEqual(TEXT("Failure response exposes protocol error"), State->FailureCall->Error.Code, FString(TEXT("invalid_request")));
	Test->TestEqual(TEXT("Failure response exposes HTTP status"), State->FailureCall->Error.HttpStatusCode, 400);
	Test->TestNotEqual(
		TEXT("Concurrent requests receive unique IDs"),
		State->SuccessCall->ExpectedRequestId,
		State->FailureCall->ExpectedRequestId);

	if (State->Subsystem)
	{
		State->Subsystem->RemoveFromRoot();
		State->Subsystem = nullptr;
	}
	if (State->GameInstance)
	{
		State->GameInstance->RemoveFromRoot();
		State->GameInstance = nullptr;
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLAIServiceClientIntegrationTest,
	"ZLAIRuntime.Integration.ServiceClientCallbacks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FZLAIServiceClientIntegrationTest::RunTest(const FString& Parameters)
{
	TSharedRef<FDialogueIntegrationState> State = MakeShared<FDialogueIntegrationState>();
	State->GameInstance = NewObject<UGameInstance>();
	State->GameInstance->AddToRoot();
	State->Subsystem = NewObject<UZLAIServiceSubsystem>(State->GameInstance);
	State->Subsystem->AddToRoot();
	State->DeadlineSeconds = FPlatformTime::Seconds() + 10.0;

	State->SuccessCall->ExpectedRequestId = State->Subsystem->SendDialogueRequest(
		TEXT("http://127.0.0.1:8000"),
		TEXT("npc_guard_01"),
		TEXT("What happened here?"),
		FZLDialogueSuccessDelegate::CreateLambda([CallState = State->SuccessCall](const FZLDialogueResponse& Response)
		{
			CallState->bCompleted = true;
			CallState->bSucceeded = true;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Response = Response;
		}),
		FZLDialogueFailureDelegate::CreateLambda([CallState = State->SuccessCall](const FZLServiceError& Error)
		{
			CallState->bCompleted = true;
			CallState->bSucceeded = false;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Error = Error;
		}));

	State->FailureCall->ExpectedRequestId = State->Subsystem->SendDialogueRequest(
		TEXT("http://127.0.0.1:8000/"),
		TEXT("npc_guard_01"),
		TEXT(""),
		FZLDialogueSuccessDelegate::CreateLambda([CallState = State->FailureCall](const FZLDialogueResponse& Response)
		{
			CallState->bCompleted = true;
			CallState->bSucceeded = true;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Response = Response;
		}),
		FZLDialogueFailureDelegate::CreateLambda([CallState = State->FailureCall](const FZLServiceError& Error)
		{
			CallState->bCompleted = true;
			CallState->bSucceeded = false;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Error = Error;
		}));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDialogueIntegrationCalls(State, this));
	return true;
}

#endif
