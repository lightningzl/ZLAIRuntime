"""Version 1 dialogue protocol schemas."""

from typing import Annotated, Literal

from pydantic import AfterValidator, BaseModel, ConfigDict, Field, model_validator


def _bounded_text(max_length: int):
    def validate(value: str) -> str:
        stripped_value = value.strip()
        if stripped_value and len(stripped_value) > max_length:
            raise ValueError(f"text must contain at most {max_length} Unicode code points")
        return value

    return AfterValidator(validate)


DisplayName = Annotated[str, _bounded_text(64)]
NpcRole = Annotated[str, _bounded_text(128)]
PersonalityTrait = Annotated[str, _bounded_text(64)]
SpeakingStyle = Annotated[str, _bounded_text(256)]
NpcGoal = Annotated[str, _bounded_text(128)]
WorldLocation = Annotated[str, _bounded_text(128)]
WorldSituation = Annotated[str, _bounded_text(512)]
WorldFact = Annotated[str, _bounded_text(256)]
HistoryContent = Annotated[str, _bounded_text(512)]
MemoryScopeId = Annotated[str, _bounded_text(128)]


class ProtocolModel(BaseModel):
    """Base model for strict protocol field validation."""

    model_config = ConfigDict(strict=True, extra="ignore")


class NpcContext(ProtocolModel):
    """Author-provided NPC identity and behavior for this request."""

    display_name: DisplayName
    role: NpcRole
    personality: Annotated[list[PersonalityTrait], Field(min_length=1, max_length=8)]
    speaking_style: SpeakingStyle
    goals: Annotated[list[NpcGoal], Field(max_length=8)]


class WorldContext(ProtocolModel):
    """UE-confirmed world facts for this request."""

    location: WorldLocation
    situation: WorldSituation
    facts: Annotated[list[WorldFact], Field(max_length=16)]


class DialogueHistoryMessage(ProtocolModel):
    """One completed player or NPC turn ordered from oldest to newest."""

    role: Literal["player", "npc"]
    content: HistoryContent


class DialogueContext(ProtocolModel):
    """Complete transient context snapshot supplied by UE."""

    npc: NpcContext
    world: WorldContext
    dialogue_history: Annotated[list[DialogueHistoryMessage], Field(max_length=8)]


class DialogueMemory(ProtocolModel):
    """Explicit persistent-memory scope supplied by UE."""

    scope_id: MemoryScopeId


class DialogueRequest(ProtocolModel):
    """Client request for one NPC dialogue turn."""

    request_id: str
    npc_id: str
    player_input: str
    context: DialogueContext | None = None
    memory: DialogueMemory | None = None

    @model_validator(mode="before")
    @classmethod
    def reject_explicit_null_memory(cls, data: object) -> object:
        if isinstance(data, dict) and "memory" in data and data["memory"] is None:
            raise ValueError("memory must be an object when provided")
        return data


class DialogueResponse(ProtocolModel):
    """Successful deterministic dialogue response."""

    request_id: str
    npc_id: str
    reply: str
    provider: Literal["stub", "kimi"]


class ErrorDetail(ProtocolModel):
    """Stable machine-readable error details."""

    code: Literal[
        "invalid_request",
        "validation_error",
        "provider_auth_error",
        "provider_rate_limited",
        "provider_error",
        "planner_invalid_response",
        "provider_unavailable",
        "provider_timeout",
        "internal_error",
    ]
    message: str


class ErrorResponse(ProtocolModel):
    """Error envelope returned for non-successful dialogue requests."""

    request_id: str
    error: ErrorDetail
