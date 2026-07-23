"""Supplier-neutral exceptions raised inside the dialogue Provider boundary."""

from app.providers.base import ProviderId


class DialogueProviderError(RuntimeError):
    """Base exception for failures attributable to a dialogue Provider."""

    def __init__(self, provider: ProviderId, message: str) -> None:
        super().__init__(message)
        self.provider = provider


class ProviderAuthenticationError(DialogueProviderError):
    """Raised when Provider credentials are missing, invalid, or unauthorized."""


class ProviderRateLimitError(DialogueProviderError):
    """Raised when a Provider rejects a request because of rate or quota limits."""


class ProviderTimeoutError(DialogueProviderError):
    """Raised when a Provider request exceeds its configured timeout."""


class ProviderUnavailableError(DialogueProviderError):
    """Raised when a Provider cannot currently serve the configured request."""


class ProviderInvalidResponseError(DialogueProviderError):
    """Raised when a Provider returns no usable reply text."""
