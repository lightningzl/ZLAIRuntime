"""Local-only aggregate inspection and exact-partition cleanup commands."""

import argparse
import json
from pathlib import Path
import sys
from typing import Sequence

from app.core.settings import DEFAULT_MEMORY_DATABASE_PATH
from app.memory.base import MemoryRepositoryError
from app.memory.sqlite_repository import SQLiteDialogueMemoryRepository


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Inspect or clean local dialogue memory without printing content.",
    )
    parser.add_argument(
        "--database-path",
        type=Path,
        default=DEFAULT_MEMORY_DATABASE_PATH,
        help="SQLite database path (defaults to the runtime database path).",
    )
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("stats", help="Print aggregate counts only.")

    clear_parser = commands.add_parser(
        "clear",
        help="Delete one exact scope/NPC partition.",
    )
    clear_parser.add_argument("--scope-id", required=True)
    clear_parser.add_argument("--npc-id", required=True)
    clear_parser.add_argument(
        "--confirm",
        action="store_true",
        help="Confirm the destructive partition cleanup.",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    """Run one maintenance operation and emit only stable aggregate output."""
    parser = _parser()
    arguments = parser.parse_args(argv)
    if arguments.command == "clear":
        if not arguments.confirm:
            parser.error("clear requires --confirm")
        if not arguments.scope_id.strip() or arguments.scope_id != (
            arguments.scope_id.strip()
        ):
            parser.error("scope ID must be nonblank without surrounding whitespace")
        if not arguments.npc_id.strip() or arguments.npc_id != arguments.npc_id.strip():
            parser.error("NPC ID must be nonblank without surrounding whitespace")

    repository = SQLiteDialogueMemoryRepository(arguments.database_path)
    output: dict[str, int | str]
    try:
        repository.initialize()
        if arguments.command == "stats":
            statistics = repository.get_statistics()
            output = {
                "command": "stats",
                "scope_count": statistics.scope_count,
                "scope_npc_pair_count": statistics.scope_npc_pair_count,
                "turn_count": statistics.turn_count,
            }
        else:
            deleted_turn_count = repository.delete_scope_npc(
                scope_id=arguments.scope_id,
                npc_id=arguments.npc_id,
            )
            output = {
                "command": "clear",
                "deleted_turn_count": deleted_turn_count,
            }
        repository.close()
    except MemoryRepositoryError:
        try:
            repository.close()
        except MemoryRepositoryError:
            pass
        print("memory maintenance failed", file=sys.stderr)
        return 1

    print(json.dumps(output, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
