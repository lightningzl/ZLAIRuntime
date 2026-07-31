"""Repository interface kept independent of SQLite and protocol schemas."""

from typing import Protocol

from app.memory.models import (
    DialogueTurnToStore,
    MemoryWriteResult,
    StoredDialogueTurn,
)


class MemoryRepositoryError(RuntimeError):
    """Raised when persistent memory cannot complete an operation safely."""


class DialogueMemoryRepository(Protocol):
    """Persistence operations required by the dialogue Memory Service."""

    def initialize(self) -> None:
        """Create or validate persistent storage without destroying existing data."""

    def load_recent(
        self,
        *,
        scope_id: str,
        npc_id: str,
        limit: int,
    ) -> tuple[StoredDialogueTurn, ...]:
        """Load at most ``limit`` turns, returned from oldest to newest."""

    def store_turn(self, turn: DialogueTurnToStore) -> MemoryWriteResult:
        """Persist one complete turn idempotently by request ID."""

    def close(self) -> None:
        """Release repository resources; repeated calls are safe."""
