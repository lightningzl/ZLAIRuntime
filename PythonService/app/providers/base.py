"""Supplier-neutral dialogue Provider contract."""

from dataclasses import dataclass
from typing import Literal, Protocol


ProviderId = Literal["stub", "openai"]


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
