"""Tests for the local-only, content-free Memory maintenance command."""

import json
from pathlib import Path

import pytest

from app.main import create_app
from app.memory.maintenance import main
from app.memory.models import DialogueTurnToStore
from app.memory.sqlite_repository import SQLiteDialogueMemoryRepository
from app.providers.stub_provider import StubDialogueProvider


def _populate(database_path: Path) -> None:
    repository = SQLiteDialogueMemoryRepository(database_path)
    repository.initialize()
    for turn in (
        DialogueTurnToStore(
            request_id="request-a1",
            scope_id="PRIVATE_SCOPE_A",
            npc_id="PRIVATE_NPC_A",
            player_input="PRIVATE_PLAYER_A1",
            npc_reply="PRIVATE_REPLY_A1",
        ),
        DialogueTurnToStore(
            request_id="request-a2",
            scope_id="PRIVATE_SCOPE_A",
            npc_id="PRIVATE_NPC_A",
            player_input="PRIVATE_PLAYER_A2",
            npc_reply="PRIVATE_REPLY_A2",
        ),
        DialogueTurnToStore(
            request_id="request-b1",
            scope_id="PRIVATE_SCOPE_B",
            npc_id="PRIVATE_NPC_B",
            player_input="PRIVATE_PLAYER_B1",
            npc_reply="PRIVATE_REPLY_B1",
        ),
    ):
        repository.store_turn(turn)
    repository.close()


def test_stats_outputs_only_aggregate_json(
    tmp_path: Path,
    capsys: pytest.CaptureFixture[str],
) -> None:
    database_path = tmp_path / "PRIVATE_DATABASE.sqlite3"
    _populate(database_path)

    exit_code = main(["--database-path", str(database_path), "stats"])

    captured = capsys.readouterr()
    assert exit_code == 0
    assert json.loads(captured.out) == {
        "command": "stats",
        "scope_count": 2,
        "scope_npc_pair_count": 2,
        "turn_count": 3,
    }
    assert captured.err == ""
    assert "PRIVATE_" not in captured.out


def test_clear_requires_confirmation() -> None:
    with pytest.raises(SystemExit):
        main(
            [
                "clear",
                "--scope-id",
                "PRIVATE_SCOPE_A",
                "--npc-id",
                "PRIVATE_NPC_A",
            ]
        )


@pytest.mark.parametrize(
    ("scope_id", "npc_id"),
    [
        ("", "npc"),
        (" scope", "npc"),
        ("scope", " "),
        ("scope", "npc "),
    ],
)
def test_clear_rejects_blank_or_surrounding_whitespace_identifiers(
    scope_id: str,
    npc_id: str,
) -> None:
    with pytest.raises(SystemExit):
        main(
            [
                "clear",
                "--scope-id",
                scope_id,
                "--npc-id",
                npc_id,
                "--confirm",
            ]
        )


def test_clear_deletes_only_exact_partition_and_does_not_echo_identifiers(
    tmp_path: Path,
    capsys: pytest.CaptureFixture[str],
) -> None:
    database_path = tmp_path / "memory.sqlite3"
    _populate(database_path)

    exit_code = main(
        [
            "--database-path",
            str(database_path),
            "clear",
            "--scope-id",
            "PRIVATE_SCOPE_A",
            "--npc-id",
            "PRIVATE_NPC_A",
            "--confirm",
        ]
    )

    captured = capsys.readouterr()
    assert exit_code == 0
    assert json.loads(captured.out) == {
        "command": "clear",
        "deleted_turn_count": 2,
    }
    assert "PRIVATE_" not in captured.out

    repository = SQLiteDialogueMemoryRepository(database_path)
    repository.initialize()
    assert repository.get_statistics().turn_count == 1
    assert len(
        repository.load_recent(
            scope_id="PRIVATE_SCOPE_B",
            npc_id="PRIVATE_NPC_B",
            limit=4,
        )
    ) == 1
    repository.close()


def test_database_failure_is_sanitized(
    tmp_path: Path,
    capsys: pytest.CaptureFixture[str],
) -> None:
    database_path = tmp_path / "PRIVATE_DATABASE_PATH.sqlite3"
    database_path.write_bytes(b"not a database")

    exit_code = main(["--database-path", str(database_path), "stats"])

    captured = capsys.readouterr()
    assert exit_code == 1
    assert captured.out == ""
    assert captured.err.strip() == "memory maintenance failed"
    assert "PRIVATE_DATABASE_PATH" not in captured.err


def test_maintenance_command_is_not_exposed_as_http_api() -> None:
    route_paths = set(
        create_app(provider=StubDialogueProvider()).openapi()["paths"].keys()
    )

    assert route_paths == {"/v1/dialogue", "/v1/decision", "/v2/decision"}
