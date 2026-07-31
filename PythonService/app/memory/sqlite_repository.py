"""SQLite implementation of the dialogue memory persistence boundary."""

from pathlib import Path
import sqlite3
from threading import RLock

from app.memory.base import MemoryRepositoryError
from app.memory.models import (
    DialogueTurnToStore,
    MemoryWriteResult,
    MemoryStatistics,
    StoredDialogueTurn,
)


SCHEMA_VERSION = 1
_TABLE_NAME = "dialogue_memory_turns"
_SCOPE_INDEX_NAME = "idx_dialogue_memory_scope_npc_sequence"

_CREATE_TABLE_SQL = f"""
CREATE TABLE IF NOT EXISTS {_TABLE_NAME} (
    sequence INTEGER PRIMARY KEY AUTOINCREMENT,
    request_id TEXT NOT NULL UNIQUE CHECK(length(request_id) > 0),
    scope_id TEXT NOT NULL CHECK(length(scope_id) > 0),
    npc_id TEXT NOT NULL CHECK(length(npc_id) > 0),
    player_input TEXT NOT NULL CHECK(length(player_input) > 0),
    npc_reply TEXT NOT NULL CHECK(length(npc_reply) > 0),
    created_at_utc TEXT NOT NULL DEFAULT (
        strftime('%Y-%m-%dT%H:%M:%fZ', 'now')
    )
)
"""

_CREATE_SCOPE_INDEX_SQL = f"""
CREATE INDEX IF NOT EXISTS {_SCOPE_INDEX_NAME}
ON {_TABLE_NAME} (scope_id, npc_id, sequence DESC)
"""

_INSERT_TURN_SQL = f"""
INSERT INTO {_TABLE_NAME} (
    request_id,
    scope_id,
    npc_id,
    player_input,
    npc_reply
) VALUES (?, ?, ?, ?, ?)
ON CONFLICT(request_id) DO NOTHING
"""

_LOAD_RECENT_SQL = f"""
SELECT sequence, request_id, scope_id, npc_id, player_input, npc_reply
FROM (
    SELECT sequence, request_id, scope_id, npc_id, player_input, npc_reply
    FROM {_TABLE_NAME}
    WHERE scope_id = ? AND npc_id = ?
    ORDER BY sequence DESC
    LIMIT ?
)
ORDER BY sequence ASC
"""

_SELECT_STATISTICS_SQL = f"""
SELECT
    COUNT(*) AS turn_count,
    COUNT(DISTINCT scope_id) AS scope_count,
    (
        SELECT COUNT(*)
        FROM (
            SELECT DISTINCT scope_id, npc_id
            FROM {_TABLE_NAME}
        )
    ) AS scope_npc_pair_count
FROM {_TABLE_NAME}
"""

_DELETE_SCOPE_NPC_SQL = f"""
DELETE FROM {_TABLE_NAME}
WHERE scope_id = ? AND npc_id = ?
"""


class SQLiteDialogueMemoryRepository:
    """Own the SQLite connection, schema, SQL, transactions, and row mapping."""

    def __init__(self, database_path: Path) -> None:
        self._database_path = Path(database_path)
        self._connection: sqlite3.Connection | None = None
        self._lock = RLock()

    def initialize(self) -> None:
        """Open the database and idempotently install the supported schema."""
        with self._lock:
            if self._connection is None:
                connection = self._open_connection()
                self._connection = connection
            else:
                connection = self._connection

            try:
                connection.execute("BEGIN IMMEDIATE")
                row = connection.execute("PRAGMA user_version").fetchone()
                current_version = int(row[0]) if row is not None else 0
                if current_version not in (0, SCHEMA_VERSION):
                    raise MemoryRepositoryError(
                        "unsupported memory database schema version"
                    )

                connection.execute(_CREATE_TABLE_SQL)
                connection.execute(_CREATE_SCOPE_INDEX_SQL)
                if current_version == 0:
                    connection.execute(f"PRAGMA user_version = {SCHEMA_VERSION}")
                connection.commit()
            except MemoryRepositoryError:
                self._rollback_quietly(connection)
                self._close_after_failed_initialization(connection)
                raise
            except (sqlite3.Error, OSError):
                self._rollback_quietly(connection)
                self._close_after_failed_initialization(connection)
                raise MemoryRepositoryError(
                    "memory repository initialization failed"
                ) from None

    def load_recent(
        self,
        *,
        scope_id: str,
        npc_id: str,
        limit: int,
    ) -> tuple[StoredDialogueTurn, ...]:
        """Load one isolated history window in stable chronological order."""
        if isinstance(limit, bool) or not isinstance(limit, int) or limit <= 0:
            raise ValueError("limit must be a positive integer")

        with self._lock:
            connection = self._require_connection()
            try:
                rows = connection.execute(
                    _LOAD_RECENT_SQL,
                    (scope_id, npc_id, limit),
                ).fetchall()
            except sqlite3.Error:
                raise MemoryRepositoryError(
                    "memory repository read failed"
                ) from None

        return tuple(self._map_turn(row) for row in rows)

    def store_turn(self, turn: DialogueTurnToStore) -> MemoryWriteResult:
        """Atomically store a complete turn, ignoring a repeated request ID."""
        with self._lock:
            connection = self._require_connection()
            try:
                connection.execute("BEGIN IMMEDIATE")
                cursor = connection.execute(
                    _INSERT_TURN_SQL,
                    (
                        turn.request_id,
                        turn.scope_id,
                        turn.npc_id,
                        turn.player_input,
                        turn.npc_reply,
                    ),
                )
                connection.commit()
            except sqlite3.Error:
                self._rollback_quietly(connection)
                raise MemoryRepositoryError(
                    "memory repository write failed"
                ) from None

        if cursor.rowcount == 0:
            return MemoryWriteResult.DUPLICATE_REQUEST
        return MemoryWriteResult.INSERTED

    def get_statistics(self) -> MemoryStatistics:
        """Return aggregate counts without exposing persisted identifiers."""
        with self._lock:
            connection = self._require_connection()
            try:
                row = connection.execute(_SELECT_STATISTICS_SQL).fetchone()
            except sqlite3.Error:
                raise MemoryRepositoryError(
                    "memory repository statistics failed"
                ) from None

        if row is None:
            raise MemoryRepositoryError("memory repository statistics failed")
        return MemoryStatistics(
            turn_count=int(row["turn_count"]),
            scope_count=int(row["scope_count"]),
            scope_npc_pair_count=int(row["scope_npc_pair_count"]),
        )

    def delete_scope_npc(self, *, scope_id: str, npc_id: str) -> int:
        """Atomically delete only the exact requested scope/NPC partition."""
        with self._lock:
            connection = self._require_connection()
            try:
                connection.execute("BEGIN IMMEDIATE")
                cursor = connection.execute(
                    _DELETE_SCOPE_NPC_SQL,
                    (scope_id, npc_id),
                )
                connection.commit()
            except sqlite3.Error:
                self._rollback_quietly(connection)
                raise MemoryRepositoryError(
                    "memory repository delete failed"
                ) from None
        return cursor.rowcount

    def close(self) -> None:
        """Close the active connection; repeated close calls are harmless."""
        with self._lock:
            connection = self._connection
            self._connection = None
            if connection is None:
                return
            try:
                connection.close()
            except sqlite3.Error:
                raise MemoryRepositoryError(
                    "memory repository close failed"
                ) from None

    def _open_connection(self) -> sqlite3.Connection:
        connection: sqlite3.Connection | None = None
        try:
            self._database_path.parent.mkdir(parents=True, exist_ok=True)
            connection = sqlite3.connect(
                self._database_path,
                isolation_level=None,
                check_same_thread=False,
            )
            connection.row_factory = sqlite3.Row
            connection.execute("PRAGMA foreign_keys = ON")
            connection.execute("PRAGMA busy_timeout = 5000")
            connection.execute("PRAGMA journal_mode = WAL")
            connection.execute("PRAGMA synchronous = NORMAL")
            return connection
        except (sqlite3.Error, OSError):
            if connection is not None:
                try:
                    connection.close()
                except sqlite3.Error:
                    pass
            raise MemoryRepositoryError(
                "memory repository initialization failed"
            ) from None

    def _require_connection(self) -> sqlite3.Connection:
        if self._connection is None:
            raise MemoryRepositoryError("memory repository is not initialized")
        return self._connection

    def _close_after_failed_initialization(
        self,
        connection: sqlite3.Connection,
    ) -> None:
        self._connection = None
        try:
            connection.close()
        except sqlite3.Error:
            pass

    @staticmethod
    def _rollback_quietly(connection: sqlite3.Connection) -> None:
        try:
            connection.rollback()
        except sqlite3.Error:
            pass

    @staticmethod
    def _map_turn(row: sqlite3.Row) -> StoredDialogueTurn:
        return StoredDialogueTurn(
            sequence=int(row["sequence"]),
            request_id=str(row["request_id"]),
            scope_id=str(row["scope_id"]),
            npc_id=str(row["npc_id"]),
            player_input=str(row["player_input"]),
            npc_reply=str(row["npc_reply"]),
        )
