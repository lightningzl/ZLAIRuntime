#include "SocialSandbox/UI/ZLSocialSandboxInspectorPresenter.h"

#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/World/ZLSocialSandboxGameMode.h"
#include "ZLSocialKnowledge.h"

FText FZLSocialSandboxInspectorPresenter::Build(
	const AZLSocialSandboxNpc* Npc,
	const FZLSocialSandboxDecisionDebug* DecisionDebug,
	const TArray<FZLDecisionV2SocialFact>& SocialFacts,
	const TArray<FZLSocialKnowledgeItem>& Knowledge)
{
	if (Npc == nullptr)
	{
		return FText::FromString(TEXT("选择一个 NPC 查看个人感知。"));
	}
	const FZLSocialObservation* Observation = Npc->GetLatestObservation();
	if (Observation == nullptr)
	{
		return FText::FromString(FString::Printf(TEXT("%s\n尚无个人感知记录"), *Npc->GetDisplayName().ToString()));
	}
	auto YesNo = [](const bool Value) { return Value ? TEXT("是") : TEXT("否"); };
	auto SpeechModeText = [](const EZLSocialSpeechMode Mode)
	{
		switch (Mode) { case EZLSocialSpeechMode::Whisper: return TEXT("小声说话"); case EZLSocialSpeechMode::Shout: return TEXT("大声呼喊"); case EZLSocialSpeechMode::InEar: return TEXT("耳边说话"); default: return TEXT("正常说话"); }
	};
	auto TargetText = [](const EZLSocialTargetJudgment Value)
	{
		switch (Value) { case EZLSocialTargetJudgment::Candidate: return TEXT("可能指向自己"); case EZLSocialTargetJudgment::ExplicitSelf: return TEXT("明确指向自己"); case EZLSocialTargetJudgment::ExplicitOther: return TEXT("明确指向他人"); default: return TEXT("未确定"); }
	};
	auto FilterText = [](const EZLSocialObservationFilterReason Value)
	{
		switch (Value) { case EZLSocialObservationFilterReason::InvalidEvent: return TEXT("事件无效"); case EZLSocialObservationFilterReason::Expired: return TEXT("事件已过期"); case EZLSocialObservationFilterReason::CannotSee: return TEXT("无法看见"); case EZLSocialObservationFilterReason::CannotHear: return TEXT("无法听见"); case EZLSocialObservationFilterReason::OutsideVisualRange: return TEXT("超出视觉范围"); case EZLSocialObservationFilterReason::OutsideFieldOfView: return TEXT("不在视野内"); case EZLSocialObservationFilterReason::OutsideHearingRange: return TEXT("超出听觉范围"); case EZLSocialObservationFilterReason::NotExplicitInEarTarget: return TEXT("不是耳边说话目标"); default: return TEXT("无"); }
	};
	const FString Source = Observation->Source == EZLSocialObservationSource::Speech ? TEXT("说话") : TEXT("行为");
	auto ActionText = [](const EZLSocialActionType Value)
	{
		switch (Value) { case EZLSocialActionType::Face: return TEXT("面向"); case EZLSocialActionType::Approach: return TEXT("靠近"); case EZLSocialActionType::MoveAway: return TEXT("远离"); case EZLSocialActionType::Attack: return TEXT("攻击"); default: return TEXT("停止"); }
	};
	auto ToolText = [](const FString& Value)
	{
		if (Value == TEXT("face_target")) { return TEXT("面向目标"); }
		if (Value == TEXT("move_toward")) { return TEXT("靠近目标"); }
		if (Value == TEXT("move_away")) { return TEXT("远离目标"); }
		if (Value == TEXT("keep_distance_from_player")) { return TEXT("远离玩家"); }
		if (Value == TEXT("become_defensive")) { return TEXT("进入防御姿态"); }
		if (Value == TEXT("refuse_trade")) { return TEXT("设置交易拒绝"); }
		if (Value == TEXT("stop")) { return TEXT("停止"); }
		return TEXT("无");
	};
	auto ToolResultText = [](const FName Value)
	{
		if (Value == TEXT("Accepted")) { return TEXT("已接受"); }
		if (Value == TEXT("NoTool")) { return TEXT("未建议工具"); }
		if (Value == TEXT("ServiceUnavailable")) { return TEXT("服务不可用"); }
		if (Value == TEXT("StateVersionMismatch")) { return TEXT("状态已变化"); }
		return Value.IsNone() ? TEXT("无") : TEXT("未执行");
	};
	const FString SourceDetails = Observation->Source == EZLSocialObservationSource::Speech
		? FString::Printf(TEXT("说话方式：%s · 明确目标：%s\n目标判断：%s · 听觉过滤：%s"), SpeechModeText(Observation->SpeechMode), Observation->ExplicitTargetId.IsNone() ? TEXT("无") : *Observation->ExplicitTargetId.ToString(), TargetText(Observation->TargetJudgment), FilterText(Observation->AuditoryFilter))
		: FString::Printf(TEXT("行为：%s · 阶段：%s · 目标：%s\n目标判断：%s · 输入文本：无"), ActionText(Observation->Action), Observation->ActionPhase == EZLSocialActionPhase::Started ? TEXT("开始") : TEXT("完成"), Observation->ExplicitTargetId.IsNone() ? TEXT("无") : *Observation->ExplicitTargetId.ToString(), TargetText(Observation->TargetJudgment));
	FString KnowledgeLine;
	for (const FZLSocialKnowledgeItem& Item : Knowledge)
	{
		KnowledgeLine += FString::Printf(TEXT("\n- %s（来源 %s，可信度 %.2f，原因 %s）"), *Item.FactId.ToString(), *UEnum::GetValueAsString(Item.Source), Item.Confidence, *Item.UpdateReason);
	}
	if (KnowledgeLine.IsEmpty()) { KnowledgeLine = TEXT("\n- 未获知任何动态世界事实"); }
	FString FactsLine;
	for (const FZLDecisionV2SocialFact& Fact : SocialFacts)
	{
		FactsLine += FString::Printf(TEXT("\n- %s（%s → %s）"), *Fact.Kind, *Fact.SubjectId, Fact.TargetId.IsEmpty() ? TEXT("无") : *Fact.TargetId);
	}
	if (FactsLine.IsEmpty())
	{
		FactsLine = TEXT("\n- 无");
	}
	const FString DecisionLine = DecisionDebug != nullptr
		? FString::Printf(
			TEXT("\n个人社会事实：%s\n个人世界认知：%s\n冲突：%s · 生命 %.0f/%.0f · 防御：%s · 失能：%s\n决策：%s · 待处理：%s · 触发：%s · 本地降级：%s\n请求：%s · 状态：%lld · 合并：%d · 自动：%d\n来源：%s · 当前目标：%s · 台词：%s\n步骤：%s · 结果：%s · 延迟：%d 毫秒"),
			*FactsLine,
			*KnowledgeLine,
			DecisionDebug->ConflictLevel.IsEmpty() ? TEXT("平静") : *DecisionDebug->ConflictLevel,
			Npc->GetHealth(),
			Npc->GetMaxHealth(),
			Npc->IsDefending() ? TEXT("是") : TEXT("否"),
			Npc->IsIncapacitated() ? TEXT("是") : TEXT("否"),
			DecisionDebug->bInFlight ? TEXT("进行中") : TEXT("空闲"),
			DecisionDebug->bPending ? TEXT("是") : TEXT("否"),
			DecisionDebug->TriggerReason.IsNone() ? TEXT("无") : *DecisionDebug->TriggerReason.ToString(),
			DecisionDebug->bLocalFallback ? TEXT("是") : TEXT("否"),
			DecisionDebug->RequestId.IsEmpty() ? TEXT("无") : *DecisionDebug->RequestId,
			DecisionDebug->StateVersion,
			DecisionDebug->CoalescedTriggers,
			DecisionDebug->AutomaticReplans,
			DecisionDebug->Provider.IsEmpty() ? TEXT("无") : *DecisionDebug->Provider,
			DecisionDebug->Intent.IsEmpty() ? TEXT("无") : *DecisionDebug->Intent,
			DecisionDebug->bSpeechAccepted ? TEXT("已接受") : TEXT("无"),
			ToolText(DecisionDebug->ToolName),
			ToolResultText(DecisionDebug->ToolResult),
			DecisionDebug->LatencyMs)
		: FString();
	const TCHAR* FeedbackSource = DecisionDebug != nullptr
		? TEXT("反馈来源：结构化决策")
		: TEXT("反馈来源：规则占位");
	return FText::FromString(FString::Printf(
		TEXT("%s\n来源：%s · 距离：%.0f 厘米\n看见：%s · 视觉过滤：%s\n听见：%s · 听清：%s · 强度：%.2f\n%s\n输入来源：UE 事件 · %s%s"),
		*Npc->GetDisplayName().ToString(), *Source, Observation->Distance,
		YesNo(Observation->bSaw), FilterText(Observation->VisualFilter), YesNo(Observation->bHeard), YesNo(Observation->bHeardClearly), Observation->HearingStrength,
		*SourceDetails, FeedbackSource, *DecisionLine));
}
