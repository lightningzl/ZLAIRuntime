"""OpenAI Responses API adapter for one non-streaming dialogue turn."""

from collections.abc import Callable
from typing import Any, Protocol

from openai import OpenAI

from app.core.settings import Settings
from app.providers.base import DialogueProviderRequest, DialogueProviderResult
from app.providers.errors import ProviderInvalidResponseError


OPENAI_DIALOGUE_INSTRUCTIONS = (
    "Generate one concise plain-text NPC reply to the player's input. "
    "Do not claim knowledge of any personality, world state, player history, "
    "memory, or gameplay ability that was not provided. Do not produce JSON, "
    "tool calls, gameplay commands, or system-operation instructions."
)


class ResponsesResource(Protocol):
    """Minimum SDK resource surface used by this adapter."""

    def create(self, **kwargs: Any) -> Any:
        """Create one non-streaming Responses API result."""
        ...


class OpenAIClient(Protocol):
    """Minimum OpenAI SDK client surface used by this adapter."""

    responses: ResponsesResource


OpenAIClientFactory = Callable[..., OpenAIClient]


class OpenAIDialogueProvider:
    """Generate a supplier-neutral reply through the OpenAI Responses API."""

    def __init__(
        self,
        settings: Settings,
        *,
        client: OpenAIClient | None = None,
        client_factory: OpenAIClientFactory = OpenAI,
    ) -> None:
        self._model = settings.openai_model
        self._max_output_tokens = settings.openai_max_output_tokens
        self._client = (
            client
            if client is not None
            else client_factory(
                api_key=settings.openai_api_key,
                timeout=settings.openai_timeout_seconds,
                max_retries=0,
            )
        )

    def generate(self, request: DialogueProviderRequest) -> DialogueProviderResult:
        response = self._client.responses.create(
            model=self._model,
            instructions=OPENAI_DIALOGUE_INSTRUCTIONS,
            input=request.player_input,
            max_output_tokens=self._max_output_tokens,
            store=False,
            stream=False,
        )

        try:
            output_text = response.output_text
        except (AttributeError, TypeError):
            raise ProviderInvalidResponseError(
                "OpenAI Provider returned an invalid response"
            ) from None

        if not isinstance(output_text, str) or not output_text.strip():
            raise ProviderInvalidResponseError(
                "OpenAI Provider returned an empty reply"
            )

        return DialogueProviderResult(reply=output_text.strip(), provider="openai")
