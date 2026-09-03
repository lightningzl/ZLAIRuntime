"""Deterministic social-plan fallback for Decision v2."""

import json
from typing import Any

from app.planners.decision_v2 import DecisionV2GenerationContext, DecisionV2PlannerResult, DecisionV2StepResult


class StubDecisionV2Planner:
    def plan(self, context: DecisionV2GenerationContext) -> DecisionV2PlannerResult:
        data: dict[str, Any] = json.loads(context.context_data_json)
        npc = data["context"]["npc"]
        facts = data["context"].get("social_situation", [])
        capabilities = data["available_capabilities"]
        role = f"{data['npc_id']} {npc['role']}".lower()
        kinds = {item["kind"] for item in facts}

        def capability(*names: str) -> dict[str, Any] | None:
            return next((item for item in capabilities if item["capability_id"] in names or item["kind"] in names), None)

        harmed = "received_harm" in kinds
        repeated = sum(item["kind"] == "received_harm" for item in facts) >= 2
        apologized = "apology_received" in kinds
        if "merchant" in role:
            objective = "保护自己和生意，避免继续与攻击者互动" if harmed else "维持安全的交易环境"
            speech = "你刚刚又攻击了我。我不会和袭击我的人做生意。" if repeated else ("我听到了道歉，但现在还不能放心交易。" if apologized else "请保持礼貌，这里是做生意的地方。")
            choices = ("keep_distance_from_player", "refuse_trade", "seek_nearby_guard")
            tags = ["fear", "anger"] if harmed else ["wary"]
        elif "civilian" in role:
            objective = "确保自身安全并避免卷入冲突" if harmed else "避开可能的危险"
            speech = "别再靠近我！有人能帮帮我吗？" if harmed else "我不想卷入麻烦。"
            choices = ("keep_distance_from_player", "seek_nearby_guard")
            tags = ["fear"]
        elif "guard" in role:
            objective = "确认危险并恢复现场秩序"
            speech = "停止攻击，退后并说明情况。" if harmed or "report_confirmed" in kinds else "这里需要保持秩序。"
            choices = ("face_player", "keep_distance_from_player")
            tags = ["alert"]
        else:
            objective = "保护自己，同时判断是否应升级冲突"
            speech = "我记得刚才发生的事，别再试探我。" if harmed else "离我远一点。"
            choices = ("keep_distance_from_player", "face_player")
            tags = ["anger", "wary"]

        steps: list[DecisionV2StepResult] = []
        for name in choices:
            selected = capability(name)
            if selected is not None and len(steps) < 2:
                targets = selected["target_ids"]
                steps.append(DecisionV2StepResult(selected["capability_id"], targets[0] if targets else None))
        return DecisionV2PlannerResult(
            objective=objective,
            public_reason=None,
            attention_target_id=data["trigger"].get("source_id"),
            expression={"valence": -0.7 if harmed else -0.2, "arousal": 0.9 if harmed else 0.4, "dominance": -0.2, "tags": tags},
            steps=tuple(steps),
            speech_text=speech,
            speech_emotion="alarmed" if harmed else "guarded",
            confidence=0.85,
            provider="stub",
        )
