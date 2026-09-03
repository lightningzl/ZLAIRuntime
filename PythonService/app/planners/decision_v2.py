"""Supplier-neutral Decision v2 Planner contract."""

from dataclasses import dataclass
from typing import Protocol

from app.providers.base import ProviderId


@dataclass(frozen=True, slots=True)
class DecisionV2GenerationContext:
    system_instructions: str
    context_data_json: str


@dataclass(frozen=True, slots=True)
class DecisionV2StepResult:
    capability_id: str
    target_id: str | None = None


@dataclass(frozen=True, slots=True)
class DecisionV2PlannerResult:
    objective: str
    public_reason: str | None
    attention_target_id: str | None
    expression: dict[str, object] | None
    steps: tuple[DecisionV2StepResult, ...]
    speech_text: str | None
    speech_emotion: str | None
    confidence: float
    provider: ProviderId


class DecisionV2Planner(Protocol):
    def plan(self, context: DecisionV2GenerationContext) -> DecisionV2PlannerResult: ...
