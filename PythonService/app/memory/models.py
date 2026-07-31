"""Supplier-neutral internal types used at the memory persistence boundary."""

from dataclasses import dataclass
from enum import Enum
from typing import Literal


DialogueMemoryRole = Literal["player", "npc"]


@dataclass(frozen=True, slots=True)
class DialogueMemoryMessage:
    """One ordered, untrusted dialogue message used during history merging."""

    role: DialogueMemoryRole
    content: str


@dataclass(frozen=True, slots=True)
class MergedDialogueHistory:
    """Merged history plus the number of persistent turns retrieved."""

    messages: tuple[DialogueMemoryMessage, ...]
    retrieved_turn_count: int


@dataclass(frozen=True, slots=True)
class DialogueTurnToStore:
    """One complete successful dialogue turn awaiting persistence."""

    request_id: str
    scope_id: str
    npc_id: str
    player_input: str
    npc_reply: str


@dataclass(frozen=True, slots=True)
class StoredDialogueTurn:
    """One persisted dialogue turn with its stable database order."""

    sequence: int
    request_id: str
    scope_id: str
    npc_id: str
    player_input: str
    npc_reply: str


class MemoryWriteResult(Enum):
    """Idempotent result of attempting to persist a complete turn."""

    INSERTED = "inserted"
    DUPLICATE_REQUEST = "duplicate_request"
