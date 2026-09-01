"""Deterministic offline Decision Planner."""

import json
from typing import Any

from app.planners.base import DecisionGenerationContext, DecisionPlannerResult


class StubDecisionPlanner:
    """Return a visible deterministic suggestion without external I/O."""

    def plan(self, context: DecisionGenerationContext) -> DecisionPlannerResult:
        data: dict[str, Any] = json.loads(context.context_data_json)
        allowed_tools = data["allowed_tools"]
        selected = next(
            (tool for tool in allowed_tools if tool["name"] == "move_away"),
            allowed_tools[0],
        )
        target_ids = selected["target_ids"]
        return DecisionPlannerResult(
            intent="disengage" if selected["name"] == "move_away" else "respond",
            speech_text="我听见了。先保持一点距离。",
            speech_emotion="wary",
            tool_name=selected["name"],
            tool_target_id=target_ids[0] if target_ids else None,
            confidence=0.8,
            provider="stub",
        )
