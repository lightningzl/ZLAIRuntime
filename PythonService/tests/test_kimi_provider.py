"""Offline tests for the Kimi Chat Completions API adapter."""

from types import SimpleNamespace
from typing import Any

import httpx
import openai
import pytest

from app.core.settings import Settings
from app.providers.base import DialogueProviderRequest, DialogueProviderResult
from app.providers.errors import (
    DialogueProviderError,
    ProviderAuthenticationError,
    ProviderInvalidResponseError,
    ProviderRateLimitError,
    ProviderTimeoutError,
    ProviderUnavailableError,
)
from app.providers.kimi_provider import (
    KIMI_DIALOGUE_INSTRUCTIONS,
    KimiDialogueProvider,
)


class FakeChatCompletionsResource:
    def __init__(self, response: object, error: Exception | None = None) -> None:
        self._response = response
        self._error = error
        self.calls: list[dict[str, Any]] = []

    def create(self, **kwargs: Any) -> object:
        self.calls.append(kwargs)
        if self._error is not None:
            raise self._error
        return self._response


class FakeKimiClient:
    def __init__(self, response: object, error: Exception | None = None) -> None:
        self.chat = SimpleNamespace(
            completions=FakeChatCompletionsResource(response, error)
        )


def _settings() -> Settings:
    return Settings.from_env(
        {
            "MOONSHOT_API_KEY": "test-placeholder",
            "ZL_KIMI_MODEL": "test-model",
            "ZL_KIMI_TIMEOUT_SECONDS": "12.5",
            "ZL_KIMI_MAX_OUTPUT_TOKENS": "321",
        }
    )


def test_client_configuration_disables_retries_and_uses_settings() -> None:
    captured_kwargs: dict[str, Any] = {}
    fake_client = FakeKimiClient(
        SimpleNamespace(
            choices=[SimpleNamespace(message=SimpleNamespace(content="reply"))]
        )
    )

    def client_factory(**kwargs: Any) -> FakeKimiClient:
        captured_kwargs.update(kwargs)
        return fake_client

    KimiDialogueProvider(_settings(), client_factory=client_factory)

    assert captured_kwargs == {
        "api_key": "test-placeholder",
        "base_url": "https://api.moonshot.ai/v1",
        "timeout": 12.5,
        "max_retries": 0,
    }


def test_generate_constructs_one_non_streaming_response_request() -> None:
    fake_client = FakeKimiClient(
        SimpleNamespace(
            choices=[
                SimpleNamespace(
                    message=SimpleNamespace(content="  城门已经关闭。  ")
                )
            ]
        )
    )
    provider = KimiDialogueProvider(_settings(), client=fake_client)

    result = provider.generate(
        DialogueProviderRequest(
            npc_id="npc_guard_01",
            player_input="发生了什么？",
        )
    )

    assert result == DialogueProviderResult(
        reply="城门已经关闭。",
        provider="kimi",
    )
    assert fake_client.chat.completions.calls == [
        {
            "model": "test-model",
            "messages": [
                {"role": "system", "content": KIMI_DIALOGUE_INSTRUCTIONS},
                {"role": "user", "content": "发生了什么？"},
            ],
            "reasoning_effort": "low",
            "max_completion_tokens": 321,
            "stream": False,
        }
    ]
    request_arguments = fake_client.chat.completions.calls[0]
    assert "tools" not in request_arguments
    assert "previous_response_id" not in str(request_arguments)
    assert "npc_guard_01" not in str(request_arguments)


@pytest.mark.parametrize("output_text", [None, "", "   ", 42])
def test_empty_or_non_text_output_is_rejected(output_text: object) -> None:
    provider = KimiDialogueProvider(
        _settings(),
        client=FakeKimiClient(
            SimpleNamespace(
                choices=[
                    SimpleNamespace(message=SimpleNamespace(content=output_text))
                ]
            )
        ),
    )

    with pytest.raises(
        ProviderInvalidResponseError,
        match="Kimi Provider returned an empty reply",
    ):
        provider.generate(
            DialogueProviderRequest(npc_id="npc-1", player_input="hello")
        )


def test_missing_output_text_is_rejected_without_sdk_type_leakage() -> None:
    provider = KimiDialogueProvider(
        _settings(),
        client=FakeKimiClient(SimpleNamespace()),
    )

    with pytest.raises(
        ProviderInvalidResponseError,
        match="Kimi Provider returned an invalid response",
    ):
        provider.generate(
            DialogueProviderRequest(npc_id="npc-1", player_input="hello")
        )


def _status_error(error_type: type[openai.APIStatusError], status: int) -> Exception:
    request = httpx.Request(
        "POST", "https://api.moonshot.ai/v1/chat/completions"
    )
    response = httpx.Response(status, request=request)
    return error_type("sensitive upstream detail", response=response, body=None)


@pytest.mark.parametrize(
    ("sdk_error", "expected_type"),
    [
        (_status_error(openai.AuthenticationError, 401), ProviderAuthenticationError),
        (_status_error(openai.PermissionDeniedError, 403), ProviderAuthenticationError),
        (_status_error(openai.RateLimitError, 429), ProviderRateLimitError),
        (
            openai.APITimeoutError(
                httpx.Request(
                    "POST", "https://api.moonshot.ai/v1/chat/completions"
                )
            ),
            ProviderTimeoutError,
        ),
        (
            openai.APIConnectionError(
                request=httpx.Request(
                    "POST", "https://api.moonshot.ai/v1/chat/completions"
                )
            ),
            ProviderUnavailableError,
        ),
        (_status_error(openai.InternalServerError, 500), ProviderUnavailableError),
        (_status_error(openai.NotFoundError, 404), ProviderUnavailableError),
        (_status_error(openai.BadRequestError, 400), DialogueProviderError),
    ],
)
def test_sdk_errors_are_classified_without_raw_details(
    sdk_error: Exception,
    expected_type: type[DialogueProviderError],
) -> None:
    fake_client = FakeKimiClient(SimpleNamespace(), sdk_error)
    provider = KimiDialogueProvider(_settings(), client=fake_client)

    with pytest.raises(expected_type) as captured:
        provider.generate(
            DialogueProviderRequest(npc_id="npc-1", player_input="hello")
        )

    assert type(captured.value) is expected_type
    assert captured.value.provider == "kimi"
    assert "sensitive upstream detail" not in str(captured.value)
    assert len(fake_client.chat.completions.calls) == 1
