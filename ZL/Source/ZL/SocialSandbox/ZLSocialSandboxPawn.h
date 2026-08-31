#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZLSocialObservation.h"
#include "ZLSocialSandboxPawn.generated.h"

class UArrowComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UWidgetComponent;

UCLASS()
class ZL_API AZLSocialSandboxPawn final : public ACharacter
{
	GENERATED_BODY()

public:
	AZLSocialSandboxPawn();

	void ResetToSandboxStart();
	bool StartScriptedAction(EZLSocialActionType Action, AActor* Target, TFunction<void()> OnCompleted);
	void StopScriptedAction();
	bool IsScriptedActionActive() const { return bScriptedActionActive; }
	void ShowSpeechBubble(const FString& SpokenText);
	void ShowActionBubble(EZLSocialActionType Action, EZLSocialActionPhase Phase, const FText& TargetName);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void ShowBubble(const FText& Text, const FColor& Color, float DurationSeconds = 4.0f);
	void ClearBubble();
	void FaceLabelsToCamera() const;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UArrowComponent> FacingArrow;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UTextRenderComponent> NameLabel;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UWidgetComponent> BubbleWidget;

	FTransform SandboxStartTransform;
	TWeakObjectPtr<AActor> ScriptedTarget;
	TFunction<void()> ScriptedCompletion;
	EZLSocialActionType ScriptedAction = EZLSocialActionType::Stop;
	bool bScriptedActionActive = false;
	float ScriptedSpeed = 300.0f;
	FTimerHandle BubbleTimer;
};
