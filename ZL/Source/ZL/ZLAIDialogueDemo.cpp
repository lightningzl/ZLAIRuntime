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
					TEXT("AI Dialogue Reply request_id=%s npc_id=%s reply=%s"),
					*Response.RequestId,
					*Response.NpcId,
					*Response.Reply);
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
					TEXT("AI Dialogue Failed request_id=%s code=%s http_status=%d message=%s"),
					*Error.RequestId,
					*Error.Code,
					Error.HttpStatusCode,
					*Error.Message);
				ShowDemoMessage(ScreenMessage, FColor::Red);
			}));

		UE_LOG(
			LogZL,
			Display,
			TEXT("AI Dialogue Request request_id=%s npc_id=%s player_input=%s"),
			*RequestId,
			*NpcId,
			*PlayerInput);
		ShowDemoMessage(TEXT("AI Dialogue request sent"), FColor::Yellow);
	}

	FAutoConsoleCommandWithWorldAndArgs DialogueDemoCommand(
		TEXT("ZL.AI.DialogueDemo"),
		TEXT("Send a demo AI dialogue request. Usage: ZL.AI.DialogueDemo <npc_id> <player_input>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RunDialogueDemo));
}
