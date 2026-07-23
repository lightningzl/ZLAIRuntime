"""Supplier-neutral exceptions raised inside the dialogue Provider boundary."""


class DialogueProviderError(RuntimeError):
    """Base exception for failures attributable to a dialogue Provider."""


class ProviderInvalidResponseError(DialogueProviderError):
    """Raised when a Provider returns no usable reply text."""
