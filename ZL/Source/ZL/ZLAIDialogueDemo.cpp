#include "ZL.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "ZLAIServiceSubsystem.h"

namespace
{
	void ShowDemoMessage(const FString& Message, const FColor Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.0f, Color, Message);
		}
	}

	void RunDialogueDemo(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() < 2)
		{
			const FString Usage = TEXT("Usage: ZL.AI.DialogueDemo <npc_id> <player_input>");
			UE_LOG(LogZL, Warning, TEXT("AI Dialogue Failed code=invalid_demo_arguments message=%s"), *Usage);
			ShowDemoMessage(Usage, FColor::Red);
			return;
		}

		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UZLAIServiceSubsystem* ServiceSubsystem = GameInstance
			? GameInstance->GetSubsystem<UZLAIServiceSubsystem>()
			: nullptr;
		if (!ServiceSubsystem)
		{
			const FString Message = TEXT("AI Dialogue Failed: no active Game Instance or AI Service Subsystem");
			UE_LOG(LogZL, Warning, TEXT("AI Dialogue Failed code=missing_subsystem"));
			ShowDemoMessage(Message, FColor::Red);
			return;
		}

		const FString NpcId = Args[0];
		FString PlayerInput = Args[1];
		for (int32 Index = 2; Index < Args.Num(); ++Index)
		{
			PlayerInput += TEXT(" ");
			PlayerInput += Args[Index];
		}
		const TWeakObjectPtr<UGameInstance> WeakGameInstance(GameInstance);
		const FString RequestId = ServiceSubsystem->SendDialogueRequest(
			NpcId,
			PlayerInput,
			FZLDialogueSuccessDelegate::CreateLambda([WeakGameInstance](const FZLDialogueResponse& Response)
			{
				if (!WeakGameInstance.IsValid())
				{
					return;
				}

				const FString ScreenMessage = FString::Printf(TEXT("AI Reply [%s]: %s"), *Response.NpcId, *Response.Reply);
				UE_LOG(
					LogZL,
					Display,
					TEXT("AI Dialogue Reply request_id=%s npc_id=%s provider=%s reply_length=%d"),
					*Response.RequestId,
					*Response.NpcId,
					*Response.Provider,
					Response.Reply.Len());
				ShowDemoMessage(ScreenMessage, FColor::Green);
			}),
			FZLDialogueFailureDelegate::CreateLambda([WeakGameInstance](const FZLServiceError& Error)
			{
				if (!WeakGameInstance.IsValid())
				{
					return;
				}

				const FString ScreenMessage = FString::Printf(TEXT("AI Dialogue Failed [%s]: %s"), *Error.Code, *Error.Message);
				UE_LOG(
					LogZL,
					Warning,
					TEXT("AI Dialogue Failed request_id=%s code=%s http_status=%d"),
					*Error.RequestId,
					*Error.Code,
					Error.HttpStatusCode);
				ShowDemoMessage(ScreenMessage, FColor::Red);
			}));

		UE_LOG(
			LogZL,
			Display,
			TEXT("AI Dialogue Request request_id=%s npc_id=%s has_context=false input_length=%d"),
			*RequestId,
			*NpcId,
			PlayerInput.Len());
		ShowDemoMessage(TEXT("AI Dialogue request sent"), FColor::Yellow);
	}

	FZLDialogueContext BuildDemoContext(const FString& Scenario)
	{
		FZLDialogueContext Context;
		Context.Npc.DisplayName = TEXT("Gate Keeper");
		Context.Npc.Role = TEXT("city gate guard");
		Context.Npc.Personality = Scenario == TEXT("persona")
			? TArray<FString>{TEXT("warm"), TEXT("talkative")}
			: TArray<FString>{TEXT("careful"), TEXT("formal")};
		Context.Npc.SpeakingStyle = Scenario == TEXT("persona")
			? TEXT("friendly and expressive")
			: TEXT("brief and official");
		Context.Npc.Goals = {TEXT("protect the city gate")};
		Context.World.Location = TEXT("north city gate");
		Context.World.Situation = Scenario == TEXT("world")
			? TEXT("the gate has reopened for escorted visitors")
			: TEXT("the gate is closed after an alarm");
		Context.World.Facts = Scenario == TEXT("world")
			? TArray<FString>{TEXT("an escort is waiting nearby")}
			: TArray<FString>{TEXT("entry currently requires authorization")};
		if (Scenario == TEXT("history"))
		{
			FZLDialogueHistoryMessage PlayerHistory;
			PlayerHistory.Role = TEXT("player");
			PlayerHistory.Content = TEXT("I helped the patrol earlier.");
			Context.DialogueHistory.Add(PlayerHistory);
			FZLDialogueHistoryMessage NpcHistory;
			NpcHistory.Role = TEXT("npc");
			NpcHistory.Content = TEXT("I remember your assistance.");
			Context.DialogueHistory.Add(NpcHistory);
		}
		return Context;
	}

	void RunDialogueContextDemo(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() < 3)
		{
			const FString Usage = TEXT(
				"Usage: ZL.AI.DialogueContextDemo <persona|world|history> <npc_id> <player_input>");
			UE_LOG(LogZL, Warning, TEXT("AI Context Dialogue Failed code=invalid_demo_arguments"));
			ShowDemoMessage(Usage, FColor::Red);
			return;
		}

		const FString Scenario = Args[0].ToLower();
		if (Scenario != TEXT("persona") && Scenario != TEXT("world") && Scenario != TEXT("history"))
		{
			UE_LOG(LogZL, Warning, TEXT("AI Context Dialogue Failed code=invalid_demo_scenario"));
			ShowDemoMessage(TEXT("Scenario must be persona, world, or history"), FColor::Red);
			return;
		}

		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UZLAIServiceSubsystem* ServiceSubsystem = GameInstance
			? GameInstance->GetSubsystem<UZLAIServiceSubsystem>()
			: nullptr;
		if (!ServiceSubsystem)
		{
			UE_LOG(LogZL, Warning, TEXT("AI Context Dialogue Failed code=missing_subsystem"));
			ShowDemoMessage(TEXT("AI Context Dialogue Failed: no active subsystem"), FColor::Red);
			return;
		}

		const FString NpcId = Args[1];
		FString PlayerInput = Args[2];
		for (int32 Index = 3; Index < Args.Num(); ++Index)
		{
			PlayerInput += TEXT(" ");
			PlayerInput += Args[Index];
		}

		const FZLDialogueContext Context = BuildDemoContext(Scenario);
		const TWeakObjectPtr<UGameInstance> WeakGameInstance(GameInstance);
		const FString RequestId = ServiceSubsystem->SendDialogueRequest(
			NpcId,
			PlayerInput,
			Context,
			FZLDialogueSuccessDelegate::CreateLambda([WeakGameInstance](const FZLDialogueResponse& Response)
			{
				if (!WeakGameInstance.IsValid())
				{
					return;
				}

				UE_LOG(
					LogZL,
					Display,
					TEXT("AI Context Dialogue Reply request_id=%s npc_id=%s provider=%s reply_length=%d"),
					*Response.RequestId,
					*Response.NpcId,
					*Response.Provider,
					Response.Reply.Len());
				ShowDemoMessage(
					FString::Printf(TEXT("AI Context Reply [%s]: %s"), *Response.NpcId, *Response.Reply),
					FColor::Green);
			}),
			FZLDialogueFailureDelegate::CreateLambda([WeakGameInstance](const FZLServiceError& Error)
			{
				if (!WeakGameInstance.IsValid())
				{
					return;
				}

				UE_LOG(
					LogZL,
					Warning,
					TEXT("AI Context Dialogue Failed request_id=%s code=%s http_status=%d"),
					*Error.RequestId,
					*Error.Code,
					Error.HttpStatusCode);
				ShowDemoMessage(
					FString::Printf(TEXT("AI Context Dialogue Failed [%s]"), *Error.Code),
					FColor::Red);
			}));

		UE_LOG(
			LogZL,
			Display,
			TEXT("AI Context Dialogue Request request_id=%s npc_id=%s scenario=%s "
				"has_context=true history_count=%d input_length=%d"),
			*RequestId,
			*NpcId,
			*Scenario,
			Context.DialogueHistory.Num(),
			PlayerInput.Len());
		ShowDemoMessage(TEXT("AI context dialogue request sent"), FColor::Yellow);
	}

	FAutoConsoleCommandWithWorldAndArgs DialogueDemoCommand(
		TEXT("ZL.AI.DialogueDemo"),
		TEXT("Send a demo AI dialogue request. Usage: ZL.AI.DialogueDemo <npc_id> <player_input>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RunDialogueDemo));

	FAutoConsoleCommandWithWorldAndArgs DialogueContextDemoCommand(
		TEXT("ZL.AI.DialogueContextDemo"),
		TEXT("Send a contextual demo request. Usage: ZL.AI.DialogueContextDemo "
			"<persona|world|history> <npc_id> <player_input>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RunDialogueContextDemo));
}
