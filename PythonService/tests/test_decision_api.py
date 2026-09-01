from fastapi.testclient import TestClient

from app.main import create_app
from app.planners.base import DecisionGenerationContext, DecisionPlannerResult
from app.planners.stub_planner import StubDecisionPlanner
from app.providers.stub_provider import StubDialogueProvider
from tests.decision_payloads import valid_decision_payload


class InvalidDecisionPlanner:
    def plan(self, _context: DecisionGenerationContext) -> DecisionPlannerResult:
        return DecisionPlannerResult(
            intent="engage",
            speech_text=None,
            speech_emotion=None,
            tool_name="face_target",
            tool_target_id="not_allowed",
            confidence=0.5,
            provider="stub",
        )


def test_decision_endpoint_returns_structured_stub_response() -> None:
    app = create_app(
        provider=StubDialogueProvider(),
        decision_planner=StubDecisionPlanner(),
    )
    with TestClient(app) as client:
        response = client.post("/v1/decision", json=valid_decision_payload())

    assert response.status_code == 200
    payload = response.json()
    assert payload["request_id"] == "request-1"
    assert payload["state_version"] == 12
    assert payload["provider"] == "stub"
    assert payload["tool_call"]["name"] == "move_away"
    assert "call_id" in payload["tool_call"]
    assert "decision_id" in payload


def test_decision_endpoint_maps_blank_business_data_to_400() -> None:
    payload = valid_decision_payload()
    payload["trigger"]["summary"] = "  "
    app = create_app(
        provider=StubDialogueProvider(),
        decision_planner=StubDecisionPlanner(),
    )
    with TestClient(app) as client:
        response = client.post("/v1/decision", json=payload)

    assert response.status_code == 400
    assert response.json() == {
        "request_id": "request-1",
        "error": {
            "code": "invalid_request",
            "message": "trigger.summary must not be blank",
        },
    }


def test_decision_endpoint_maps_disallowed_planner_result_to_502() -> None:
    app = create_app(
        provider=StubDialogueProvider(),
        decision_planner=InvalidDecisionPlanner(),
    )
    with TestClient(app) as client:
        response = client.post("/v1/decision", json=valid_decision_payload())

    assert response.status_code == 502
    assert response.json()["error"]["code"] == "planner_invalid_response"
    assert response.json()["request_id"] == "request-1"
