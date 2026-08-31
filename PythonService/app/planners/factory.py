"""Construct the explicitly configured Decision Planner."""

from app.core.settings import Settings
from app.planners.base import DecisionPlanner
from app.planners.kimi_planner import KimiDecisionPlanner
from app.planners.stub_planner import StubDecisionPlanner


def create_decision_planner(settings: Settings) -> DecisionPlanner:
    """Use the configured logical Provider without silent fallback."""

    if settings.dialogue_provider == "stub":
        return StubDecisionPlanner()
    return KimiDecisionPlanner(settings)
