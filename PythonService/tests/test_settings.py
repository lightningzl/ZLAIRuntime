"""Tests for centralized dialogue Provider configuration."""

import pytest

from app.core.settings import (
    DEFAULT_KIMI_MAX_OUTPUT_TOKENS,
    DEFAULT_KIMI_MODEL,
    DEFAULT_KIMI_TIMEOUT_SECONDS,
    KIMI_MAX_OUTPUT_TOKENS_HARD_LIMIT,
    UE_REQUEST_TIMEOUT_SECONDS,
    Settings,
    SettingsError,
)


def test_kimi_defaults_are_applied() -> None:
    settings = Settings.from_env({"MOONSHOT_API_KEY": "test-placeholder"})

    assert settings.dialogue_provider == "kimi"
    assert settings.kimi_model == DEFAULT_KIMI_MODEL
    assert settings.kimi_timeout_seconds == DEFAULT_KIMI_TIMEOUT_SECONDS
    assert settings.kimi_max_output_tokens == DEFAULT_KIMI_MAX_OUTPUT_TOKENS


def test_valid_environment_overrides_are_parsed() -> None:
    settings = Settings.from_env(
        {
            "ZL_DIALOGUE_PROVIDER": "KIMI",
            "MOONSHOT_API_KEY": "test-placeholder",
            "ZL_KIMI_MODEL": "test-model",
            "ZL_KIMI_TIMEOUT_SECONDS": "12.5",
            "ZL_KIMI_MAX_OUTPUT_TOKENS": "512",
        }
    )

    assert settings.dialogue_provider == "kimi"
    assert settings.kimi_model == "test-model"
    assert settings.kimi_timeout_seconds == 12.5
    assert settings.kimi_max_output_tokens == 512


def test_explicit_stub_mode_does_not_require_api_key() -> None:
    settings = Settings.from_env({"ZL_DIALOGUE_PROVIDER": "stub"})

    assert settings.dialogue_provider == "stub"
    assert settings.moonshot_api_key is None


@pytest.mark.parametrize(
    ("environment", "expected_message"),
    [
        ({"ZL_DIALOGUE_PROVIDER": "other"}, "ZL_DIALOGUE_PROVIDER"),
        ({}, "MOONSHOT_API_KEY"),
        (
            {"MOONSHOT_API_KEY": "test-placeholder", "ZL_KIMI_MODEL": " "},
            "ZL_KIMI_MODEL",
        ),
        (
            {
                "MOONSHOT_API_KEY": "test-placeholder",
                "ZL_KIMI_TIMEOUT_SECONDS": "invalid",
            },
            "ZL_KIMI_TIMEOUT_SECONDS",
        ),
        (
            {
                "MOONSHOT_API_KEY": "test-placeholder",
                "ZL_KIMI_TIMEOUT_SECONDS": "0",
            },
            "ZL_KIMI_TIMEOUT_SECONDS",
        ),
        (
            {
                "MOONSHOT_API_KEY": "test-placeholder",
                "ZL_KIMI_TIMEOUT_SECONDS": str(UE_REQUEST_TIMEOUT_SECONDS),
            },
            "UE request timeout",
        ),
        (
            {
                "MOONSHOT_API_KEY": "test-placeholder",
                "ZL_KIMI_MAX_OUTPUT_TOKENS": "1.5",
            },
            "ZL_KIMI_MAX_OUTPUT_TOKENS",
        ),
        (
            {
                "MOONSHOT_API_KEY": "test-placeholder",
                "ZL_KIMI_MAX_OUTPUT_TOKENS": "0",
            },
            "ZL_KIMI_MAX_OUTPUT_TOKENS",
        ),
        (
            {
                "MOONSHOT_API_KEY": "test-placeholder",
                "ZL_KIMI_MAX_OUTPUT_TOKENS": str(
                    KIMI_MAX_OUTPUT_TOKENS_HARD_LIMIT + 1
                ),
            },
            "hard limit",
        ),
    ],
)
def test_invalid_configuration_fails_safely(
    environment: dict[str, str],
    expected_message: str,
) -> None:
    with pytest.raises(SettingsError, match=expected_message):
        Settings.from_env(environment)


def test_api_key_is_excluded_from_repr_and_validation_errors() -> None:
    secret = "sensitive-placeholder"
    settings = Settings.from_env(
        {"ZL_DIALOGUE_PROVIDER": "stub", "MOONSHOT_API_KEY": secret}
    )

    assert secret not in repr(settings)

    with pytest.raises(SettingsError) as error:
        Settings.from_env(
            {
                "MOONSHOT_API_KEY": secret,
                "ZL_KIMI_TIMEOUT_SECONDS": "invalid",
            }
        )
    assert secret not in str(error.value)
