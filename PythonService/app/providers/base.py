"""Supplier-neutral dialogue Provider contract."""

from dataclasses import dataclass
from typing import Literal, Protocol


ProviderId = Literal["stub", "kimi"]
DialogueMessageRole = Literal["user", "assistant"]


@dataclass(frozen=True, slots=True)
class DialogueGenerationMessage:
    """One supplier-neutral conversational message."""

    role: DialogueMessageRole
    content: str


@dataclass(frozen=True, slots=True)
class DialogueGenerationContext:
    """Deterministic supplier-neutral input assembled for one generation."""

    system_instructions: str
    context_data_json: str
    messages: tuple[DialogueGenerationMessage, ...]


@dataclass(frozen=True, slots=True)
class DialogueProviderRequest:
    """Minimum internal input needed to generate one dialogue reply."""

    npc_id: str
    player_input: str


@dataclass(frozen=True, slots=True)
class DialogueProviderResult:
    """Supplier-neutral result returned by a dialogue Provider."""

    reply: str
    provider: ProviderId


class DialogueProvider(Protocol):
    """Generate one non-streaming reply without HTTP or protocol model coupling."""

    def generate(self, request: DialogueProviderRequest) -> DialogueProviderResult:
        """Generate a single dialogue result."""
        ...
