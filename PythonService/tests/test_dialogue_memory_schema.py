"""Milestone 4 protocol validation for optional persistent-memory scope."""

from collections.abc import Iterator

from fastapi.testclient import TestClient
import pytest

from app.main import create_app
from app.providers.stub_provider import StubDialogueProvider


VALID_REQUEST = {
    "request_id": "req-memory-001",
    "npc_id": "npc_guard_01",
    "player_input": "Do you remember me?",
}


@pytest.fixture
def client() -> Iterator[TestClient]:
    with TestClient(create_app(provider=StubDialogueProvider())) as test_client:
        yield test_client


def test_request_without_memory_remains_compatible(client: TestClient) -> None:
    response = client.post("/v1/dialogue", json=VALID_REQUEST)

    assert response.status_code == 200
    assert response.json().keys() == {"request_id", "npc_id", "reply", "provider"}


@pytest.mark.parametrize("scope_id", ["a", "😀" * 128])
def test_memory_scope_boundaries_are_accepted(
    client: TestClient,
    scope_id: str,
) -> None:
    response = client.post(
        "/v1/dialogue",
        json={**VALID_REQUEST, "memory": {"scope_id": scope_id}},
    )

    assert response.status_code == 200
    assert response.json().keys() == {"request_id", "npc_id", "reply", "provider"}


def test_memory_and_transient_context_can_be_combined(client: TestClient) -> None:
    response = client.post(
        "/v1/dialogue",
        json={
            **VALID_REQUEST,
            "context": {
                "npc": {
                    "display_name": "Guard",
                    "role": "gate guard",
                    "personality": ["careful"],
                    "speaking_style": "brief",
                    "goals": [],
                },
                "world": {
                    "location": "north gate",
                    "situation": "the gate is closed",
                    "facts": [],
                },
                "dialogue_history": [],
            },
            "memory": {"scope_id": "player-local-01"},
        },
    )

    assert response.status_code == 200


def test_unknown_memory_fields_are_ignored(client: TestClient) -> None:
    response = client.post(
        "/v1/dialogue",
        json={
            **VALID_REQUEST,
            "memory": {"scope_id": "player-local-01", "future_option": True},
        },
    )

    assert response.status_code == 200


@pytest.mark.parametrize(
    "memory",
    [
        None,
        {},
        {"scope_id": 42},
        {"scope_id": "x" * 129},
        "player-local-01",
    ],
)
def test_invalid_memory_structure_returns_validation_error(
    client: TestClient,
    memory: object,
) -> None:
    response = client.post(
        "/v1/dialogue",
        json={**VALID_REQUEST, "memory": memory},
    )

    assert response.status_code == 422
    assert response.json()["error"]["code"] == "validation_error"


@pytest.mark.parametrize(
    ("scope_id", "message"),
    [
        (" \t ", "memory.scope_id must not be blank"),
        (
            " player-local-01",
            "memory.scope_id must not contain leading or trailing whitespace",
        ),
        (
            "player-local-01\n",
            "memory.scope_id must not contain leading or trailing whitespace",
        ),
    ],
)
def test_invalid_memory_scope_content_returns_business_error(
    client: TestClient,
    scope_id: str,
    message: str,
) -> None:
    response = client.post(
        "/v1/dialogue",
        json={**VALID_REQUEST, "memory": {"scope_id": scope_id}},
    )

    assert response.status_code == 400
    assert response.json() == {
        "request_id": VALID_REQUEST["request_id"],
        "error": {"code": "invalid_request", "message": message},
    }
