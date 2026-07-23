"""Construct the explicitly configured dialogue Provider."""

from collections.abc import Callable

from app.core.settings import Settings
from app.providers.base import DialogueProvider
from app.providers.kimi_provider import KimiDialogueProvider
from app.providers.stub_provider import StubDialogueProvider


KimiProviderFactory = Callable[[Settings], DialogueProvider]


def create_dialogue_provider(
    settings: Settings,
    *,
    kimi_factory: KimiProviderFactory | None = None,
) -> DialogueProvider:
    """Create exactly the selected Provider and never fall back silently."""
    if settings.dialogue_provider == "stub":
        return StubDialogueProvider()

    factory = kimi_factory or KimiDialogueProvider
    return factory(settings)
