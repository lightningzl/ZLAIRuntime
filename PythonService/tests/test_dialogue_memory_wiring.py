"""Dialogue orchestration and application lifecycle tests for Memory."""

import logging
from pathlib import Path

from fastapi.testclient import TestClient
import pytest

from app.core.settings import Settings
from app.main import create_app
from app.memory.base import MemoryRepositoryError
from app.memory.models import (
    DialogueMemoryMessage,
    DialogueTurnToStore,
    MemoryWriteResult,
    MergedDialogueHistory,
    StoredDialogueTurn,
)
from app.providers.base import (
    DialogueGenerationContext,
    DialogueProviderResult,
)
from app.providers.errors import ProviderInvalidResponseError, ProviderUnavailableError
from app.schemas.dialogue import DialogueRequest
from app.services.dialogue_service import DialogueService


class CapturingProvider:
    def __init__(
        self,
        *,
        reply: str = "generated reply",
        error: Exception | None = None,
        events: list[str] | None = None,
    ) -> None:
        self.reply = reply
        self.error = error
        self.events = events
        self.contexts: list[DialogueGenerationContext] = []

    def generate(
        self,
        context: DialogueGenerationContext,
    ) -> DialogueProviderResult:
        if self.events is not None:
            self.events.append("provider")
        self.contexts.append(context)
        if self.error is not None:
            raise self.error
        return DialogueProviderResult(reply=self.reply, provider="stub")


class FakeMemoryService:
    def __init__(
        self,
        *,
        merged_messages: tuple[DialogueMemoryMessage, ...] = (),
        retrieved_turn_count: int = 0,
        write_result: MemoryWriteResult = MemoryWriteResult.INSERTED,
        read_error: MemoryRepositoryError | None = None,
        write_error: MemoryRepositoryError | None = None,
        events: list[str] | None = None,
    ) -> None:
        self.merged_messages = merged_messages
        self.retrieved_turn_count = retrieved_turn_count
        self.write_result = write_result
        self.read_error = read_error
        self.write_error = write_error
        self.events = events
        self.load_calls: list[
            tuple[str, str, tuple[DialogueMemoryMessage, ...]]
        ] = []
        self.store_calls: list[tuple[str, str, str, str, str]] = []

    def load_merged_history(
        self,
        *,
        scope_id: str,
        npc_id: str,
        client_history: tuple[DialogueMemoryMessage, ...],
    ) -> MergedDialogueHistory:
        if self.events is not None:
            self.events.append("read")
        self.load_calls.append((scope_id, npc_id, client_history))
        if self.read_error is not None:
            raise self.read_error
        return MergedDialogueHistory(
            messages=self.merged_messages,
            retrieved_turn_count=self.retrieved_turn_count,
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
        if self.events is not None:
            self.events.append("write")
        self.store_calls.append(
            (request_id, scope_id, npc_id, player_input, npc_reply)
        )
        if self.write_error is not None:
            raise self.write_error
        return self.write_result


def _request(
    *,
    request_id: str = "request-1",
    with_memory: bool = True,
    with_context: bool = True,
) -> DialogueRequest:
    payload: dict[str, object] = {
        "request_id": request_id,
        "npc_id": "npc-keeper",
        "player_input": "current question",
    }
    if with_memory:
        payload["memory"] = {"scope_id": "save-a"}
    if with_context:
        payload["context"] = {
            "npc": {
                "display_name": "Keeper",
                "role": "keeper",
                "personality": ["calm"],
                "speaking_style": "brief",
                "goals": [],
            },
            "world": {
                "location": "gate",
                "situation": "quiet",
                "facts": [],
            },
            "dialogue_history": [
                {"role": "player", "content": "client question"},
                {"role": "npc", "content": "client answer"},
            ],
        }
    return DialogueRequest.model_validate(payload)


def test_no_memory_request_never_calls_memory_service() -> None:
    provider = CapturingProvider()
    memory_service = FakeMemoryService(
        read_error=MemoryRepositoryError("must not read"),
        write_error=MemoryRepositoryError("must not write"),
    )
    service = DialogueService(provider, memory_service)

    response = service.build_response(_request(with_memory=False))

    assert response.reply == "generated reply"
    assert memory_service.load_calls == []
    assert memory_service.store_calls == []
    assert [message.content for message in provider.contexts[0].messages] == [
        "client question",
        "client answer",
        "current question",
    ]


def test_memory_flow_is_read_then_generate_then_successful_write() -> None:
    events: list[str] = []
    merged_messages = (
        DialogueMemoryMessage(role="player", content="persistent question"),
        DialogueMemoryMessage(role="npc", content="persistent answer"),
        DialogueMemoryMessage(role="player", content="client question"),
        DialogueMemoryMessage(role="npc", content="client answer"),
    )
    provider = CapturingProvider(events=events)
    memory_service = FakeMemoryService(
        merged_messages=merged_messages,
        retrieved_turn_count=1,
        events=events,
    )
    service = DialogueService(provider, memory_service)

    response = service.build_response(_request())

    assert response.reply == "generated reply"
    assert events == ["read", "provider", "write"]
    assert memory_service.load_calls == [
        (
            "save-a",
            "npc-keeper",
            (
                DialogueMemoryMessage(role="player", content="client question"),
                DialogueMemoryMessage(role="npc", content="client answer"),
            ),
        )
    ]
    assert [message.content for message in provider.contexts[0].messages] == [
        "persistent question",
        "persistent answer",
        "client question",
        "client answer",
        "current question",
    ]
    assert memory_service.store_calls == [
        (
            "request-1",
            "save-a",
            "npc-keeper",
            "current question",
            "generated reply",
        )
    ]


def test_memory_without_context_uses_empty_client_history() -> None:
    provider = CapturingProvider()
    memory_service = FakeMemoryService()

    DialogueService(provider, memory_service).build_response(
        _request(with_context=False)
    )

    assert memory_service.load_calls == [("save-a", "npc-keeper", ())]


def test_memory_read_failure_stops_before_provider_and_write() -> None:
    provider = CapturingProvider()
    read_error = MemoryRepositoryError("safe read failure")
    memory_service = FakeMemoryService(read_error=read_error)

    with pytest.raises(MemoryRepositoryError) as raised:
        DialogueService(provider, memory_service).build_response(_request())

    assert raised.value is read_error
    assert provider.contexts == []
    assert memory_service.store_calls == []


def test_provider_failure_is_called_once_and_never_writes() -> None:
    provider_error = ProviderUnavailableError("stub", "provider unavailable")
    provider = CapturingProvider(error=provider_error)
    memory_service = FakeMemoryService()

    with pytest.raises(ProviderUnavailableError) as raised:
        DialogueService(provider, memory_service).build_response(_request())

    assert raised.value is provider_error
    assert len(provider.contexts) == 1
    assert memory_service.store_calls == []


@pytest.mark.parametrize("invalid_reply", ["", " \t "])
def test_invalid_provider_reply_never_writes(invalid_reply: str) -> None:
    provider = CapturingProvider(reply=invalid_reply)
    memory_service = FakeMemoryService()

    with pytest.raises(ProviderInvalidResponseError, match="empty reply"):
        DialogueService(provider, memory_service).build_response(_request())

    assert len(provider.contexts) == 1
    assert memory_service.store_calls == []


def test_memory_write_failure_occurs_after_single_provider_call() -> None:
    provider = CapturingProvider()
    write_error = MemoryRepositoryError("safe write failure")
    memory_service = FakeMemoryService(write_error=write_error)

    with pytest.raises(MemoryRepositoryError) as raised:
        DialogueService(provider, memory_service).build_response(_request())

    assert raised.value is write_error
    assert len(provider.contexts) == 1
    assert len(memory_service.store_calls) == 1


def test_duplicate_write_result_still_returns_success() -> None:
    provider = CapturingProvider()
    memory_service = FakeMemoryService(
        write_result=MemoryWriteResult.DUPLICATE_REQUEST
    )

    response = DialogueService(provider, memory_service).build_response(_request())

    assert response.reply == "generated reply"
    assert len(memory_service.store_calls) == 1


def test_memory_success_logs_only_bounded_metadata(
    caplog: pytest.LogCaptureFixture,
) -> None:
    sensitive_markers = (
        "PRIVATE_SCOPE",
        "PRIVATE_PERSISTENT_CONTENT",
        "PRIVATE_CLIENT_CONTENT",
        "PRIVATE_REPLY",
    )
    payload = _request().model_dump(mode="json")
    payload["memory"]["scope_id"] = sensitive_markers[0]  # type: ignore[index]
    payload["context"]["dialogue_history"][0]["content"] = (  # type: ignore[index]
        sensitive_markers[2]
    )
    provider = CapturingProvider(reply=sensitive_markers[3])
    memory_service = FakeMemoryService(
        merged_messages=(
            DialogueMemoryMessage(
                role="player",
                content=sensitive_markers[1],
            ),
        ),
        retrieved_turn_count=2,
    )
    caplog.set_level(logging.INFO, logger="app.services.dialogue_service")

    with TestClient(
        create_app(provider=provider, memory_service=memory_service)
    ) as client:
        response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 200
    assert "request_id=request-1" in caplog.text
    assert "npc_id=npc-keeper" in caplog.text
    assert "has_memory=True" in caplog.text
    assert "retrieved_turn_count=2" in caplog.text
    assert "memory_write_result=inserted" in caplog.text
    for marker in sensitive_markers:
        assert marker not in caplog.text


def test_memory_database_error_maps_to_sanitized_500_and_logs_metadata(
    caplog: pytest.LogCaptureFixture,
) -> None:
    sensitive_marker = "PRIVATE_SCOPE_SQL_PATH_CONTENT"
    memory_service = FakeMemoryService(
        read_error=MemoryRepositoryError(sensitive_marker)
    )
    payload = _request().model_dump(mode="json")
    payload["memory"]["scope_id"] = sensitive_marker  # type: ignore[index]
    caplog.set_level(logging.ERROR)

    with TestClient(
        create_app(
            provider=CapturingProvider(),
            memory_service=memory_service,
        ),
        raise_server_exceptions=False,
    ) as client:
        response = client.post("/v1/dialogue", json=payload)

    assert response.status_code == 500
    assert response.json() == {
        "request_id": "request-1",
        "error": {"code": "internal_error", "message": "internal server error"},
    }
    assert sensitive_marker not in response.text
    assert sensitive_marker not in caplog.text
    assert "request_id=request-1" in caplog.text
    assert "npc_id=npc-keeper" in caplog.text
    assert "has_memory=True" in caplog.text
    assert "category=memory_error" in caplog.text


def test_memory_request_without_injected_service_fails_safely() -> None:
    with TestClient(
        create_app(provider=CapturingProvider()),
        raise_server_exceptions=False,
    ) as client:
        response = client.post(
            "/v1/dialogue",
            json=_request(with_context=False).model_dump(mode="json"),
        )

    assert response.status_code == 500
    assert response.json()["error"] == {
        "code": "internal_error",
        "message": "internal server error",
    }


def test_real_app_lifecycle_recovers_memory_after_restart(tmp_path: Path) -> None:
    settings = Settings(
        dialogue_provider="stub",
        memory_database_path=tmp_path / "memory.sqlite3",
        memory_max_turns=4,
    )
    first_provider = CapturingProvider(reply="first answer")
    first_payload = _request(request_id="request-first", with_context=False)
    with TestClient(create_app(settings=settings, provider=first_provider)) as client:
        first_response = client.post(
            "/v1/dialogue",
            json=first_payload.model_dump(mode="json"),
        )

    second_provider = CapturingProvider(reply="second answer")
    second_payload = _request(request_id="request-second", with_context=False)
    with TestClient(create_app(settings=settings, provider=second_provider)) as client:
        second_response = client.post(
            "/v1/dialogue",
            json=second_payload.model_dump(mode="json"),
        )

    assert first_response.status_code == 200
    assert second_response.status_code == 200
    assert [message.content for message in second_provider.contexts[0].messages] == [
        "current question",
        "first answer",
        "current question",
    ]


def test_application_owns_injected_repository_lifecycle() -> None:
    class LifecycleRepository:
        def __init__(self) -> None:
            self.initialized = False
            self.closed = False

        def initialize(self) -> None:
            self.initialized = True

        def load_recent(
            self,
            *,
            scope_id: str,
            npc_id: str,
            limit: int,
        ) -> tuple[StoredDialogueTurn, ...]:
            return ()

        def store_turn(self, turn: DialogueTurnToStore) -> MemoryWriteResult:
            return MemoryWriteResult.INSERTED

        def close(self) -> None:
            self.closed = True

    repository = LifecycleRepository()
    with TestClient(
        create_app(
            provider=CapturingProvider(),
            memory_repository=repository,
        )
    ) as client:
        assert repository.initialized is True
        assert repository.closed is False
        assert client.post(
                "/v1/dialogue",
                json=_request(with_memory=False).model_dump(
                    mode="json",
                    exclude_none=True,
                ),
        ).status_code == 200

    assert repository.closed is True


def test_memory_service_and_repository_cannot_both_be_injected() -> None:
    class UnusedRepository:
        def initialize(self) -> None:
            pass

        def load_recent(
            self,
            *,
            scope_id: str,
            npc_id: str,
            limit: int,
        ) -> tuple[StoredDialogueTurn, ...]:
            return ()

        def store_turn(self, turn: DialogueTurnToStore) -> MemoryWriteResult:
            return MemoryWriteResult.INSERTED

        def close(self) -> None:
            pass

    with pytest.raises(ValueError, match="either memory_service"):
        create_app(
            provider=CapturingProvider(),
            memory_service=FakeMemoryService(),
            memory_repository=UnusedRepository(),
        )
