"""Version 1 dialogue protocol schemas."""

from typing import Literal

from pydantic import BaseModel, ConfigDict


class ProtocolModel(BaseModel):
    """Base model for strict protocol field validation."""

    model_config = ConfigDict(strict=True, extra="ignore")


class DialogueRequest(ProtocolModel):
    """Client request for one NPC dialogue turn."""

    request_id: str
    npc_id: str
    player_input: str


class DialogueResponse(ProtocolModel):
    """Successful deterministic dialogue response."""

    request_id: str
    npc_id: str
    reply: str
    provider: Literal["stub", "openai"]


class ErrorDetail(ProtocolModel):
    """Stable machine-readable error details."""

    code: Literal["invalid_request", "validation_error", "internal_error"]
    message: str


class ErrorResponse(ProtocolModel):
    """Error envelope returned for non-successful dialogue requests."""

    request_id: str
    error: ErrorDetail
