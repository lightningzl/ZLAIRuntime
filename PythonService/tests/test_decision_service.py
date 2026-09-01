import pytest

from app.planners.base import (
    DecisionGenerationContext,
    DecisionPlannerInvalidResponse,
    DecisionPlannerResult,
)
from app.planners.stub_planner import StubDecisionPlanner
from app.schemas.decision import DecisionRequest
from app.services.decision_service import DecisionService, InvalidDecisionRequest
from tests.decision_payloads import valid_decision_payload


class InvalidToolPlanner:
    def plan(self, _context: DecisionGenerationContext) -> DecisionPlannerResult:
        return DecisionPlannerResult(
            intent="engage",
            speech_text="I will teleport.",
            speech_emotion=None,
            tool_name="move_toward",
            tool_target_id="unknown_target",
            confidence=0.5,
            provider="stub",
        )


def test_stub_decision_service_returns_correlated_visible_action() -> None:
    request = DecisionRequest.model_validate(valid_decision_payload())

    response = DecisionService(StubDecisionPlanner()).build_response(request)

    assert response.request_id == request.request_id
    assert response.npc_id == request.npc_id
    assert response.state_version == request.state_version
    assert response.provider == "stub"
    assert response.speech is not None
    assert response.tool_call is not None
    assert response.tool_call.name == "move_away"
    assert response.tool_call.target_id == "player"


def test_decision_service_rejects_planner_tool_outside_ue_allowlist() -> None:
    request = DecisionRequest.model_validate(valid_decision_payload())

    with pytest.raises(DecisionPlannerInvalidResponse):
        DecisionService(InvalidToolPlanner()).build_response(request)


def test_decision_service_rejects_blank_untrusted_data_as_invalid_request() -> None:
    payload = valid_decision_payload()
    payload["trigger"]["content"] = "   "
    request = DecisionRequest.model_validate(payload)

    with pytest.raises(InvalidDecisionRequest) as captured:
        DecisionService(StubDecisionPlanner()).build_response(request)

    assert captured.value.request_id == "request-1"
    assert "trigger.content" in captured.value.message
