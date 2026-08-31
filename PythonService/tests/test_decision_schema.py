from copy import deepcopy

import pytest
from pydantic import ValidationError

from app.schemas.decision import DecisionRequest, DecisionResponse


def valid_request() -> dict:
    return {
        "request_id": "request-1",
        "npc_id": "npc_guard",
        "state_version": 12,
        "ttl_ms": 30_000,
        "trigger": {
            "event_id": "speech-42",
            "kind": "speech",
            "source_id": "player",
            "target_id": "npc_guard",
            "channels": ["auditory", "direct"],
            "content": "Please keep your distance.",
            "summary": "The player spoke to the guard.",
            "occurred_at_ms": 1_725_100_800_000,
        },
        "context": {
            "npc": {
                "display_name": "Guard",
                "role": "gate guard",
                "personality": ["cautious"],
                "speaking_style": "brief",
                "goals": ["keep order"],
            },
            "relationship": {
                "trust": -0.1,
                "affinity": 0.0,
                "fear": 0.2,
                "familiarity": 0.4,
            },
            "instant_state": {
                "fear": 0.2,
                "anger": 0.3,
                "curiosity": 0.1,
                "alert": 0.8,
            },
            "recent_history": [
                {
                    "kind": "action_result",
                    "source_id": "player",
                    "target_id": "npc_guard",
                    "summary": "The player stopped approaching.",
                    "occurred_at_ms": 1_725_100_795_000,
                }
            ],
        },
        "allowed_tools": [
            {"name": "face_target", "target_ids": ["player"]},
            {"name": "move_toward", "target_ids": ["player"]},
            {"name": "move_away", "target_ids": ["player"]},
            {"name": "stop", "target_ids": []},
        ],
    }


def valid_response() -> dict:
    return {
        "request_id": "request-1",
        "npc_id": "npc_guard",
        "state_version": 12,
        "decision_id": "decision-1",
        "intent": "disengage",
        "speech": {"text": "Keep your distance.", "emotion": "wary"},
        "tool_call": {
            "call_id": "tool-1",
            "name": "move_away",
            "target_id": "player",
        },
        "confidence": 0.82,
        "provider": "stub",
    }


def test_decision_request_accepts_confirmed_contract_and_ignores_unknown_fields() -> None:
    payload = valid_request()
    payload["future_optional"] = "ignored"

    parsed = DecisionRequest.model_validate(payload)

    assert parsed.npc_id == "npc_guard"
    assert parsed.trigger.channels == ["auditory", "direct"]
    assert [tool.name for tool in parsed.allowed_tools] == [
        "face_target",
        "move_toward",
        "move_away",
        "stop",
    ]


@pytest.mark.parametrize("ttl_ms", [99, 60_001])
def test_decision_request_rejects_ttl_outside_bounds(ttl_ms: int) -> None:
    payload = valid_request()
    payload["ttl_ms"] = ttl_ms

    with pytest.raises(ValidationError):
        DecisionRequest.model_validate(payload)


def test_decision_request_rejects_duplicate_channels_and_tools() -> None:
    duplicate_channels = valid_request()
    duplicate_channels["trigger"]["channels"] = ["auditory", "auditory"]
    with pytest.raises(ValidationError):
        DecisionRequest.model_validate(duplicate_channels)

    duplicate_tools = valid_request()
    duplicate_tools["allowed_tools"][1]["name"] = "face_target"
    with pytest.raises(ValidationError):
        DecisionRequest.model_validate(duplicate_tools)


def test_decision_request_keeps_speech_and_action_result_semantics_separate() -> None:
    missing_speech_content = valid_request()
    del missing_speech_content["trigger"]["content"]
    with pytest.raises(ValidationError):
        DecisionRequest.model_validate(missing_speech_content)

    action_with_input = valid_request()
    action_with_input["trigger"]["kind"] = "action_result"
    with pytest.raises(ValidationError):
        DecisionRequest.model_validate(action_with_input)


@pytest.mark.parametrize(
    ("tool_name", "target_ids"),
    [("stop", ["player"]), ("move_toward", [])],
)
def test_decision_request_rejects_invalid_allowed_tool_targets(
    tool_name: str,
    target_ids: list[str],
) -> None:
    payload = valid_request()
    payload["allowed_tools"] = [{"name": tool_name, "target_ids": target_ids}]

    with pytest.raises(ValidationError):
        DecisionRequest.model_validate(payload)


def test_decision_response_accepts_speech_and_one_tool() -> None:
    parsed = DecisionResponse.model_validate(valid_response())

    assert parsed.speech is not None
    assert parsed.speech.text == "Keep your distance."
    assert parsed.tool_call is not None
    assert parsed.tool_call.name == "move_away"


def test_decision_response_allows_speech_when_tool_is_absent() -> None:
    payload = valid_response()
    del payload["tool_call"]

    parsed = DecisionResponse.model_validate(payload)

    assert parsed.speech is not None
    assert parsed.tool_call is None


def test_decision_response_rejects_empty_result() -> None:
    payload = valid_response()
    del payload["speech"]
    del payload["tool_call"]

    with pytest.raises(ValidationError):
        DecisionResponse.model_validate(payload)


@pytest.mark.parametrize(
    ("name", "target_id"),
    [("stop", "player"), ("move_away", None)],
)
def test_decision_response_rejects_invalid_tool_target(
    name: str,
    target_id: str | None,
) -> None:
    payload = deepcopy(valid_response())
    payload["tool_call"]["name"] = name
    payload["tool_call"]["target_id"] = target_id

    with pytest.raises(ValidationError):
        DecisionResponse.model_validate(payload)


@pytest.mark.parametrize("confidence", [-0.01, 1.01])
def test_decision_response_rejects_confidence_outside_bounds(confidence: float) -> None:
    payload = valid_response()
    payload["confidence"] = confidence

    with pytest.raises(ValidationError):
        DecisionResponse.model_validate(payload)
