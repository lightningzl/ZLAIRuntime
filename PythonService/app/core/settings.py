"""Validated runtime settings loaded exclusively from process environment variables."""

from collections.abc import Mapping
from dataclasses import dataclass, field
import math
import os
from typing import Literal, cast


DEFAULT_OPENAI_MODEL = "gpt-5.6-luna"
DEFAULT_OPENAI_TIMEOUT_SECONDS = 20.0
DEFAULT_OPENAI_MAX_OUTPUT_TOKENS = 256
OPENAI_MAX_OUTPUT_TOKENS_HARD_LIMIT = 4096
UE_REQUEST_TIMEOUT_SECONDS = 30.0

DialogueProviderName = Literal["openai", "stub"]


class SettingsError(ValueError):
    """Raised when runtime configuration is missing or invalid."""


def _parse_float(source: Mapping[str, str], name: str, default: float) -> float:
    raw_value = source.get(name)
    if raw_value is None:
        return default

    try:
        value = float(raw_value)
    except ValueError:
        raise SettingsError(f"{name} must be a number") from None

    if not math.isfinite(value):
        raise SettingsError(f"{name} must be a finite number")
    return value


def _parse_int(source: Mapping[str, str], name: str, default: int) -> int:
    raw_value = source.get(name)
    if raw_value is None:
        return default

    try:
        return int(raw_value)
    except ValueError:
        raise SettingsError(f"{name} must be an integer") from None


@dataclass(frozen=True, slots=True)
class Settings:
    """Central validated settings for the dialogue Provider boundary."""

    dialogue_provider: DialogueProviderName = "openai"
    openai_api_key: str | None = field(default=None, repr=False)
    openai_model: str = DEFAULT_OPENAI_MODEL
    openai_timeout_seconds: float = DEFAULT_OPENAI_TIMEOUT_SECONDS
    openai_max_output_tokens: int = DEFAULT_OPENAI_MAX_OUTPUT_TOKENS

    def __post_init__(self) -> None:
        if self.dialogue_provider not in ("openai", "stub"):
            raise SettingsError("ZL_DIALOGUE_PROVIDER must be 'openai' or 'stub'")
        if not isinstance(self.openai_model, str) or not self.openai_model.strip():
            raise SettingsError("ZL_OPENAI_MODEL must not be empty")
        if (
            isinstance(self.openai_timeout_seconds, bool)
            or not isinstance(self.openai_timeout_seconds, (int, float))
            or not math.isfinite(self.openai_timeout_seconds)
        ):
            raise SettingsError("ZL_OPENAI_TIMEOUT_SECONDS must be a finite number")
        if self.openai_timeout_seconds <= 0:
            raise SettingsError("ZL_OPENAI_TIMEOUT_SECONDS must be positive")
        if self.openai_timeout_seconds >= UE_REQUEST_TIMEOUT_SECONDS:
            raise SettingsError(
                "ZL_OPENAI_TIMEOUT_SECONDS must be less than the UE request timeout"
            )
        if isinstance(self.openai_max_output_tokens, bool) or not isinstance(
            self.openai_max_output_tokens, int
        ):
            raise SettingsError("ZL_OPENAI_MAX_OUTPUT_TOKENS must be an integer")
        if self.openai_max_output_tokens <= 0:
            raise SettingsError("ZL_OPENAI_MAX_OUTPUT_TOKENS must be positive")
        if self.openai_max_output_tokens > OPENAI_MAX_OUTPUT_TOKENS_HARD_LIMIT:
            raise SettingsError(
                "ZL_OPENAI_MAX_OUTPUT_TOKENS exceeds the supported hard limit"
            )
        if self.dialogue_provider == "openai" and (
            not isinstance(self.openai_api_key, str)
            or not self.openai_api_key.strip()
        ):
            raise SettingsError(
                "OPENAI_API_KEY is required when ZL_DIALOGUE_PROVIDER=openai"
            )

    @classmethod
    def from_env(cls, environ: Mapping[str, str] | None = None) -> "Settings":
        """Load settings without retaining or exposing the source environment."""
        source = os.environ if environ is None else environ
        provider_value = source.get("ZL_DIALOGUE_PROVIDER", "openai").strip().lower()
        if provider_value not in ("openai", "stub"):
            raise SettingsError("ZL_DIALOGUE_PROVIDER must be 'openai' or 'stub'")

        raw_api_key = source.get("OPENAI_API_KEY")
        api_key = raw_api_key.strip() if raw_api_key else None
        model = source.get("ZL_OPENAI_MODEL", DEFAULT_OPENAI_MODEL).strip()

        return cls(
            dialogue_provider=cast(DialogueProviderName, provider_value),
            openai_api_key=api_key,
            openai_model=model,
            openai_timeout_seconds=_parse_float(
                source,
                "ZL_OPENAI_TIMEOUT_SECONDS",
                DEFAULT_OPENAI_TIMEOUT_SECONDS,
            ),
            openai_max_output_tokens=_parse_int(
                source,
                "ZL_OPENAI_MAX_OUTPUT_TOKENS",
                DEFAULT_OPENAI_MAX_OUTPUT_TOKENS,
            ),
        )
