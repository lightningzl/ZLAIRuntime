#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZLSocialObservation.h"
#include "SocialSandbox/Domain/ZLSocialSandboxCombatPresentation.h"
#include "ZLSocialSandboxPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWidgetComponent;
class UAnimMontage;
class UInputAction;
struct FInputActionValue;
struct FZLSocialSandboxPlayerPreset;

UCLASS()
class ZL_API AZLSocialSandboxPawn final : public ACharacter, public IZLSocialSandboxCombatPresentation
{
	GENERATED_BODY()

public:
	AZLSocialSandboxPawn();
	void InitializeSandboxPlayer(const FZLSocialSandboxPlayerPreset& Preset);
	FName GetSandboxStableId() const { return SandboxStableId; }
	const FText& GetSandboxDisplayName() const { return SandboxDisplayName; }

	void ResetToSandboxStart();
	bool StartScriptedAction(EZLSocialActionType Action, AActor* Target, TFunction<void()> OnCompleted);
	void StopScriptedAction();
	bool IsScriptedActionActive() const { return bScriptedActionActive; }
	void ShowSpeechBubble(const FString& SpokenText);
	void ShowActionBubble(EZLSocialActionType Action, EZLSocialActionPhase Phase, const FText& TargetName);
	virtual void PlaySandboxAttackPresentation_Implementation(AActor* Target) override;
	virtual void PlaySandboxHitPresentation_Implementation(AActor* Instigator, float AppliedDamage, bool bIncapacitated) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void MoveFromInput(const FInputActionValue& Value);
	void LookFromInput(const FInputActionValue& Value);
	void ConfiguredAttackPressed();
	void ShowBubble(const FText& Text, const FColor& Color, float DurationSeconds = 4.0f);
	void ClearBubble();

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UWidgetComponent> NameWidget;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UWidgetComponent> BubbleWidget;

	FTransform SandboxStartTransform;
	FName SandboxStableId = TEXT("player");
	FText SandboxDisplayName = FText::FromString(TEXT("玩家"));
	TWeakObjectPtr<AActor> ScriptedTarget;
	TFunction<void()> ScriptedCompletion;
	EZLSocialActionType ScriptedAction = EZLSocialActionType::Stop;
	bool bScriptedActionActive = false;
	float ScriptedSpeed = 300.0f;
	FTimerHandle BubbleTimer;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	FName AttackMontageSection = NAME_None;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	TObjectPtr<UAnimMontage> HitMontage;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	FName HitMontageSection = NAME_None;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation", meta=(ClampMin="0.1", ClampMax="3.0"))
	float MontagePlayRate = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Input")
	TObjectPtr<UInputAction> MoveInputAction;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Input")
	TObjectPtr<UInputAction> LookInputAction;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Input")
	TObjectPtr<UInputAction> AttackInputAction;
};
