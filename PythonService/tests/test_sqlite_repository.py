"""Tests for the isolated SQLite dialogue memory repository."""

from pathlib import Path
import sqlite3

import pytest

from app.memory.base import MemoryRepositoryError
from app.memory.models import DialogueTurnToStore, MemoryWriteResult
from app.memory.sqlite_repository import (
    SCHEMA_VERSION,
    SQLiteDialogueMemoryRepository,
)


def _turn(
    request_id: str,
    *,
    scope_id: str = "save-a",
    npc_id: str = "npc-keeper",
    player_input: str | None = None,
    npc_reply: str | None = None,
) -> DialogueTurnToStore:
    return DialogueTurnToStore(
        request_id=request_id,
        scope_id=scope_id,
        npc_id=npc_id,
        player_input=(
            f"player-{request_id}" if player_input is None else player_input
        ),
        npc_reply=f"npc-{request_id}" if npc_reply is None else npc_reply,
    )


def _repository(database_path: Path) -> SQLiteDialogueMemoryRepository:
    repository = SQLiteDialogueMemoryRepository(database_path)
    repository.initialize()
    return repository


def test_empty_database_initializes_schema_version_table_and_index(
    tmp_path: Path,
) -> None:
    database_path = tmp_path / "nested" / "memory.sqlite3"
    repository = _repository(database_path)
    repository.close()

    with sqlite3.connect(database_path) as connection:
        version = connection.execute("PRAGMA user_version").fetchone()[0]
        objects = dict(
            connection.execute(
                "SELECT name, type FROM sqlite_master WHERE name IN (?, ?)",
                (
                    "dialogue_memory_turns",
                    "idx_dialogue_memory_scope_npc_sequence",
                ),
            ).fetchall()
        )

    assert version == SCHEMA_VERSION
    assert objects == {
        "dialogue_memory_turns": "table",
        "idx_dialogue_memory_scope_npc_sequence": "index",
    }


def test_repeated_initialization_preserves_existing_turn(tmp_path: Path) -> None:
    repository = _repository(tmp_path / "memory.sqlite3")
    assert repository.store_turn(_turn("request-1")) is MemoryWriteResult.INSERTED

    repository.initialize()

    assert [turn.request_id for turn in repository.load_recent(
        scope_id="save-a", npc_id="npc-keeper", limit=4
    )] == ["request-1"]
    repository.close()


def test_restart_reopens_and_reads_existing_turns(tmp_path: Path) -> None:
    database_path = tmp_path / "memory.sqlite3"
    first = _repository(database_path)
    first.store_turn(_turn("request-1"))
    first.close()

    second = _repository(database_path)
    loaded = second.load_recent(scope_id="save-a", npc_id="npc-keeper", limit=4)
    second.close()

    assert [(turn.request_id, turn.sequence) for turn in loaded] == [
        ("request-1", 1)
    ]


def test_recent_limit_returns_oldest_to_newest_with_stable_sequence(
    tmp_path: Path,
) -> None:
    repository = _repository(tmp_path / "memory.sqlite3")
    for index in range(1, 6):
        repository.store_turn(_turn(f"request-{index}"))

    loaded = repository.load_recent(
        scope_id="save-a",
        npc_id="npc-keeper",
        limit=3,
    )
    repository.close()

    assert [turn.request_id for turn in loaded] == [
        "request-3",
        "request-4",
        "request-5",
    ]
    assert [turn.sequence for turn in loaded] == sorted(
        turn.sequence for turn in loaded
    )


def test_scope_and_npc_are_both_isolated(tmp_path: Path) -> None:
    repository = _repository(tmp_path / "memory.sqlite3")
    repository.store_turn(_turn("matching"))
    repository.store_turn(_turn("other-scope", scope_id="save-b"))
    repository.store_turn(_turn("other-npc", npc_id="npc-smith"))

    loaded = repository.load_recent(
        scope_id="save-a",
        npc_id="npc-keeper",
        limit=8,
    )
    repository.close()

    assert [turn.request_id for turn in loaded] == ["matching"]


def test_duplicate_request_id_is_idempotent_across_all_scopes(tmp_path: Path) -> None:
    repository = _repository(tmp_path / "memory.sqlite3")

    first = repository.store_turn(_turn("same-request"))
    duplicate = repository.store_turn(
        _turn("same-request", scope_id="save-b", npc_id="npc-smith")
    )

    assert first is MemoryWriteResult.INSERTED
    assert duplicate is MemoryWriteResult.DUPLICATE_REQUEST
    assert len(
        repository.load_recent(scope_id="save-a", npc_id="npc-keeper", limit=4)
    ) == 1
    assert repository.load_recent(
        scope_id="save-b", npc_id="npc-smith", limit=4
    ) == ()
    repository.close()


def test_failed_write_rolls_back_without_leaving_partial_turn(tmp_path: Path) -> None:
    repository = _repository(tmp_path / "memory.sqlite3")
    repository.store_turn(_turn("valid"))

    with pytest.raises(MemoryRepositoryError, match="write failed"):
        repository.store_turn(_turn("invalid", npc_reply=""))

    loaded = repository.load_recent(
        scope_id="save-a",
        npc_id="npc-keeper",
        limit=8,
    )
    repository.close()
    assert [turn.request_id for turn in loaded] == ["valid"]


def test_corrupt_database_fails_without_leaking_path(tmp_path: Path) -> None:
    database_path = tmp_path / "private-memory.sqlite3"
    database_path.write_bytes(b"not a sqlite database")
    repository = SQLiteDialogueMemoryRepository(database_path)

    with pytest.raises(MemoryRepositoryError) as error:
        repository.initialize()

    assert str(database_path) not in str(error.value)


def test_invalid_database_parent_fails_without_leaking_path(tmp_path: Path) -> None:
    parent_file = tmp_path / "not-a-directory"
    parent_file.write_text("occupied", encoding="utf-8")
    database_path = parent_file / "memory.sqlite3"
    repository = SQLiteDialogueMemoryRepository(database_path)

    with pytest.raises(MemoryRepositoryError) as error:
        repository.initialize()

    assert str(database_path) not in str(error.value)


def test_unsupported_schema_version_is_rejected(tmp_path: Path) -> None:
    database_path = tmp_path / "future.sqlite3"
    with sqlite3.connect(database_path) as connection:
        connection.execute(f"PRAGMA user_version = {SCHEMA_VERSION + 1}")

    repository = SQLiteDialogueMemoryRepository(database_path)
    with pytest.raises(MemoryRepositoryError, match="unsupported"):
        repository.initialize()


@pytest.mark.parametrize("invalid_limit", [0, -1, True, 1.5])
def test_invalid_retrieval_limit_is_rejected(
    tmp_path: Path,
    invalid_limit: object,
) -> None:
    repository = _repository(tmp_path / "memory.sqlite3")

    with pytest.raises(ValueError, match="positive integer"):
        repository.load_recent(
            scope_id="save-a",
            npc_id="npc-keeper",
            limit=invalid_limit,  # type: ignore[arg-type]
        )
    repository.close()


def test_operations_require_initialization_and_close_is_idempotent(
    tmp_path: Path,
) -> None:
    repository = SQLiteDialogueMemoryRepository(tmp_path / "memory.sqlite3")
    with pytest.raises(MemoryRepositoryError, match="not initialized"):
        repository.load_recent(scope_id="save-a", npc_id="npc-keeper", limit=1)

    repository.initialize()
    repository.close()
    repository.close()

    with pytest.raises(MemoryRepositoryError, match="not initialized"):
        repository.store_turn(_turn("after-close"))
