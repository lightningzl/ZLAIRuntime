from fastapi.testclient import TestClient

from app.main import create_app
from app.planners.base import DecisionGenerationContext, DecisionPlannerResult
from app.planners.stub_planner import StubDecisionPlanner
from app.providers.stub_provider import StubDialogueProvider
from tests.decision_payloads import valid_decision_payload


def valid_decision_v2_payload() -> dict:
    return {
        "request_id": "request-v2-1", "npc_id": "npc_merchant", "state_version": 12, "ttl_ms": 30000,
        "trigger": {"event_id": "event-v2-1", "kind": "action_result", "source_id": "player", "target_id": "npc_merchant", "channels": ["direct", "visual"], "summary": "confirmed harm", "occurred_at_ms": 2},
        "context": {"npc": {"display_name": "Merchant", "role": "merchant", "personality": ["careful"], "speaking_style": "guarded", "goals": ["stay safe"]}, "relationship": {"trust": -0.5, "affinity": -0.2, "fear": 0.7, "familiarity": 0.3}, "instant_state": {"fear": 0.8, "anger": 0.5, "curiosity": 0.1, "alert": 1.0}, "recent_history": [], "social_situation": [{"kind": "received_harm", "subject_id": "player", "target_id": "npc_merchant", "summary": "confirmed harm", "occurred_at_ms": 2, "salience": 1.0}]},
        "available_capabilities": [{"capability_id": "keep_distance_from_player", "kind": "move_away", "target_ids": ["player"]}],
    }


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


def test_decision_v2_endpoint_initializes_and_returns_a_social_plan() -> None:
    app = create_app(provider=StubDialogueProvider(), decision_planner=StubDecisionPlanner())
    with TestClient(app) as client:
        response = client.post("/v2/decision", json=valid_decision_v2_payload())

    assert response.status_code == 200
    payload = response.json()
    assert payload["request_id"] == "request-v2-1"
    assert payload["provider"] == "stub"
    assert payload["plan"]["steps"][0]["capability_id"] == "keep_distance_from_player"


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
