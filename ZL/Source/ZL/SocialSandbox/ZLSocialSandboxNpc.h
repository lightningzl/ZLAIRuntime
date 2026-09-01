#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZLSocialObservation.h"
#include "ZLSocialSandboxNpc.generated.h"

class UArrowComponent;
class UCapsuleComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UWidgetComponent;

UCLASS()
class ZL_API AZLSocialSandboxNpc final : public AActor
{
	GENERATED_BODY()

public:
	AZLSocialSandboxNpc();

	void InitializeSandboxNpc(FName InStableId, const FText& InDisplayName, const FTransform& InStartTransform);
	void ResetToSandboxStart();

	FName GetStableId() const { return StableId; }
	const FText& GetDisplayName() const { return DisplayName; }
	FVector GetPlanarForwardVector() const;
	void RecordObservation(const FZLSocialObservation& Observation) { ObservationBuffer.Add(Observation); }
	const FZLSocialObservation* GetLatestObservation() const { return ObservationBuffer.Latest(); }
	const TArray<FZLSocialObservation>& GetObservationItems() const { return ObservationBuffer.GetItems(); }
	void ClearObservations() { ObservationBuffer.Reset(); }
	int64 GetStateVersion() const { return StateVersion; }
	bool IsDecisionActionActive() const { return bDecisionActionActive; }
	bool StartDecisionAction(EZLSocialActionType Action, AActor* Target, TFunction<void()> OnCompleted);
	void StopDecisionAction();
	void ShowRuleSpeech(const FZLSocialObservation& Observation);
	void ShowActionObservation(const FZLSocialObservation& Observation);
	void ResetDecisionPresentation() { LastDecisionSpeech.Reset(); }
	void ShowDecisionSpeech(const FString& Text, const FString& Provider);
	void ShowDecisionAction(EZLSocialActionType Action, EZLSocialActionPhase Phase);
	void ShowDecisionRejection(FName ReasonCode);
	void ShowDecisionFallback();

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UArrowComponent> FacingArrow;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UTextRenderComponent> NameLabel;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UWidgetComponent> BubbleWidget;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	FName StableId;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	FText DisplayName;

	FTransform SandboxStartTransform;
	FZLSocialObservationBuffer ObservationBuffer;
	TWeakObjectPtr<AActor> DecisionTarget;
	TFunction<void()> DecisionCompletion;
	EZLSocialActionType DecisionAction = EZLSocialActionType::Stop;
	int64 StateVersion = 1;
	bool bDecisionActionActive = false;
	float DecisionSpeed = 240.0f;
	FString LastDecisionSpeech;
	FTimerHandle BubbleTimer;

	void ShowBubble(const FText& Text, const FColor& Color, float DurationSeconds = 4.0f);
	void ClearBubble();
	void FaceLabelsToCamera() const;
	void AdvanceDecisionAction(float DeltaSeconds);

	friend class FZLSocialSandboxNpcDecisionActionTest;
};
