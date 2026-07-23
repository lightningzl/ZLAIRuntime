"""Tests for explicit Provider selection and injection boundaries."""

import pytest

from app.core.settings import Settings
from app.providers.base import (
    DialogueProviderRequest,
    DialogueProviderResult,
)
from app.providers.factory import (
    ProviderConfigurationError,
    create_dialogue_provider,
)
from app.providers.stub_provider import STUB_REPLY, StubDialogueProvider


def test_explicit_stub_selection_returns_deterministic_provider() -> None:
    provider = create_dialogue_provider(
        Settings.from_env({"ZL_DIALOGUE_PROVIDER": "stub"})
    )

    assert isinstance(provider, StubDialogueProvider)
    result = provider.generate(
        DialogueProviderRequest(npc_id="npc-1", player_input="hello")
    )
    assert result == DialogueProviderResult(reply=STUB_REPLY, provider="stub")


def test_openai_selection_uses_injected_factory() -> None:
    settings = Settings.from_env({"OPENAI_API_KEY": "test-placeholder"})
    captured_settings: list[Settings] = []

    class FakeOpenAIProvider:
        def generate(
            self,
            _request: DialogueProviderRequest,
        ) -> DialogueProviderResult:
            return DialogueProviderResult(reply="fake", provider="openai")

    fake_provider = FakeOpenAIProvider()

    def build_fake(received_settings: Settings) -> FakeOpenAIProvider:
        captured_settings.append(received_settings)
        return fake_provider

    provider = create_dialogue_provider(settings, openai_factory=build_fake)

    assert provider is fake_provider
    assert captured_settings == [settings]


def test_openai_selection_never_falls_back_to_stub() -> None:
    settings = Settings.from_env({"OPENAI_API_KEY": "test-placeholder"})

    with pytest.raises(
        ProviderConfigurationError,
        match="OpenAI Provider implementation is unavailable",
    ):
        create_dialogue_provider(settings)
