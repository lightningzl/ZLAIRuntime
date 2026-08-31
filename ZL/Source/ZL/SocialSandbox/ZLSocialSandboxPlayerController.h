#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZLSocialSandboxWidget.h"
#include "ZLSocialSandboxPlayerController.generated.h"

class UZLSocialSandboxWidget;

UCLASS()
class ZL_API AZLSocialSandboxPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	void RefreshSandboxTargets();
	UZLSocialSandboxWidget* GetSandboxWidget() const { return SandboxWidget; }

protected:
	virtual void BeginPlay() override;

private:
	FText SubmitSandboxInput(EZLSocialSandboxInputMode InputMode, FName SpeechMode, FName TargetId, const FString& Input);
	void ResetSandbox();

	UPROPERTY()
	TObjectPtr<UZLSocialSandboxWidget> SandboxWidget;
};
