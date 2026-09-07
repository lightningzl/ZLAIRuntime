#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZLAIServiceTypes.h"
#include "ZLSocialObservation.h"
#include "SocialSandbox/Decision/ZLSocialSandboxDecisionState.h"
#include "ZLSocialSandboxGameMode.generated.h"

class AZLSocialSandboxNpc;
class AZLSocialSandboxPawn;
class UZLSocialSandboxDecisionComponent;
class UZLSocialSandboxInteractionComponent;
class UZLSocialSandboxWorldKnowledgeSubsystem;
class AController;

UCLASS()
class ZL_API AZLSocialSandboxGameMode final : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZLSocialSandboxGameMode();

	UFUNCTION(Exec)
	void ResetSocialSandbox();

	UFUNCTION(Exec)
	void TriggerWorldEvent(FName EventType);
	UFUNCTION(Exec)
	void ReportWorldEvent(FName ReporterId, FName ReceiverId);
	UFUNCTION(Exec)
	void SpreadWorldRumor(FName ReporterId, FName ReceiverId);

	const TArray<TObjectPtr<AZLSocialSandboxNpc>>& GetSandboxNpcs() const;
	/** Registers a valid NPC that was placed through a scene spawner. Duplicate stable IDs are rejected. */
	bool RegisterSandboxNpc(AZLSocialSandboxNpc* Npc);
	AZLSocialSandboxNpc* FindSandboxNpc(FName StableId) const;
	/** 返回只读感知阈值，供场景协作系统执行同一套观察规则。 */
	const FZLSocialObservationSettings& GetObservationSettings() const;
	/** 返回决策协作组件；其内部状态仍由组件自行维护。 */
	UZLSocialSandboxDecisionComponent* GetDecisionComponent() const;
	/** 返回交互协作组件，供决策结果派发可见行为观察。 */
	UZLSocialSandboxInteractionComponent* GetInteractionComponent() const;
	FText SubmitSpeech(FName SpeechMode, FName TargetId, const FString& Text);
	FText SubmitAction(FName TargetId, const FString& Text);
	void ResolvePlayerAttackFromAnimNotify(AZLSocialSandboxPawn* Player, FName DamageSourceBone);
	/** 请求玩家控制器刷新个人观察 Inspector。 */
	void RefreshInspector() const;
	/** 向场景互动记录写入一条可见反馈。 */
	void AppendInteractionRecord(const FText& Text, const FLinearColor& Color) const;
	FText BuildInspectorText(FName NpcId) const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

private:

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Character Classes")
	TSubclassOf<AZLSocialSandboxPawn> SandboxPlayerClass;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Perception")
	FZLSocialObservationSettings ObservationSettings;
	UPROPERTY(VisibleAnywhere, Category="Sandbox|Systems")
	TObjectPtr<UZLSocialSandboxDecisionComponent> DecisionComponent;
	UPROPERTY(VisibleAnywhere, Category="Sandbox|Systems")
	TObjectPtr<UZLSocialSandboxInteractionComponent> InteractionComponent;
};
