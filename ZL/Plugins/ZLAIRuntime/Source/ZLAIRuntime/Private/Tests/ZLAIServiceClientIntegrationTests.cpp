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
		int32 CallbackCount = 0;
		FString ExpectedRequestId;
		FZLDialogueResponse Response;
		FZLServiceError Error;
	};

	struct FDialogueIntegrationState
	{
		UGameInstance* GameInstance = nullptr;
		UZLAIServiceSubsystem* Subsystem = nullptr;
		TSharedRef<FDialogueCallState> SuccessCall = MakeShared<FDialogueCallState>();
		TSharedRef<FDialogueCallState> ContextCall = MakeShared<FDialogueCallState>();
		TSharedRef<FDialogueCallState> FailureCall = MakeShared<FDialogueCallState>();
		TSharedRef<FDialogueCallState> InvalidContextCall = MakeShared<FDialogueCallState>();
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
	if ((!State->SuccessCall->bCompleted
		|| !State->ContextCall->bCompleted
		|| !State->FailureCall->bCompleted
		|| !State->InvalidContextCall->bCompleted)
		&& !bTimedOut)
	{
		return false;
	}

	Test->TestTrue(TEXT("Success request completed before timeout"), State->SuccessCall->bCompleted);
	Test->TestEqual(TEXT("Success request completes exactly once"), State->SuccessCall->CallbackCount, 1);
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

	Test->TestTrue(TEXT("Context request completed before timeout"), State->ContextCall->bCompleted);
	Test->TestEqual(TEXT("Context request completes exactly once"), State->ContextCall->CallbackCount, 1);
	Test->TestTrue(TEXT("Context request used the success delegate"), State->ContextCall->bSucceeded);
	Test->TestTrue(TEXT("Context success delegate ran on the Game Thread"), State->ContextCall->bCallbackOnGameThread);
	Test->TestEqual(
		TEXT("Context response preserves request ID"),
		State->ContextCall->Response.RequestId,
		State->ContextCall->ExpectedRequestId);
	Test->TestEqual(
		TEXT("Context response preserves NPC ID"),
		State->ContextCall->Response.NpcId,
		FString(TEXT("npc_context_01")));
	Test->TestEqual(TEXT("Context response uses Stub provider"), State->ContextCall->Response.Provider, FString(TEXT("stub")));
	Test->TestFalse(TEXT("Context response contains a reply"), State->ContextCall->Response.Reply.IsEmpty());

	Test->TestTrue(TEXT("Invalid request completed before timeout"), State->FailureCall->bCompleted);
	Test->TestEqual(TEXT("Invalid request completes exactly once"), State->FailureCall->CallbackCount, 1);
	Test->TestFalse(TEXT("Invalid request used the failure delegate"), State->FailureCall->bSucceeded);
	Test->TestTrue(TEXT("Failure delegate ran on the Game Thread"), State->FailureCall->bCallbackOnGameThread);
	Test->TestEqual(
		TEXT("Failure response is categorized as HTTP"),
		State->FailureCall->Error.Category,
		EZLServiceErrorCategory::Http);
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

	Test->TestTrue(TEXT("Invalid context completed before timeout"), State->InvalidContextCall->bCompleted);
	Test->TestEqual(TEXT("Invalid context completes exactly once"), State->InvalidContextCall->CallbackCount, 1);
	Test->TestFalse(TEXT("Invalid context never uses success"), State->InvalidContextCall->bSucceeded);
	Test->TestTrue(TEXT("Invalid context callback ran on the Game Thread"), State->InvalidContextCall->bCallbackOnGameThread);
	Test->TestEqual(
		TEXT("Invalid context fails locally as a client error"),
		State->InvalidContextCall->Error.Category,
		EZLServiceErrorCategory::Client);
	Test->TestEqual(
		TEXT("Invalid context has no HTTP status because no request was sent"),
		State->InvalidContextCall->Error.HttpStatusCode,
		0);
	Test->TestEqual(
		TEXT("Invalid context preserves request ID"),
		State->InvalidContextCall->Error.RequestId,
		State->InvalidContextCall->ExpectedRequestId);

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
		TEXT("npc_guard_01"),
		TEXT("What happened here?"),
		FZLDialogueSuccessDelegate::CreateLambda([CallState = State->SuccessCall](const FZLDialogueResponse& Response)
		{
			++CallState->CallbackCount;
			CallState->bCompleted = true;
			CallState->bSucceeded = true;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Response = Response;
		}),
		FZLDialogueFailureDelegate::CreateLambda([CallState = State->SuccessCall](const FZLServiceError& Error)
		{
			++CallState->CallbackCount;
			CallState->bCompleted = true;
			CallState->bSucceeded = false;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Error = Error;
		}));

	FZLDialogueContext Context;
	Context.Npc.DisplayName = TEXT("Gate Guard");
	Context.Npc.Role = TEXT("guard");
	Context.Npc.Personality = {TEXT("careful")};
	Context.Npc.SpeakingStyle = TEXT("brief");
	Context.Npc.Goals = {TEXT("protect the gate")};
	Context.World.Location = TEXT("north gate");
	Context.World.Situation = TEXT("the gate is closed");
	Context.World.Facts = {TEXT("the alarm sounded")};
	FZLDialogueHistoryMessage PlayerHistory;
	PlayerHistory.Role = TEXT("player");
	PlayerHistory.Content = TEXT("Why is the gate closed?");
	Context.DialogueHistory.Add(PlayerHistory);
	FZLDialogueHistoryMessage NpcHistory;
	NpcHistory.Role = TEXT("npc");
	NpcHistory.Content = TEXT("An alarm was raised.");
	Context.DialogueHistory.Add(NpcHistory);

	State->ContextCall->ExpectedRequestId = State->Subsystem->SendDialogueRequest(
		TEXT("npc_context_01"),
		TEXT("Can I enter?"),
		Context,
		FZLDialogueSuccessDelegate::CreateLambda([CallState = State->ContextCall](const FZLDialogueResponse& Response)
		{
			++CallState->CallbackCount;
			CallState->bCompleted = true;
			CallState->bSucceeded = true;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Response = Response;
		}),
		FZLDialogueFailureDelegate::CreateLambda([CallState = State->ContextCall](const FZLServiceError& Error)
		{
			++CallState->CallbackCount;
			CallState->bCompleted = true;
			CallState->bSucceeded = false;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Error = Error;
		}));

	State->FailureCall->ExpectedRequestId = State->Subsystem->SendDialogueRequest(
		TEXT("npc_guard_01"),
		TEXT(""),
		FZLDialogueSuccessDelegate::CreateLambda([CallState = State->FailureCall](const FZLDialogueResponse& Response)
		{
			++CallState->CallbackCount;
			CallState->bCompleted = true;
			CallState->bSucceeded = true;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Response = Response;
		}),
		FZLDialogueFailureDelegate::CreateLambda([CallState = State->FailureCall](const FZLServiceError& Error)
		{
			++CallState->CallbackCount;
			CallState->bCompleted = true;
			CallState->bSucceeded = false;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Error = Error;
		}));

	FZLDialogueContext InvalidContext = Context;
	InvalidContext.Npc.Personality.Reset();
	State->InvalidContextCall->ExpectedRequestId = State->Subsystem->SendDialogueRequest(
		TEXT("npc_invalid_context"),
		TEXT("This must not be sent"),
		InvalidContext,
		FZLDialogueSuccessDelegate::CreateLambda([CallState = State->InvalidContextCall](const FZLDialogueResponse& Response)
		{
			++CallState->CallbackCount;
			CallState->bCompleted = true;
			CallState->bSucceeded = true;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Response = Response;
		}),
		FZLDialogueFailureDelegate::CreateLambda([CallState = State->InvalidContextCall](const FZLServiceError& Error)
		{
			++CallState->CallbackCount;
			CallState->bCompleted = true;
			CallState->bSucceeded = false;
			CallState->bCallbackOnGameThread = IsInGameThread();
			CallState->Error = Error;
		}));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDialogueIntegrationCalls(State, this));
	return true;
}

#endif
