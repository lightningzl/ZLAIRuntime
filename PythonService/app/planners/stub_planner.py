"""Deterministic offline Decision Planner."""

import json
from typing import Any

from app.planners.base import DecisionGenerationContext, DecisionPlannerResult


class StubDecisionPlanner:
    """Return a visible deterministic suggestion without external I/O."""

    def plan(self, context: DecisionGenerationContext) -> DecisionPlannerResult:
        data: dict[str, Any] = json.loads(context.context_data_json)
        allowed_tools = data["allowed_tools"]
        trigger = data["trigger"]
        npc_context = data["context"]["npc"]
        relationship = data["context"]["relationship"]
        history = data["context"]["recent_history"]
        npc_id = str(data["npc_id"])
        role = str(npc_context["role"]).lower()
        trigger_text = " ".join(
            str(trigger.get(field, "")) for field in ("content", "summary")
        ).lower()
        history_text = " ".join(
            str(item.get("summary", "")) for item in history[-3:]
        ).lower()

        def select(name: str) -> dict[str, Any]:
            return next((tool for tool in allowed_tools if tool["name"] == name), allowed_tools[0])

        if "merchant" in role or npc_id == "npc_merchant":
            voice = "merchant"
        elif "rival" in role or npc_id == "npc_rival":
            voice = "rival"
        elif "civilian" in role or npc_id == "npc_civilian":
            voice = "civilian"
        else:
            voice = "guard"

        lines = {
            "guard": {
                "attack": "立刻停止攻击，和我保持距离。",
                "apology": "我听到了你的道歉，但请保持距离。",
                "history": "不要再靠近了。",
                "default": "我听见了，请保持适当距离。",
            },
            "merchant": {
                "attack": "够了，暴力对这里所有人都没有好处。",
                "apology": "我接受你的道歉，但请保持和平。",
                "history": "等危险过去后，我们再谈。",
                "default": "我们还是和气些，麻烦会影响生意。",
            },
            "rival": {
                "attack": "还想靠拳头解决问题？退后。",
                "apology": "一句道歉抹不掉我们的旧账。",
                "history": "我没有忘记你刚才做的事。",
                "default": "你知道我们的旧账，别再试探我。",
            },
            "civilian": {
                "attack": "请住手！我不想卷入这件事。",
                "apology": "好吧，但请别把我牵扯进去。",
                "history": "等安全了再靠近我吧。",
                "default": "我听见了，但我更想避开麻烦。",
            },
        }

        if "attack" in trigger_text:
            selected = select("move_away")
            intent = "engage"
            speech_text = lines[voice]["attack"]
            emotion = "alarmed"
        elif any(word in trigger_text for word in ("sorry", "apolog", "对不起", "道歉")):
            selected = select("stop")
            intent = "respond"
            speech_text = lines[voice]["apology"]
            emotion = "guarded"
        elif "attack" in history_text:
            selected = select("move_away")
            intent = "disengage"
            speech_text = lines[voice]["history"]
            emotion = "wary"
        else:
            selected = select("move_away")
            intent = "disengage" if selected["name"] == "move_away" else "respond"
            speech_text = lines[voice]["default"]
            emotion = "wary"

        if voice == "rival" and float(relationship["trust"]) < -0.5:
            confidence = 0.9
        elif voice == "civilian":
            confidence = 0.75
        else:
            confidence = 0.8

        target_ids = selected["target_ids"]
        return DecisionPlannerResult(
            intent=intent,
            speech_text=speech_text,
            speech_emotion=emotion,
            tool_name=selected["name"],
            tool_target_id=target_ids[0] if target_ids else None,
            confidence=confidence,
            provider="stub",
        )
