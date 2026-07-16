"""Protocol-level tests for the dialogue API."""

from collections.abc import Iterator
import socket

from fastapi.testclient import TestClient
import pytest

import app.api.dialogue as dialogue_api
from app.main import create_app
from app.schemas.dialogue import DialogueRequest, DialogueResponse
from app.services.dialogue_service import STUB_REPLY


VALID_REQUEST = {
    "request_id": "req-001",
    "npc_id": "npc_guard_01",
    "player_input": "这里发生了什么？",
}


@pytest.fixture
def client() -> Iterator[TestClient]:
    """Create an in-process client that exposes handled 500 responses."""
    with TestClient(create_app(), raise_server_exceptions=False) as test_client:
        yield test_client


def test_success_response_preserves_fields_and_is_deterministic(
    client: TestClient,
) -> None:
    first_response = client.post("/v1/dialogue", json=VALID_REQUEST)
    second_response = client.post("/v1/dialogue", json=VALID_REQUEST)

    expected_response = {
        "request_id": VALID_REQUEST["request_id"],
        "npc_id": VALID_REQUEST["npc_id"],
        "reply": STUB_REPLY,
        "provider": "stub",
    }
    assert first_response.status_code == 200
    assert first_response.json() == expected_response
    assert second_response.status_code == 200
    assert second_response.json() == expected_response


def test_empty_player_input_returns_business_error(client: TestClient) -> None:
    payload = {**VALID_REQUEST, "player_input": ""}

    response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 400
    assert response.json() == {
        "request_id": VALID_REQUEST["request_id"],
        "error": {
            "code": "invalid_request",
            "message": "player_input must not be empty",
        },
    }


@pytest.mark.parametrize("missing_field", ["request_id", "npc_id", "player_input"])
def test_missing_required_field_returns_validation_error(
    client: TestClient,
    missing_field: str,
) -> None:
    payload = VALID_REQUEST.copy()
    payload.pop(missing_field)

    response = client.post("/v1/dialogue", json=payload)

    expected_request_id = "" if missing_field == "request_id" else VALID_REQUEST["request_id"]
    assert response.status_code == 422
    assert response.json() == {
        "request_id": expected_request_id,
        "error": {
            "code": "validation_error",
            "message": "request validation failed",
        },
    }


@pytest.mark.parametrize("invalid_field", ["request_id", "npc_id", "player_input"])
def test_wrong_field_type_returns_validation_error(
    client: TestClient,
    invalid_field: str,
) -> None:
    payload = {**VALID_REQUEST, invalid_field: 42}

    response = client.post("/v1/dialogue", json=payload)

    expected_request_id = "" if invalid_field == "request_id" else VALID_REQUEST["request_id"]
    assert response.status_code == 422
    assert response.json() == {
        "request_id": expected_request_id,
        "error": {
            "code": "validation_error",
            "message": "request validation failed",
        },
    }


def test_internal_error_does_not_expose_exception_details(
    client: TestClient,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def raise_internal_error(_request: DialogueRequest) -> DialogueResponse:
        raise RuntimeError("secret_token at C:/internal/service.py")

    monkeypatch.setattr(dialogue_api, "build_dialogue_response", raise_internal_error)

    response = client.post("/v1/dialogue", json=VALID_REQUEST)

    assert response.status_code == 500
    assert response.json() == {
        "request_id": VALID_REQUEST["request_id"],
        "error": {
            "code": "internal_error",
            "message": "internal server error",
        },
    }
    assert "secret_token" not in response.text
    assert "internal/service.py" not in response.text
    assert "traceback" not in response.text.lower()


def test_request_succeeds_without_api_key_or_network(
    client: TestClient,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    monkeypatch.delenv("OPENAI_API_KEY", raising=False)

    def reject_network(*_args: object, **_kwargs: object) -> None:
        raise AssertionError("external network access is not allowed")

    monkeypatch.setattr(socket, "create_connection", reject_network)

    response = client.post("/v1/dialogue", json=VALID_REQUEST)

    assert response.status_code == 200
    assert response.json()["provider"] == "stub"
