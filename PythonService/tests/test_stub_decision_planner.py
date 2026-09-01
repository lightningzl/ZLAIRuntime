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


def test_stub_planner_produces_bounded_persona_differences() -> None:
    payloads = []
    profiles = [
        ("npc_guard", "order-focused town guard", ["cautious"], -0.1),
        ("npc_merchant", "pragmatic market merchant", ["sociable"], 0.15),
        ("npc_rival", "proud rival with a prior grievance", ["resentful"], -0.65),
        ("npc_civilian", "uninvolved local civilian", ["conflict-averse"], 0.0),
    ]
    for npc_id, role, personality, trust in profiles:
        payload = valid_decision_payload()
        payload["npc_id"] = npc_id
        payload["trigger"]["target_id"] = npc_id
        payload["context"]["npc"]["role"] = role
        payload["context"]["npc"]["personality"] = personality
        payload["context"]["relationship"]["trust"] = trust
        payloads.append(payload)

    results = [_plan(payload) for payload in payloads]

    assert len({result.speech_text for result in results}) == 4
    assert all(result.tool_name in {"move_away", "stop"} for result in results)
    assert results[2].confidence > results[3].confidence
