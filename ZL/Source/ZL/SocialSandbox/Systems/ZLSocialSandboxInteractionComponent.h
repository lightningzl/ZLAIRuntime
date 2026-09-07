#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZLSocialObservation.h"
#include "ZLSocialSandboxInteractionComponent.generated.h"

/** 交互组件：维护场景感知变化，并在阈值跨越时触发 NPC 重规划。 */
UCLASS(ClassGroup=(SocialSandbox), meta=(BlueprintSpawnableComponent))
class ZL_API UZLSocialSandboxInteractionComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 不启用组件 Tick，由 GameMode 统一调度。 */
	UZLSocialSandboxInteractionComponent();
	/** 更新玩家与各 NPC 的距离分段，生成必要感知与决策事件。 */
	void UpdateNpcDistanceBands();

	/** 处理玩家语音并分发给能够感知到语音的 NPC。 */
	FText SubmitSpeech(FName SpeechMode, FName TargetId, const FString& Text);
	/** 处理玩家的受控行为或交易请求。 */
	FText SubmitAction(FName TargetId, const FString& Text);
	/** 处理攻击动画通知触发的命中、伤害与后续感知。 */
	void ResolvePlayerAttackFromAnimNotify(class AZLSocialSandboxPawn* Player, FName DamageSourceBone);
	/** 分发 NPC 已执行行为，让其他 NPC 获得可见行动观察。 */
	FZLSocialObservation DispatchNpcActionObservation(class AZLSocialSandboxNpc* Actor, EZLSocialActionType Action, EZLSocialActionPhase Phase, FName TargetId);

private:
	class AZLSocialSandboxGameMode* GetSandboxGameMode() const;
	FText SubmitTradeAttempt(FName TargetId);
	void DispatchActionObservation(EZLSocialActionType Action, EZLSocialActionPhase Phase, FName TargetId);
};
