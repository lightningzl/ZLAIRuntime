#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Variant_Combat/Interfaces/CombatAttacker.h"
#include "ZLSocialObservation.h"
#include "SocialSandbox/Domain/ZLSocialSandboxCombatPresentation.h"
#include "ZLSocialSandboxPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWidgetComponent;
class UAnimMontage;
class UInputAction;
class AZLSocialSandboxNpc;
struct FInputActionValue;
struct FZLSocialSandboxPlayerPreset;

UCLASS()
class ZL_API AZLSocialSandboxPawn final : public ACharacter, public IZLSocialSandboxCombatPresentation, public ICombatAttacker
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
	bool StartSandboxComboAttack();
	bool StartSandboxChargedAttack();
	void ReleaseSandboxChargedAttack();
	bool ConsumePendingAttackHit();
	virtual void DoAttackTrace(FName DamageSourceBone) override;
	virtual void CheckCombo() override;
	virtual void CheckChargedAttack() override;

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
	void ComboAttackPressed();
	void ChargedAttackPressed();
	void ChargedAttackReleased();
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	bool PlayAttackMontage(UAnimMontage* Montage, FName Section);
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
	FOnMontageEnded OnAttackMontageEnded;
	bool bIsAttacking = false;
	bool bQueuedCombo = false;
	double CachedComboInputSeconds = -DBL_MAX;
	bool bChargingAttack = false;
	bool bChargeLoopReached = false;
	bool bPendingAttackHitConsumed = false;
	int32 ComboIndex = 0;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation", meta=(ClampMin="0.0", ClampMax="2.0"))
	float ComboInputCacheSeconds = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	TArray<FName> ComboSections;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	TObjectPtr<UAnimMontage> ChargedAttackMontage;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	FName ChargeLoopSection = NAME_None;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	FName ChargeAttackSection = NAME_None;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation", meta=(ClampMin="0.1", ClampMax="3.0"))
	float MontagePlayRate = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Input")
	TObjectPtr<UInputAction> MoveInputAction;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Input")
	TObjectPtr<UInputAction> LookInputAction;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Input")
	TObjectPtr<UInputAction> AttackInputAction;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Input")
	TObjectPtr<UInputAction> ChargedAttackInputAction;
};
