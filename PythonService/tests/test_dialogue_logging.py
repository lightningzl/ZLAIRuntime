"""Logging tests for bounded dialogue request metadata."""

import logging

from fastapi.testclient import TestClient
import pytest

from app.main import create_app
from app.providers.stub_provider import StubDialogueProvider


SENSITIVE_MARKERS = (
    "LOG_PRIVATE_PLAYER_INPUT",
    "LOG_PRIVATE_DISPLAY_NAME",
    "LOG_PRIVATE_ROLE",
    "LOG_PRIVATE_PERSONALITY",
    "LOG_PRIVATE_STYLE",
    "LOG_PRIVATE_GOAL",
    "LOG_PRIVATE_LOCATION",
    "LOG_PRIVATE_SITUATION",
    "LOG_PRIVATE_FACT",
    "LOG_PRIVATE_HISTORY",
)


def test_success_logs_only_allowed_request_metadata(
    caplog: pytest.LogCaptureFixture,
) -> None:
    payload = {
        "request_id": "req-log-safe",
        "npc_id": "npc-log-safe",
        "player_input": SENSITIVE_MARKERS[0],
        "context": {
            "npc": {
                "display_name": SENSITIVE_MARKERS[1],
                "role": SENSITIVE_MARKERS[2],
                "personality": [SENSITIVE_MARKERS[3]],
                "speaking_style": SENSITIVE_MARKERS[4],
                "goals": [SENSITIVE_MARKERS[5]],
            },
            "world": {
                "location": SENSITIVE_MARKERS[6],
                "situation": SENSITIVE_MARKERS[7],
                "facts": [SENSITIVE_MARKERS[8]],
            },
            "dialogue_history": [
                {"role": "player", "content": SENSITIVE_MARKERS[9]}
            ],
        },
    }
    caplog.set_level(logging.INFO, logger="app.services.dialogue_service")

    with TestClient(create_app(provider=StubDialogueProvider())) as client:
        response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 200
    assert "request_id=req-log-safe" in caplog.text
    assert "npc_id=npc-log-safe" in caplog.text
    assert "provider=stub" in caplog.text
    assert "has_context=True" in caplog.text
    assert "history_count=1" in caplog.text
    for marker in SENSITIVE_MARKERS:
        assert marker not in caplog.text
