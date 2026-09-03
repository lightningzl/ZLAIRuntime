"""Version 2 personal social-plan Decision protocol schemas."""

from typing import Annotated

from pydantic import Field, model_validator

from app.schemas.decision import (
    BoundedId,
    DecisionContent,
    DecisionContext,
    DecisionTrigger,
    EmotionHint,
)
from app.schemas.dialogue import ProtocolModel, _bounded_text


CapabilityId = Annotated[str, Field(min_length=1, max_length=64, pattern=r"^[a-z][a-z0-9_]{0,63}$")]
SituationKind = CapabilityId
PlanText = Annotated[str, _bounded_text(256)]
ExpressionTag = Annotated[str, _bounded_text(32)]


class SocialSituationFact(ProtocolModel):
    """One UE-authoritative fact personally available to this NPC."""

    kind: SituationKind
    subject_id: BoundedId
    target_id: BoundedId | None = None
    summary: Annotated[str, _bounded_text(256)]
    occurred_at_ms: Annotated[int, Field(ge=0)]
    salience: Annotated[float, Field(ge=0.0, le=1.0)]


class DecisionV2Context(DecisionContext):
    social_situation: Annotated[list[SocialSituationFact], Field(max_length=12)]


class AvailableCapability(ProtocolModel):
    capability_id: CapabilityId
    kind: Annotated[str, _bounded_text(64)]
    target_ids: Annotated[list[BoundedId], Field(max_length=4)]

    @model_validator(mode="after")
    def validate_unique_targets(self) -> "AvailableCapability":
        if len(set(self.target_ids)) != len(self.target_ids):
            raise ValueError("available capability target_ids must not contain duplicates")
        return self


class DecisionV2Request(ProtocolModel):
    request_id: BoundedId
    npc_id: BoundedId
    state_version: Annotated[int, Field(ge=0)]
    ttl_ms: Annotated[int, Field(ge=100, le=60_000)]
    trigger: DecisionTrigger
    context: DecisionV2Context
    available_capabilities: Annotated[list[AvailableCapability], Field(min_length=1, max_length=8)]

    @model_validator(mode="after")
    def validate_unique_capabilities(self) -> "DecisionV2Request":
        ids = [item.capability_id for item in self.available_capabilities]
        if len(set(ids)) != len(ids):
            raise ValueError("available_capabilities must not contain duplicate capability_id")
        return self


class PlanExpression(ProtocolModel):
    valence: Annotated[float, Field(ge=-1.0, le=1.0)]
    arousal: Annotated[float, Field(ge=-1.0, le=1.0)]
    dominance: Annotated[float, Field(ge=-1.0, le=1.0)]
    tags: Annotated[list[ExpressionTag], Field(max_length=3)]


class PlanStep(ProtocolModel):
    step_id: BoundedId
    capability_id: CapabilityId
    target_id: BoundedId | None = None


class DecisionPlan(ProtocolModel):
    objective: PlanText
    public_reason: PlanText | None = None
    attention_target_id: BoundedId | None = None
    expression: PlanExpression | None = None
    steps: Annotated[list[PlanStep], Field(max_length=4)]

    @model_validator(mode="after")
    def validate_unique_steps(self) -> "DecisionPlan":
        if len({step.step_id for step in self.steps}) != len(self.steps):
            raise ValueError("plan steps must not contain duplicate step_id")
        return self


class DecisionV2Speech(ProtocolModel):
    text: DecisionContent
    emotion: EmotionHint | None = None


class DecisionV2Response(ProtocolModel):
    request_id: BoundedId
    npc_id: BoundedId
    state_version: Annotated[int, Field(ge=0)]
    decision_id: BoundedId
    plan: DecisionPlan
    speech: DecisionV2Speech | None = None
    confidence: Annotated[float, Field(ge=0.0, le=1.0)]
    provider: str
