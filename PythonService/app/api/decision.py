"""Structured Decision HTTP route adapter."""

from fastapi import APIRouter, Request

from app.schemas.decision import DecisionRequest, DecisionResponse
from app.schemas.dialogue import ErrorResponse
from app.services.decision_service import DecisionService


router = APIRouter(prefix="/v1")


@router.post(
    "/decision",
    response_model=DecisionResponse,
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
def create_decision(
    decision_request: DecisionRequest,
    http_request: Request,
) -> DecisionResponse:
    """Adapt a validated HTTP request to the Decision Service."""

    http_request.state.request_id = decision_request.request_id
    http_request.state.npc_id = decision_request.npc_id
    http_request.state.endpoint = "decision"
    http_request.state.state_version = decision_request.state_version
    http_request.state.history_count = len(decision_request.context.recent_history)
    service: DecisionService = http_request.app.state.decision_service
    return service.build_response(decision_request)
