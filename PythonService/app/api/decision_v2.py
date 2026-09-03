"""Decision v2 HTTP route adapter."""

from fastapi import APIRouter, Request

from app.schemas.decision_v2 import DecisionV2Request, DecisionV2Response
from app.services.decision_v2_service import DecisionV2Service

router = APIRouter(prefix="/v2")


@router.post("/decision", response_model=DecisionV2Response)
def create_decision_v2(request_body: DecisionV2Request, request: Request) -> DecisionV2Response:
    request.state.request_id = request_body.request_id
    request.state.npc_id = request_body.npc_id
    request.state.endpoint = "decision_v2"
    request.state.state_version = request_body.state_version
    return request.app.state.decision_v2_service.build_response(request_body)
