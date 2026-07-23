"""Tests for explicit Provider selection and injection boundaries."""

from app.core.settings import Settings
from app.providers.base import (
    DialogueProviderRequest,
    DialogueProviderResult,
)
from app.providers.factory import (
    create_dialogue_provider,
)
from app.providers.kimi_provider import KimiDialogueProvider
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


def test_kimi_selection_uses_injected_factory() -> None:
    settings = Settings.from_env({"MOONSHOT_API_KEY": "test-placeholder"})
    captured_settings: list[Settings] = []

    class FakeKimiProvider:
        def generate(
            self,
            _request: DialogueProviderRequest,
        ) -> DialogueProviderResult:
            return DialogueProviderResult(reply="fake", provider="kimi")

    fake_provider = FakeKimiProvider()

    def build_fake(received_settings: Settings) -> FakeKimiProvider:
        captured_settings.append(received_settings)
        return fake_provider

    provider = create_dialogue_provider(settings, kimi_factory=build_fake)

    assert provider is fake_provider
    assert captured_settings == [settings]


def test_kimi_selection_constructs_kimi_provider_without_network() -> None:
    settings = Settings.from_env({"MOONSHOT_API_KEY": "test-placeholder"})

    provider = create_dialogue_provider(settings)

    assert isinstance(provider, KimiDialogueProvider)
