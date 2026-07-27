"""Milestone 3 protocol validation for transient dialogue context."""

from collections.abc import Iterator
from copy import deepcopy

from fastapi.testclient import TestClient
import pytest

from app.main import create_app
from app.providers.base import DialogueGenerationContext, DialogueProviderResult
from app.providers.stub_provider import StubDialogueProvider


VALID_REQUEST = {
    "request_id": "req-context-001",
    "npc_id": "npc_guard_01",
    "player_input": "这里发生了什么？",
}

VALID_CONTEXT = {
    "npc": {
        "display_name": "城门守卫",
        "role": "守卫",
        "personality": ["谨慎", "忠于职守"],
        "speaking_style": "简短、正式",
        "goals": ["保护城门"],
    },
    "world": {
        "location": "北城门",
        "situation": "警报后城门关闭",
        "facts": ["玩家曾帮助巡逻队"],
    },
    "dialogue_history": [
        {"role": "player", "content": "城门为什么关了？"},
        {"role": "npc", "content": "刚刚响起了警报。"},
    ],
}


@pytest.fixture
def client() -> Iterator[TestClient]:
    with TestClient(create_app(provider=StubDialogueProvider())) as test_client:
        yield test_client


def _request_with_context(context: dict | None = None) -> dict:
    return {
        **VALID_REQUEST,
        "context": deepcopy(VALID_CONTEXT if context is None else context),
    }


def test_full_context_is_accepted_without_changing_response_shape(
    client: TestClient,
) -> None:
    response = client.post("/v1/dialogue", json=_request_with_context())

    assert response.status_code == 200
    assert response.json().keys() == {"request_id", "npc_id", "reply", "provider"}


def test_service_passes_supplier_neutral_context_to_injected_provider() -> None:
    captured_contexts: list[DialogueGenerationContext] = []

    class CapturingProvider:
        def generate(
            self,
            context: DialogueGenerationContext,
        ) -> DialogueProviderResult:
            captured_contexts.append(context)
            return DialogueProviderResult(reply="captured", provider="stub")

    with TestClient(create_app(provider=CapturingProvider())) as capturing_client:
        response = capturing_client.post(
            "/v1/dialogue",
            json=_request_with_context(),
        )

    assert response.status_code == 200
    assert len(captured_contexts) == 1
    captured_context = captured_contexts[0]
    assert '"display_name":"城门守卫"' in captured_context.context_data_json
    assert '"location":"北城门"' in captured_context.context_data_json
    assert [message.role for message in captured_context.messages] == [
        "user",
        "assistant",
        "user",
    ]
    assert captured_context.messages[-1].content == VALID_REQUEST["player_input"]


def test_unknown_context_fields_are_ignored_for_v1_compatibility(
    client: TestClient,
) -> None:
    payload = _request_with_context()
    payload["context"]["future_context"] = {"enabled": True}
    payload["context"]["npc"]["future_trait"] = 42
    payload["context"]["dialogue_history"][0]["future_message"] = "ignored"

    response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 200


def test_all_context_maximum_boundaries_are_accepted(client: TestClient) -> None:
    context = {
        "npc": {
            "display_name": "😀" * 64,
            "role": "r" * 128,
            "personality": ["p" * 64] * 8,
            "speaking_style": "s" * 256,
            "goals": ["g" * 128] * 8,
        },
        "world": {
            "location": "l" * 128,
            "situation": "s" * 512,
            "facts": ["f" * 256] * 16,
        },
        "dialogue_history": [
            {
                "role": "player" if index % 2 == 0 else "npc",
                "content": "h" * 512,
            }
            for index in range(8)
        ],
    }

    response = client.post("/v1/dialogue", json=_request_with_context(context))

    assert response.status_code == 200


@pytest.mark.parametrize(
    ("section", "field", "value"),
    [
        ("npc", "display_name", "x" * 65),
        ("npc", "role", "x" * 129),
        ("npc", "speaking_style", "x" * 257),
        ("world", "location", "x" * 129),
        ("world", "situation", "x" * 513),
    ],
)
def test_oversized_context_text_returns_validation_error(
    client: TestClient,
    section: str,
    field: str,
    value: str,
) -> None:
    payload = _request_with_context()
    payload["context"][section][field] = value

    response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 422
    assert response.json()["error"]["code"] == "validation_error"


@pytest.mark.parametrize(
    ("field", "value"),
    [
        ("personality", []),
        ("personality", ["p"] * 9),
        ("goals", ["g"] * 9),
    ],
)
def test_invalid_npc_array_size_returns_validation_error(
    client: TestClient,
    field: str,
    value: list[str],
) -> None:
    payload = _request_with_context()
    payload["context"]["npc"][field] = value

    response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 422
    assert response.json()["error"]["code"] == "validation_error"


@pytest.mark.parametrize(
    ("section", "field", "value"),
    [
        ("npc", "personality", ["x" * 65]),
        ("npc", "goals", ["x" * 129]),
        ("world", "facts", ["x" * 257]),
    ],
)
def test_oversized_array_item_returns_validation_error(
    client: TestClient,
    section: str,
    field: str,
    value: list[str],
) -> None:
    payload = _request_with_context()
    payload["context"][section][field] = value

    response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 422
    assert response.json()["error"]["code"] == "validation_error"


def test_too_many_world_facts_or_history_messages_are_rejected(
    client: TestClient,
) -> None:
    facts_payload = _request_with_context()
    facts_payload["context"]["world"]["facts"] = ["fact"] * 17
    history_payload = _request_with_context()
    history_payload["context"]["dialogue_history"] = [
        {"role": "player", "content": "old turn"}
    ] * 9

    facts_response = client.post("/v1/dialogue", json=facts_payload)
    history_response = client.post("/v1/dialogue", json=history_payload)

    assert facts_response.status_code == 422
    assert history_response.status_code == 422


def test_invalid_history_role_returns_validation_error(client: TestClient) -> None:
    payload = _request_with_context()
    payload["context"]["dialogue_history"][0]["role"] = "system"

    response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 422
    assert response.json()["error"]["code"] == "validation_error"


@pytest.mark.parametrize(
    ("section", "field"),
    [
        ("npc", "display_name"),
        ("npc", "role"),
        ("npc", "speaking_style"),
        ("world", "location"),
        ("world", "situation"),
    ],
)
def test_blank_context_text_returns_business_error(
    client: TestClient,
    section: str,
    field: str,
) -> None:
    payload = _request_with_context()
    payload["context"][section][field] = " \t "

    response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 400
    assert response.json() == {
        "request_id": VALID_REQUEST["request_id"],
        "error": {
            "code": "invalid_request",
            "message": f"context.{section}.{field} must not be blank",
        },
    }


def test_blank_array_and_history_content_return_business_errors(
    client: TestClient,
) -> None:
    trait_payload = _request_with_context()
    trait_payload["context"]["npc"]["personality"][0] = " "
    history_payload = _request_with_context()
    history_payload["context"]["dialogue_history"][0]["content"] = "\n"

    trait_response = client.post("/v1/dialogue", json=trait_payload)
    history_response = client.post("/v1/dialogue", json=history_payload)

    assert trait_response.status_code == 400
    assert trait_response.json()["error"]["message"] == (
        "context.npc.personality[0] must not be blank"
    )
    assert history_response.status_code == 400
    assert history_response.json()["error"]["message"] == (
        "context.dialogue_history[0].content must not be blank"
    )


@pytest.mark.parametrize("missing_field", ["npc", "world", "dialogue_history"])
def test_context_requires_all_three_sections(
    client: TestClient,
    missing_field: str,
) -> None:
    context = deepcopy(VALID_CONTEXT)
    context.pop(missing_field)

    response = client.post("/v1/dialogue", json=_request_with_context(context))

    assert response.status_code == 422
    assert response.json()["error"]["code"] == "validation_error"
