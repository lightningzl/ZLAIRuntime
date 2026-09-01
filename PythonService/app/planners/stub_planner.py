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
        history = data["context"]["recent_history"]
        trigger_text = " ".join(
            str(trigger.get(field, "")) for field in ("content", "summary")
        ).lower()
        history_text = " ".join(
            str(item.get("summary", "")) for item in history[-3:]
        ).lower()

        def select(name: str) -> dict[str, Any]:
            return next((tool for tool in allowed_tools if tool["name"] == name), allowed_tools[0])

        if "attack" in trigger_text:
            selected = select("move_away")
            intent = "engage"
            speech_text = "Stop attacking. Keep your distance."
            emotion = "alarmed"
        elif any(word in trigger_text for word in ("sorry", "apolog", "对不起", "道歉")):
            selected = select("stop")
            intent = "respond"
            speech_text = "I heard your apology. Keep your distance."
            emotion = "guarded"
        elif "attack" in history_text:
            selected = select("move_away")
            intent = "disengage"
            speech_text = "Do not come any closer."
            emotion = "wary"
        else:
            selected = select("move_away")
            intent = "disengage" if selected["name"] == "move_away" else "respond"
            speech_text = "I hear you. Keep some distance."
            emotion = "wary"

        target_ids = selected["target_ids"]
        return DecisionPlannerResult(
            intent=intent,
            speech_text=speech_text,
            speech_emotion=emotion,
            tool_name=selected["name"],
            tool_target_id=target_ids[0] if target_ids else None,
            confidence=0.8,
            provider="stub",
        )
