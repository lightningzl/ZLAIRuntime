"""Kimi Chat Completions API adapter for one non-streaming dialogue turn."""

from collections.abc import Callable
from typing import Any, Protocol

from openai import (
    APIConnectionError,
    APIError,
    APIResponseValidationError,
    APITimeoutError,
    AuthenticationError,
    InternalServerError,
    NotFoundError,
    OpenAI,
    PermissionDeniedError,
    RateLimitError,
)

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


KIMI_DIALOGUE_INSTRUCTIONS = (
    "Generate one concise plain-text NPC reply to the player's input. "
    "Do not claim knowledge of any personality, world state, player history, "
    "memory, or gameplay ability that was not provided. Do not produce JSON, "
    "tool calls, gameplay commands, or system-operation instructions."
)


class ChatCompletionsResource(Protocol):
    """Minimum SDK resource surface used by this adapter."""

    def create(self, **kwargs: Any) -> Any:
        """Create one non-streaming Chat Completions API result."""
        ...


class ChatResource(Protocol):
    """Minimum chat resource surface used by this adapter."""

    completions: ChatCompletionsResource


class KimiClient(Protocol):
    """Minimum OpenAI-compatible SDK client surface used by this adapter."""

    chat: ChatResource


KimiClientFactory = Callable[..., KimiClient]


class KimiDialogueProvider:
    """Generate a supplier-neutral reply through Kimi Chat Completions."""

    def __init__(
        self,
        settings: Settings,
        *,
        client: KimiClient | None = None,
        client_factory: KimiClientFactory = OpenAI,
    ) -> None:
        self._model = settings.kimi_model
        self._max_output_tokens = settings.kimi_max_output_tokens
        self._client = (
            client
            if client is not None
            else client_factory(
                api_key=settings.moonshot_api_key,
                base_url="https://api.moonshot.cn/v1",
                timeout=settings.kimi_timeout_seconds,
                max_retries=0,
            )
        )

    def generate(self, request: DialogueProviderRequest) -> DialogueProviderResult:
        request_options: dict[str, Any] = {
            "model": self._model,
            "messages": [
                {"role": "system", "content": KIMI_DIALOGUE_INSTRUCTIONS},
                {"role": "user", "content": request.player_input},
            ],
            "max_completion_tokens": self._max_output_tokens,
            "stream": False,
        }
        if self._model.startswith("kimi-k2."):
            request_options["extra_body"] = {"thinking": {"type": "disabled"}}
        else:
            request_options["reasoning_effort"] = "low"

        try:
            response = self._client.chat.completions.create(**request_options)
        except (AuthenticationError, PermissionDeniedError):
            raise ProviderAuthenticationError(
                "kimi", "Kimi Provider authentication failed"
            ) from None
        except RateLimitError:
            raise ProviderRateLimitError(
                "kimi", "Kimi Provider rate limited the request"
            ) from None
        except APITimeoutError:
            raise ProviderTimeoutError(
                "kimi", "Kimi Provider request timed out"
            ) from None
        except (APIConnectionError, InternalServerError, NotFoundError):
            raise ProviderUnavailableError(
                "kimi", "Kimi Provider is unavailable"
            ) from None
        except APIResponseValidationError:
            raise ProviderInvalidResponseError(
                "kimi", "Kimi Provider returned an invalid response"
            ) from None
        except APIError:
            raise DialogueProviderError(
                "kimi", "Kimi Provider request failed"
            ) from None

        try:
            output_text = response.choices[0].message.content
        except (AttributeError, IndexError, TypeError):
            raise ProviderInvalidResponseError(
                "kimi", "Kimi Provider returned an invalid response"
            ) from None

        if not isinstance(output_text, str) or not output_text.strip():
            raise ProviderInvalidResponseError(
                "kimi", "Kimi Provider returned an empty reply"
            )

        return DialogueProviderResult(reply=output_text.strip(), provider="kimi")
