"""Dialogue HTTP route adapter."""

from fastapi import APIRouter, Request

from app.schemas.dialogue import DialogueRequest, DialogueResponse, ErrorResponse
from app.services.dialogue_service import build_dialogue_response


router = APIRouter(prefix="/v1")


@router.post(
    "/dialogue",
    response_model=DialogueResponse,
    responses={
        400: {"model": ErrorResponse},
        422: {"model": ErrorResponse},
        500: {"model": ErrorResponse},
    },
)
def create_dialogue(
    dialogue_request: DialogueRequest,
    http_request: Request,
) -> DialogueResponse:
    """Adapt a validated HTTP request to the dialogue service."""
    http_request.state.request_id = dialogue_request.request_id
    return build_dialogue_response(dialogue_request)
