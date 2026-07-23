"""Construct the explicitly configured dialogue Provider."""

from collections.abc import Callable

from app.core.settings import Settings
from app.providers.base import DialogueProvider
from app.providers.stub_provider import StubDialogueProvider


OpenAIProviderFactory = Callable[[Settings], DialogueProvider]


class ProviderConfigurationError(RuntimeError):
    """Raised when the selected Provider cannot be constructed."""


def create_dialogue_provider(
    settings: Settings,
    *,
    openai_factory: OpenAIProviderFactory | None = None,
) -> DialogueProvider:
    """Create exactly the selected Provider and never fall back silently."""
    if settings.dialogue_provider == "stub":
        return StubDialogueProvider()

    if openai_factory is None:
        raise ProviderConfigurationError(
            "OpenAI Provider implementation is unavailable"
        )
    return openai_factory(settings)
