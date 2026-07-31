"""Tests for bounded, isolated, deterministic dialogue memory semantics."""

from pathlib import Path

import pytest

from app.memory.base import MemoryRepositoryError
from app.memory.models import (
    DialogueMemoryMessage,
    DialogueTurnToStore,
    MemoryWriteResult,
    StoredDialogueTurn,
)
from app.memory.sqlite_repository import SQLiteDialogueMemoryRepository
from app.services.memory_service import DialogueMemoryService


class FakeMemoryRepository:
    def __init__(
        self,
        *,
        turns: tuple[StoredDialogueTurn, ...] = (),
        write_result: MemoryWriteResult = MemoryWriteResult.INSERTED,
    ) -> None:
        self.turns = turns
        self.write_result = write_result
        self.load_calls: list[tuple[str, str, int]] = []
        self.stored_turns: list[DialogueTurnToStore] = []
        self.read_error: MemoryRepositoryError | None = None
        self.write_error: MemoryRepositoryError | None = None

    def initialize(self) -> None:
        pass

    def load_recent(
        self,
        *,
        scope_id: str,
        npc_id: str,
        limit: int,
    ) -> tuple[StoredDialogueTurn, ...]:
        self.load_calls.append((scope_id, npc_id, limit))
        if self.read_error is not None:
            raise self.read_error
        return self.turns

    def store_turn(self, turn: DialogueTurnToStore) -> MemoryWriteResult:
        self.stored_turns.append(turn)
        if self.write_error is not None:
            raise self.write_error
        return self.write_result

    def close(self) -> None:
        pass


def _stored_turn(
    sequence: int,
    *,
    request_id: str | None = None,
    scope_id: str = "save-a",
    npc_id: str = "npc-keeper",
    player_input: str | None = None,
    npc_reply: str | None = None,
) -> StoredDialogueTurn:
    return StoredDialogueTurn(
        sequence=sequence,
        request_id=request_id or f"request-{sequence}",
        scope_id=scope_id,
        npc_id=npc_id,
        player_input=player_input or f"player-{sequence}",
        npc_reply=npc_reply or f"npc-{sequence}",
    )


def _messages(*items: tuple[str, str]) -> tuple[DialogueMemoryMessage, ...]:
    return tuple(
        DialogueMemoryMessage(role=role, content=content)  # type: ignore[arg-type]
        for role, content in items
    )


def test_empty_memory_preserves_client_history_and_uses_configured_budget() -> None:
    repository = FakeMemoryRepository()
    service = DialogueMemoryService(repository, max_turns=4)
    client_history = _messages(("player", "client question"), ("npc", "answer"))

    result = service.load_merged_history(
        scope_id="opaque';--",
        npc_id="npc-keeper",
        client_history=client_history,
    )

    assert repository.load_calls == [("opaque';--", "npc-keeper", 4)]
    assert result.messages == client_history
    assert result.retrieved_turn_count == 0


def test_service_filters_scope_and_npc_then_sorts_and_bounds_latest_turns() -> None:
    repository = FakeMemoryRepository(
        turns=(
            _stored_turn(4),
            _stored_turn(2),
            _stored_turn(5, scope_id="save-b"),
            _stored_turn(3),
            _stored_turn(6, npc_id="npc-smith"),
            _stored_turn(1),
        )
    )
    service = DialogueMemoryService(repository, max_turns=2)

    result = service.load_merged_history(
        scope_id="save-a",
        npc_id="npc-keeper",
        client_history=(),
    )

    assert result.retrieved_turn_count == 2
    assert result.messages == _messages(
        ("player", "player-3"),
        ("npc", "npc-3"),
        ("player", "player-4"),
        ("npc", "npc-4"),
    )


@pytest.mark.parametrize(
    ("client_history", "expected"),
    [
        (
            _messages(
                ("player", "p1"),
                ("npc", "n1"),
                ("player", "p2"),
                ("npc", "n2"),
                ("player", "new"),
            ),
            _messages(
                ("player", "p1"),
                ("npc", "n1"),
                ("player", "p2"),
                ("npc", "n2"),
                ("player", "new"),
            ),
        ),
        (
            _messages(
                ("player", "p2"),
                ("npc", "n2"),
                ("player", "new"),
            ),
            _messages(
                ("player", "p1"),
                ("npc", "n1"),
                ("player", "p2"),
                ("npc", "n2"),
                ("player", "new"),
            ),
        ),
        (
            _messages(("npc", "n2"), ("player", "new")),
            _messages(
                ("player", "p1"),
                ("npc", "n1"),
                ("player", "p2"),
                ("npc", "n2"),
                ("player", "new"),
            ),
        ),
    ],
)
def test_exact_boundary_overlap_is_removed_with_client_history_priority(
    client_history: tuple[DialogueMemoryMessage, ...],
    expected: tuple[DialogueMemoryMessage, ...],
) -> None:
    repository = FakeMemoryRepository(
        turns=(
            _stored_turn(1, player_input="p1", npc_reply="n1"),
            _stored_turn(2, player_input="p2", npc_reply="n2"),
        )
    )
    service = DialogueMemoryService(repository, max_turns=4)

    result = service.load_merged_history(
        scope_id="save-a",
        npc_id="npc-keeper",
        client_history=client_history,
    )

    assert result.messages == expected
    assert result.messages[-len(client_history) :] == client_history
    assert all(
        merged is supplied
        for merged, supplied in zip(
            result.messages[-len(client_history) :],
            client_history,
            strict=True,
        )
    )


def test_same_content_with_different_role_is_not_deduplicated() -> None:
    repository = FakeMemoryRepository(
        turns=(_stored_turn(1, player_input="question", npc_reply="same"),)
    )
    service = DialogueMemoryService(repository, max_turns=4)

    result = service.load_merged_history(
        scope_id="save-a",
        npc_id="npc-keeper",
        client_history=_messages(("player", "same")),
    )

    assert result.messages == _messages(
        ("player", "question"),
        ("npc", "same"),
        ("player", "same"),
    )


def test_matching_content_away_from_boundary_is_not_deduplicated() -> None:
    repository = FakeMemoryRepository(
        turns=(_stored_turn(1, player_input="repeat", npc_reply="old reply"),)
    )
    service = DialogueMemoryService(repository, max_turns=4)

    result = service.load_merged_history(
        scope_id="save-a",
        npc_id="npc-keeper",
        client_history=_messages(
            ("player", "repeat"),
            ("npc", "newer reply"),
        ),
    )

    assert [message.content for message in result.messages].count("repeat") == 2


def test_completed_turn_write_preserves_all_fields_and_result() -> None:
    repository = FakeMemoryRepository(
        write_result=MemoryWriteResult.DUPLICATE_REQUEST
    )
    service = DialogueMemoryService(repository, max_turns=4)

    result = service.store_completed_turn(
        request_id="request-1",
        scope_id="save-a",
        npc_id="npc-keeper",
        player_input="question",
        npc_reply="answer",
    )

    assert result is MemoryWriteResult.DUPLICATE_REQUEST
    assert repository.stored_turns == [
        DialogueTurnToStore(
            request_id="request-1",
            scope_id="save-a",
            npc_id="npc-keeper",
            player_input="question",
            npc_reply="answer",
        )
    ]


def test_repository_errors_propagate_without_translation() -> None:
    repository = FakeMemoryRepository()
    service = DialogueMemoryService(repository, max_turns=4)
    read_error = MemoryRepositoryError("safe read failure")
    repository.read_error = read_error

    with pytest.raises(MemoryRepositoryError) as raised_read:
        service.load_merged_history(
            scope_id="save-a",
            npc_id="npc-keeper",
            client_history=(),
        )
    assert raised_read.value is read_error

    write_error = MemoryRepositoryError("safe write failure")
    repository.write_error = write_error
    with pytest.raises(MemoryRepositoryError) as raised_write:
        service.store_completed_turn(
            request_id="request-1",
            scope_id="save-a",
            npc_id="npc-keeper",
            player_input="question",
            npc_reply="answer",
        )
    assert raised_write.value is write_error


@pytest.mark.parametrize("invalid_max_turns", [0, -1, True, 1.5])
def test_invalid_max_turns_is_rejected(invalid_max_turns: object) -> None:
    with pytest.raises(ValueError, match="positive integer"):
        DialogueMemoryService(
            FakeMemoryRepository(),
            max_turns=invalid_max_turns,  # type: ignore[arg-type]
        )


def test_real_repository_integration_persists_and_merges_history(
    tmp_path: Path,
) -> None:
    repository = SQLiteDialogueMemoryRepository(tmp_path / "memory.sqlite3")
    repository.initialize()
    service = DialogueMemoryService(repository, max_turns=4)
    service.store_completed_turn(
        request_id="request-1",
        scope_id="save-a",
        npc_id="npc-keeper",
        player_input="question",
        npc_reply="answer",
    )

    result = service.load_merged_history(
        scope_id="save-a",
        npc_id="npc-keeper",
        client_history=(),
    )
    repository.close()

    assert result.messages == _messages(
        ("player", "question"),
        ("npc", "answer"),
    )
    assert result.retrieved_turn_count == 1
