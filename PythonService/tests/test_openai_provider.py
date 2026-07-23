"""Offline tests for the OpenAI Responses API adapter."""

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
from app.providers.openai_provider import (
    OPENAI_DIALOGUE_INSTRUCTIONS,
    OpenAIDialogueProvider,
)


class FakeResponsesResource:
    def __init__(self, response: object, error: Exception | None = None) -> None:
        self._response = response
        self._error = error
        self.calls: list[dict[str, Any]] = []

    def create(self, **kwargs: Any) -> object:
        self.calls.append(kwargs)
        if self._error is not None:
            raise self._error
        return self._response


class FakeOpenAIClient:
    def __init__(self, response: object, error: Exception | None = None) -> None:
        self.responses = FakeResponsesResource(response, error)


def _settings() -> Settings:
    return Settings.from_env(
        {
            "OPENAI_API_KEY": "test-placeholder",
            "ZL_OPENAI_MODEL": "test-model",
            "ZL_OPENAI_TIMEOUT_SECONDS": "12.5",
            "ZL_OPENAI_MAX_OUTPUT_TOKENS": "321",
        }
    )


def test_client_configuration_disables_retries_and_uses_settings() -> None:
    captured_kwargs: dict[str, Any] = {}
    fake_client = FakeOpenAIClient(SimpleNamespace(output_text="reply"))

    def client_factory(**kwargs: Any) -> FakeOpenAIClient:
        captured_kwargs.update(kwargs)
        return fake_client

    OpenAIDialogueProvider(_settings(), client_factory=client_factory)

    assert captured_kwargs == {
        "api_key": "test-placeholder",
        "timeout": 12.5,
        "max_retries": 0,
    }


def test_generate_constructs_one_non_streaming_response_request() -> None:
    fake_client = FakeOpenAIClient(SimpleNamespace(output_text="  城门已经关闭。  "))
    provider = OpenAIDialogueProvider(_settings(), client=fake_client)

    result = provider.generate(
        DialogueProviderRequest(
            npc_id="npc_guard_01",
            player_input="发生了什么？",
        )
    )

    assert result == DialogueProviderResult(
        reply="城门已经关闭。",
        provider="openai",
    )
    assert fake_client.responses.calls == [
        {
            "model": "test-model",
            "instructions": OPENAI_DIALOGUE_INSTRUCTIONS,
            "input": "发生了什么？",
            "max_output_tokens": 321,
            "store": False,
            "stream": False,
        }
    ]
    request_arguments = fake_client.responses.calls[0]
    assert "tools" not in request_arguments
    assert "previous_response_id" not in request_arguments
    assert "npc_guard_01" not in str(request_arguments)


@pytest.mark.parametrize("output_text", [None, "", "   ", 42])
def test_empty_or_non_text_output_is_rejected(output_text: object) -> None:
    provider = OpenAIDialogueProvider(
        _settings(),
        client=FakeOpenAIClient(SimpleNamespace(output_text=output_text)),
    )

    with pytest.raises(
        ProviderInvalidResponseError,
        match="OpenAI Provider returned an empty reply",
    ):
        provider.generate(
            DialogueProviderRequest(npc_id="npc-1", player_input="hello")
        )


def test_missing_output_text_is_rejected_without_sdk_type_leakage() -> None:
    provider = OpenAIDialogueProvider(
        _settings(),
        client=FakeOpenAIClient(SimpleNamespace()),
    )

    with pytest.raises(
        ProviderInvalidResponseError,
        match="OpenAI Provider returned an invalid response",
    ):
        provider.generate(
            DialogueProviderRequest(npc_id="npc-1", player_input="hello")
        )


def _status_error(error_type: type[openai.APIStatusError], status: int) -> Exception:
    request = httpx.Request("POST", "https://api.openai.invalid/v1/responses")
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
                httpx.Request("POST", "https://api.openai.invalid/v1/responses")
            ),
            ProviderTimeoutError,
        ),
        (
            openai.APIConnectionError(
                request=httpx.Request(
                    "POST", "https://api.openai.invalid/v1/responses"
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
    fake_client = FakeOpenAIClient(SimpleNamespace(output_text="unused"), sdk_error)
    provider = OpenAIDialogueProvider(_settings(), client=fake_client)

    with pytest.raises(expected_type) as captured:
        provider.generate(
            DialogueProviderRequest(npc_id="npc-1", player_input="hello")
        )

    assert type(captured.value) is expected_type
    assert captured.value.provider == "openai"
    assert "sensitive upstream detail" not in str(captured.value)
    assert len(fake_client.responses.calls) == 1
