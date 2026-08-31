"""Version 1 structured Decision protocol schemas."""

from typing import Annotated, Literal

from pydantic import Field, model_validator

from app.schemas.dialogue import NpcContext, ProtocolModel, _bounded_text


BoundedId = Annotated[str, _bounded_text(128)]
DecisionSummary = Annotated[str, _bounded_text(256)]
DecisionContent = Annotated[str, _bounded_text(512)]
EmotionHint = Annotated[str, _bounded_text(64)]
DecisionKind = Literal["speech", "action_result"]
PerceptionChannel = Literal["direct", "visual", "auditory"]
ToolName = Literal["face_target", "move_toward", "move_away", "stop"]


class DecisionTrigger(ProtocolModel):
    """One UE-confirmed event perceived by the selected NPC."""

    event_id: BoundedId
    kind: DecisionKind
    source_id: BoundedId
    target_id: BoundedId | None = None
    channels: Annotated[list[PerceptionChannel], Field(min_length=1, max_length=3)]
    content: DecisionContent | None = None
    summary: DecisionSummary
    occurred_at_ms: Annotated[int, Field(ge=0)]

    @model_validator(mode="after")
    def validate_kind_and_channels(self) -> "DecisionTrigger":
        if len(set(self.channels)) != len(self.channels):
            raise ValueError("trigger.channels must not contain duplicates")
        if self.kind == "speech" and self.content is None:
            raise ValueError("speech trigger requires content")
        if self.kind == "action_result" and self.content is not None:
            raise ValueError("action_result trigger must not contain content")
        return self


class DecisionRelationship(ProtocolModel):
    """Bounded UE relationship summary for the selected NPC."""

    trust: Annotated[float, Field(ge=-1.0, le=1.0)]
    affinity: Annotated[float, Field(ge=-1.0, le=1.0)]
    fear: Annotated[float, Field(ge=0.0, le=1.0)]
    familiarity: Annotated[float, Field(ge=0.0, le=1.0)]


class DecisionInstantState(ProtocolModel):
    """Bounded UE-authoritative instantaneous state."""

    fear: Annotated[float, Field(ge=0.0, le=1.0)]
    anger: Annotated[float, Field(ge=0.0, le=1.0)]
    curiosity: Annotated[float, Field(ge=0.0, le=1.0)]
    alert: Annotated[float, Field(ge=0.0, le=1.0)]


class DecisionHistoryItem(ProtocolModel):
    """One bounded fact already perceived by this NPC."""

    kind: DecisionKind
    source_id: BoundedId
    target_id: BoundedId | None = None
    summary: DecisionSummary
    occurred_at_ms: Annotated[int, Field(ge=0)]


class DecisionContext(ProtocolModel):
    """Complete personal context snapshot supplied by UE."""

    npc: NpcContext
    relationship: DecisionRelationship
    instant_state: DecisionInstantState
    recent_history: Annotated[list[DecisionHistoryItem], Field(max_length=8)]


class AllowedTool(ProtocolModel):
    """One Tool and the target IDs UE allows the Planner to suggest."""

    name: ToolName
    target_ids: Annotated[list[BoundedId], Field(max_length=4)]

    @model_validator(mode="after")
    def validate_targets(self) -> "AllowedTool":
        if len(set(self.target_ids)) != len(self.target_ids):
            raise ValueError("allowed tool target_ids must not contain duplicates")
        if self.name == "stop" and self.target_ids:
            raise ValueError("stop must not contain target_ids")
        if self.name != "stop" and not self.target_ids:
            raise ValueError("targeted tools require at least one target_id")
        return self


class DecisionRequest(ProtocolModel):
    """Client request for one bounded NPC Decision."""

    request_id: BoundedId
    npc_id: BoundedId
    state_version: Annotated[int, Field(ge=0)]
    ttl_ms: Annotated[int, Field(ge=100, le=60_000)]
    trigger: DecisionTrigger
    context: DecisionContext
    allowed_tools: Annotated[list[AllowedTool], Field(min_length=1, max_length=4)]

    @model_validator(mode="after")
    def validate_unique_tools(self) -> "DecisionRequest":
        names = [tool.name for tool in self.allowed_tools]
        if len(set(names)) != len(names):
            raise ValueError("allowed_tools must not contain duplicate names")
        return self


class DecisionSpeech(ProtocolModel):
    """Speech that UE may display independently from Tool execution."""

    text: DecisionContent
    emotion: EmotionHint | None = None


class DecisionToolCall(ProtocolModel):
    """At most one bounded Tool suggestion."""

    call_id: BoundedId
    name: ToolName
    target_id: BoundedId | None = None

    @model_validator(mode="after")
    def validate_target(self) -> "DecisionToolCall":
        if self.name == "stop" and self.target_id is not None:
            raise ValueError("stop must not contain target_id")
        if self.name != "stop" and self.target_id is None:
            raise ValueError("targeted tool call requires target_id")
        return self


class DecisionResponse(ProtocolModel):
    """Successful structured Decision response."""

    request_id: BoundedId
    npc_id: BoundedId
    state_version: Annotated[int, Field(ge=0)]
    decision_id: BoundedId
    intent: Literal["respond", "engage", "disengage", "hold"]
    speech: DecisionSpeech | None = None
    tool_call: DecisionToolCall | None = None
    confidence: Annotated[float, Field(ge=0.0, le=1.0)]
    provider: Literal["stub", "kimi"]

    @model_validator(mode="after")
    def require_visible_or_actionable_result(self) -> "DecisionResponse":
        if self.speech is None and self.tool_call is None:
            raise ValueError("decision response requires speech or tool_call")
        return self
