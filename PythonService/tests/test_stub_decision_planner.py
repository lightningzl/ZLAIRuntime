from app.planners.stub_planner import StubDecisionPlanner
from app.services.decision_context_builder import build_decision_generation_context
from app.schemas.decision import DecisionRequest
from tests.decision_payloads import valid_decision_payload


def _plan(payload: dict):
    return StubDecisionPlanner().plan(
        build_decision_generation_context(DecisionRequest.model_validate(payload))
    )


def test_stub_planner_escalates_from_observed_attack() -> None:
    payload = valid_decision_payload()
    payload["trigger"] = {
        "event_id": "hit-1",
        "kind": "action_result",
        "source_id": "player",
        "target_id": "npc_guard",
        "channels": ["visual", "direct"],
        "summary": "The player completed Attack toward npc_guard.",
        "occurred_at_ms": 1,
    }

    result = _plan(payload)

    assert result.intent == "engage"
    assert result.tool_name == "move_away"
    assert result.tool_target_id == "player"
    assert result.speech_emotion == "alarmed"


def test_stub_planner_deescalates_from_apology_with_stop_tool() -> None:
    payload = valid_decision_payload()
    payload["trigger"]["content"] = "I am sorry."

    result = _plan(payload)

    assert result.intent == "respond"
    assert result.tool_name == "stop"
    assert result.tool_target_id is None
    assert result.speech_emotion == "guarded"


def test_stub_planner_keeps_distance_after_recent_attack() -> None:
    payload = valid_decision_payload()
    payload["context"]["recent_history"] = [
        {
            "kind": "action_result",
            "source_id": "player",
            "target_id": "npc_guard",
            "summary": "The player completed Attack toward npc_guard.",
            "occurred_at_ms": 1,
        }
    ]

    result = _plan(payload)

    assert result.intent == "disengage"
    assert result.tool_name == "move_away"
