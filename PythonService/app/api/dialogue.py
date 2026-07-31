"""Dialogue HTTP route adapter."""

from fastapi import APIRouter, Request

from app.schemas.dialogue import DialogueRequest, DialogueResponse, ErrorResponse
from app.services.dialogue_service import DialogueService


router = APIRouter(prefix="/v1")


@router.post(
    "/dialogue",
    response_model=DialogueResponse,
    responses={
        400: {"model": ErrorResponse},
        422: {"model": ErrorResponse},
        429: {"model": ErrorResponse},
        502: {"model": ErrorResponse},
        503: {"model": ErrorResponse},
        504: {"model": ErrorResponse},
        500: {"model": ErrorResponse},
    },
)
def create_dialogue(
    dialogue_request: DialogueRequest,
    http_request: Request,
) -> DialogueResponse:
    """Adapt a validated HTTP request to the dialogue service."""
    http_request.state.request_id = dialogue_request.request_id
    http_request.state.npc_id = dialogue_request.npc_id
    http_request.state.has_context = dialogue_request.context is not None
    http_request.state.has_memory = dialogue_request.memory is not None
    http_request.state.history_count = (
        len(dialogue_request.context.dialogue_history)
        if dialogue_request.context is not None
        else 0
    )
    service: DialogueService = http_request.app.state.dialogue_service
    return service.build_response(dialogue_request)
