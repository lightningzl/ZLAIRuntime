#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZLSocialObservation.h"
#include "SocialSandbox/Domain/ZLSocialSandboxCombatPresentation.h"
#include "SocialSandbox/Domain/ZLSocialSandboxNpcProfile.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"
#include "ZLSocialSandboxNpc.generated.h"

class UWidgetComponent;
class UAnimMontage;

struct FZLSocialSandboxDamageResult
{
	FName ReasonCode;
	float HealthBefore = 0.0f;
	float HealthAfter = 0.0f;
	float AppliedDamage = 0.0f;
	bool bAccepted = false;
	bool bDefended = false;
	bool bIncapacitated = false;
};

UCLASS()
class ZL_API AZLSocialSandboxNpc final : public ACharacter, public IZLSocialSandboxCombatPresentation, public ICombatDamageable
{
	GENERATED_BODY()

public:
	AZLSocialSandboxNpc();

	void InitializeSandboxNpc(FName InStableId, const FText& InDisplayName, const FTransform& InStartTransform);
	void InitializeSandboxNpc(const FZLSocialSandboxNpcProfile& InProfile, const FTransform& InStartTransform, float InInitialHealth = 100.0f);
	void ResetToSandboxStart();

	FName GetStableId() const { return StableId; }
	const FText& GetDisplayName() const { return DisplayName; }
	const FZLSocialSandboxNpcProfile& GetProfile() const { return Profile; }
	FVector GetPlanarForwardVector() const;
	void RecordObservation(const FZLSocialObservation& Observation) { ObservationBuffer.Add(Observation); }
	const FZLSocialObservation* GetLatestObservation() const { return ObservationBuffer.Latest(); }
	const TArray<FZLSocialObservation>& GetObservationItems() const { return ObservationBuffer.GetItems(); }
	void ClearObservations() { ObservationBuffer.Reset(); }
	int64 GetStateVersion() const { return StateVersion; }
	void AdvanceAuthorityStateVersion() { ++StateVersion; }
	bool IsDecisionActionActive() const { return bDecisionActionActive; }
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	bool IsDefending() const { return bDefending; }
	bool IsIncapacitated() const { return bIncapacitated; }
	bool ApplySandboxDamage(float RawDamage, double NowSeconds, FZLSocialSandboxDamageResult& OutResult);
	void SetDefending(bool bValue);
	bool StartDecisionAction(EZLSocialActionType Action, AActor* Target, TFunction<void()> OnCompleted);
	void StopDecisionAction();
	void ShowRuleSpeech(const FZLSocialObservation& Observation);
	void ShowActionObservation(const FZLSocialObservation& Observation);
	void ResetDecisionPresentation() { LastDecisionSpeech.Reset(); }
	void ShowDecisionSpeech(const FString& Text, const FString& Provider);
	void ShowDecisionAction(EZLSocialActionType Action, EZLSocialActionPhase Phase);
	void ShowDecisionRejection(FName ReasonCode);
	void ShowDecisionFallback();
	void ShowDamageResult(const FZLSocialSandboxDamageResult& Result);
	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
	virtual void HandleDeath() override;
	virtual void ApplyHealing(float Healing, AActor* Healer) override;
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;
	virtual void PlaySandboxAttackPresentation_Implementation(AActor* Target) override;

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UWidgetComponent> NameWidget;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UWidgetComponent> BubbleWidget;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Combat Presentation", meta=(ClampMin="0.1", ClampMax="3.0"))
	float MontagePlayRate = 1.0f;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	FName StableId;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	FText DisplayName;
	FZLSocialSandboxNpcProfile Profile;

	FTransform SandboxStartTransform;
	FZLSocialObservationBuffer ObservationBuffer;
	TWeakObjectPtr<AActor> DecisionTarget;
	TFunction<void()> DecisionCompletion;
	EZLSocialActionType DecisionAction = EZLSocialActionType::Stop;
	int64 StateVersion = 1;
	bool bDecisionActionActive = false;
	float DecisionSpeed = 240.0f;
	FString LastDecisionSpeech;
	float MaxHealth = 100.0f;
	float Health = 100.0f;
	bool bDefending = false;
	bool bIncapacitated = false;
	FTimerHandle BubbleTimer;

	void ShowBubble(const FText& Text, const FColor& Color, float DurationSeconds = 4.0f);
	void ClearBubble();
	void AdvanceDecisionAction(float DeltaSeconds);
	void RefreshNameLabel();

	friend class FZLSocialSandboxNpcDecisionActionTest;
};
