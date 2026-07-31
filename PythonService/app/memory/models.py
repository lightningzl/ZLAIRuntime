"""Supplier-neutral internal types used at the memory persistence boundary."""

from dataclasses import dataclass
from enum import Enum


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
