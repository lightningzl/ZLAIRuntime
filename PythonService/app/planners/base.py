"""Supplier-neutral structured Decision Planner contract."""

from dataclasses import dataclass
from typing import Literal, Protocol

from app.providers.base import ProviderId


DecisionIntent = Literal["respond", "engage", "disengage", "hold"]
DecisionToolName = Literal["face_target", "move_toward", "move_away", "stop"]


@dataclass(frozen=True, slots=True)
class DecisionGenerationContext:
    """Deterministic supplier-neutral input assembled for one Decision."""

    system_instructions: str
    context_data_json: str


@dataclass(frozen=True, slots=True)
class DecisionPlannerResult:
    """Validated supplier-neutral Planner suggestion."""

    intent: DecisionIntent
    speech_text: str | None
    speech_emotion: str | None
    tool_name: DecisionToolName | None
    tool_target_id: str | None
    confidence: float
    provider: ProviderId


class DecisionPlannerInvalidResponse(ValueError):
    """Raised when Planner output cannot form a confirmed Decision response."""

    def __init__(self, provider: ProviderId, message: str) -> None:
        super().__init__(message)
        self.provider = provider
        self.message = message


class DecisionPlanner(Protocol):
    """Generate one non-streaming structured Decision suggestion."""

    def plan(self, context: DecisionGenerationContext) -> DecisionPlannerResult:
        """Return exactly one bounded Decision result."""
        ...
