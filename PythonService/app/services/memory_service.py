"""Deterministic dialogue memory retrieval, merging, and write semantics."""

from typing import Protocol

from app.memory.base import DialogueMemoryRepository
from app.memory.models import (
    DialogueMemoryMessage,
    DialogueTurnToStore,
    MemoryWriteResult,
    MergedDialogueHistory,
    StoredDialogueTurn,
)


class MemoryService(Protocol):
    """Dialogue orchestration boundary independent of persistent storage details."""

    def load_merged_history(
        self,
        *,
        scope_id: str,
        npc_id: str,
        client_history: tuple[DialogueMemoryMessage, ...],
    ) -> MergedDialogueHistory:
        """Load an isolated persistent window and merge it with client history."""

    def store_completed_turn(
        self,
        *,
        request_id: str,
        scope_id: str,
        npc_id: str,
        player_input: str,
        npc_reply: str,
    ) -> MemoryWriteResult:
        """Idempotently store one complete, already successful dialogue turn."""


class DialogueMemoryService:
    """Apply bounded retrieval and exact boundary-overlap elimination."""

    def __init__(
        self,
        repository: DialogueMemoryRepository,
        *,
        max_turns: int,
    ) -> None:
        if (
            isinstance(max_turns, bool)
            or not isinstance(max_turns, int)
            or max_turns <= 0
        ):
            raise ValueError("max_turns must be a positive integer")
        self._repository = repository
        self._max_turns = max_turns

    def load_merged_history(
        self,
        *,
        scope_id: str,
        npc_id: str,
        client_history: tuple[DialogueMemoryMessage, ...],
    ) -> MergedDialogueHistory:
        """Place persistent history before the newer client snapshot."""
        stored_turns = self._repository.load_recent(
            scope_id=scope_id,
            npc_id=npc_id,
            limit=self._max_turns,
        )
        matching_turns = sorted(
            (
                turn
                for turn in stored_turns
                if turn.scope_id == scope_id and turn.npc_id == npc_id
            ),
            key=lambda turn: turn.sequence,
        )
        bounded_turns = tuple(matching_turns[-self._max_turns :])
        persistent_history = self._flatten_turns(bounded_turns)
        overlap_size = self._find_boundary_overlap(
            persistent_history,
            client_history,
        )
        persistent_prefix = (
            persistent_history[:-overlap_size]
            if overlap_size
            else persistent_history
        )
        return MergedDialogueHistory(
            messages=persistent_prefix + client_history,
            retrieved_turn_count=len(bounded_turns),
        )

    def store_completed_turn(
        self,
        *,
        request_id: str,
        scope_id: str,
        npc_id: str,
        player_input: str,
        npc_reply: str,
    ) -> MemoryWriteResult:
        """Delegate one complete turn to the repository's atomic write boundary."""
        return self._repository.store_turn(
            DialogueTurnToStore(
                request_id=request_id,
                scope_id=scope_id,
                npc_id=npc_id,
                player_input=player_input,
                npc_reply=npc_reply,
            )
        )

    @staticmethod
    def _flatten_turns(
        stored_turns: tuple[StoredDialogueTurn, ...],
    ) -> tuple[DialogueMemoryMessage, ...]:
        messages: list[DialogueMemoryMessage] = []
        for turn in stored_turns:
            messages.extend(
                (
                    DialogueMemoryMessage(role="player", content=turn.player_input),
                    DialogueMemoryMessage(role="npc", content=turn.npc_reply),
                )
            )
        return tuple(messages)

    @staticmethod
    def _find_boundary_overlap(
        persistent_history: tuple[DialogueMemoryMessage, ...],
        client_history: tuple[DialogueMemoryMessage, ...],
    ) -> int:
        maximum = min(len(persistent_history), len(client_history))
        for size in range(maximum, 0, -1):
            if persistent_history[-size:] == client_history[:size]:
                return size
        return 0
