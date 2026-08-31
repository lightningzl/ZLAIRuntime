#include "SocialSandbox/ZLSocialSandboxPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "SocialSandbox/ZLSocialSandboxGameMode.h"
#include "SocialSandbox/ZLSocialSandboxNpc.h"

void AZLSocialSandboxPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalPlayerController())
	{
		return;
	}

	SandboxWidget = CreateWidget<UZLSocialSandboxWidget>(this, UZLSocialSandboxWidget::StaticClass());
	if (SandboxWidget != nullptr)
	{
		SandboxWidget->SetSubmitHandler([this](const EZLSocialSandboxInputMode InputMode, const FName SpeechMode, const FName TargetId, const FString& Input)
		{
			return SubmitSandboxInput(InputMode, SpeechMode, TargetId, Input);
		});
		SandboxWidget->SetResetHandler([this]() { ResetSandbox(); });
		SandboxWidget->AddToPlayerScreen(20);
		RefreshSandboxTargets();
	}

	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void AZLSocialSandboxPlayerController::RefreshSandboxTargets()
{
	if (SandboxWidget == nullptr)
	{
		return;
	}
	const AZLSocialSandboxGameMode* GameMode = GetWorld() == nullptr ? nullptr : GetWorld()->GetAuthGameMode<AZLSocialSandboxGameMode>();
	if (GameMode == nullptr)
	{
		return;
	}
	TArray<FName> StableIds;
	TArray<FText> DisplayNames;
	for (const AZLSocialSandboxNpc* Npc : GameMode->GetSandboxNpcs())
	{
		if (IsValid(Npc))
		{
			StableIds.Add(Npc->GetStableId());
			DisplayNames.Add(Npc->GetDisplayName());
		}
	}
	SandboxWidget->SetTargets(StableIds, DisplayNames);
}

FText AZLSocialSandboxPlayerController::SubmitSandboxInput(const EZLSocialSandboxInputMode, const FName, const FName TargetId, const FString&)
{
	if (!TargetId.IsNone())
	{
		const AZLSocialSandboxGameMode* GameMode = GetWorld() == nullptr ? nullptr : GetWorld()->GetAuthGameMode<AZLSocialSandboxGameMode>();
		if (GameMode == nullptr || GameMode->FindSandboxNpc(TargetId) == nullptr)
		{
			return FText::FromString(TEXT("拒绝：所选目标已失效"));
		}
	}
	return FText::GetEmpty();
}

void AZLSocialSandboxPlayerController::ResetSandbox()
{
	if (AZLSocialSandboxGameMode* GameMode = GetWorld() == nullptr ? nullptr : GetWorld()->GetAuthGameMode<AZLSocialSandboxGameMode>())
	{
		GameMode->ResetSocialSandbox();
	}
}
