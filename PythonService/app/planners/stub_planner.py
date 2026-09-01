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
                "attack": "Stop attacking. Keep your distance.",
                "apology": "I heard your apology. Keep your distance.",
                "history": "Do not come any closer.",
                "default": "I hear you. Keep some distance.",
            },
            "merchant": {
                "attack": "Enough. Violence is bad for everyone here.",
                "apology": "Apology accepted, but keep this peaceful.",
                "history": "We can talk when the danger has passed.",
                "default": "Let us keep this civil; trouble is bad for business.",
            },
            "rival": {
                "attack": "Still settling things with your fists? Back off.",
                "apology": "One apology does not erase our history.",
                "history": "I have not forgotten what you just did.",
                "default": "You know our history. Do not test me again.",
            },
            "civilian": {
                "attack": "Please stop! I want no part in this.",
                "apology": "All right, but please leave me out of this.",
                "history": "Please stay away until things are safe.",
                "default": "I heard you, but I would rather avoid trouble.",
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
