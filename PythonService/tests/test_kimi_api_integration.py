"""Offline API tests spanning the Kimi adapter and HTTP error mapping."""

import os
import socket
from types import SimpleNamespace
from typing import Any

from fastapi.testclient import TestClient
import httpx
import openai
import pytest

from app.core.settings import Settings
from app.main import create_app
from app.providers.kimi_provider import KimiDialogueProvider


VALID_REQUEST = {
    "request_id": "req-kimi-api",
    "npc_id": "npc-1",
    "player_input": "hello",
}


class FakeChatCompletionsResource:
    def __init__(self, result: object) -> None:
        self._result = result
        self.calls: list[dict[str, Any]] = []

    def create(self, **kwargs: Any) -> object:
        self.calls.append(kwargs)
        if isinstance(self._result, Exception):
            raise self._result
        return self._result


class FakeKimiClient:
    def __init__(self, result: object) -> None:
        self.chat = SimpleNamespace(completions=FakeChatCompletionsResource(result))


def _provider(result: object) -> tuple[KimiDialogueProvider, FakeKimiClient]:
    settings = Settings.from_env(
        {
            "MOONSHOT_API_KEY": "test-placeholder",
            "ZL_KIMI_MODEL": "test-model",
        }
    )
    client = FakeKimiClient(result)
    return KimiDialogueProvider(settings, client=client), client


def _status_error(
    error_type: type[openai.APIStatusError],
    status_code: int,
) -> openai.APIStatusError:
    request = httpx.Request(
        "POST", "https://api.moonshot.cn/v1/chat/completions"
    )
    response = httpx.Response(status_code, request=request)
    return error_type("sensitive upstream detail", response=response, body=None)


def test_kimi_adapter_success_returns_protocol_response() -> None:
    provider, fake_client = _provider(
        SimpleNamespace(
            choices=[
                SimpleNamespace(message=SimpleNamespace(content="  reply text  "))
            ]
        )
    )

    with TestClient(create_app(provider=provider)) as client:
        response = client.post("/v1/dialogue", json=VALID_REQUEST)

    assert response.status_code == 200
    assert response.json() == {
        "request_id": "req-kimi-api",
        "npc_id": "npc-1",
        "reply": "reply text",
        "provider": "kimi",
    }
    assert len(fake_client.chat.completions.calls) == 1


@pytest.mark.parametrize(
    ("sdk_result", "status_code", "error_code"),
    [
        (
            _status_error(openai.AuthenticationError, 401),
            503,
            "provider_auth_error",
        ),
        (_status_error(openai.RateLimitError, 429), 429, "provider_rate_limited"),
        (
            openai.APITimeoutError(
                httpx.Request(
                    "POST", "https://api.moonshot.cn/v1/chat/completions"
                )
            ),
            504,
            "provider_timeout",
        ),
        (
            openai.APIConnectionError(
                request=httpx.Request(
                    "POST", "https://api.moonshot.cn/v1/chat/completions"
                )
            ),
            503,
            "provider_unavailable",
        ),
        (
            SimpleNamespace(
                choices=[SimpleNamespace(message=SimpleNamespace(content=""))]
            ),
            502,
            "provider_error",
        ),
    ],
)
def test_kimi_failures_map_end_to_end_without_leaking_details(
    sdk_result: object,
    status_code: int,
    error_code: str,
) -> None:
    provider, fake_client = _provider(sdk_result)

    with TestClient(
        create_app(provider=provider),
        raise_server_exceptions=False,
    ) as client:
        response = client.post("/v1/dialogue", json=VALID_REQUEST)

    assert response.status_code == status_code
    assert response.json()["request_id"] == "req-kimi-api"
    assert response.json()["error"]["code"] == error_code
    assert "sensitive upstream detail" not in response.text
    assert len(fake_client.chat.completions.calls) == 1


def test_suite_guard_blocks_external_connections() -> None:
    with pytest.raises(
        AssertionError,
        match="external network access is not allowed",
    ):
        socket.create_connection(("api.moonshot.cn", 443))


def test_suite_guard_removes_real_moonshot_api_key() -> None:
    assert "MOONSHOT_API_KEY" not in os.environ
