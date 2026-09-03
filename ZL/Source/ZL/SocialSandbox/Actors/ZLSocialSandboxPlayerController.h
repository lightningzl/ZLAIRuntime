#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SocialSandbox/UI/ZLSocialSandboxWidget.h"
#include "ZLSocialSandboxPlayerController.generated.h"

class UZLSocialSandboxWidget;
class UInputMappingContext;

UCLASS()
class ZL_API AZLSocialSandboxPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	void RefreshSandboxTargets();
	void RefreshObservationInspector();
	void SelectInspectorTarget(FName TargetId);
	void AppendInteractionRecord(const FText& Text, const FLinearColor& Color);
	UZLSocialSandboxWidget* GetSandboxWidget() const { return SandboxWidget; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void ApplyInputMappingContext();
	void SelectSandboxNpcUnderCursor();
	FText SubmitSandboxInput(EZLSocialSandboxInputMode InputMode, FName SpeechMode, FName TargetId, const FString& Input);
	void ResetSandbox();

	UPROPERTY()
	TObjectPtr<UZLSocialSandboxWidget> SandboxWidget;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Input")
	TObjectPtr<UInputMappingContext> SandboxInputMappingContext;
};
