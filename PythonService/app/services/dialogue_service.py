"""Dialogue orchestration independent of HTTP and concrete Provider concerns."""

import logging

from app.memory.base import MemoryRepositoryError
from app.memory.models import DialogueMemoryMessage, MemoryWriteResult
from app.providers.base import DialogueProvider
from app.providers.errors import ProviderInvalidResponseError
from app.schemas.dialogue import DialogueRequest, DialogueResponse
from app.services.context_builder import build_dialogue_generation_context
from app.services.memory_service import MemoryService


LOGGER = logging.getLogger(__name__)


class InvalidDialogueRequest(ValueError):
    """Raised when a well-formed request violates a business rule."""

    def __init__(self, request_id: str, message: str) -> None:
        super().__init__(message)
        self.request_id = request_id
        self.message = message


class DialogueService:
    """Validate one dialogue turn and delegate generation to an injected Provider."""

    def __init__(
        self,
        provider: DialogueProvider,
        memory_service: MemoryService | None = None,
    ) -> None:
        self._provider = provider
        self._memory_service = memory_service

    def build_response(self, request: DialogueRequest) -> DialogueResponse:
        if request.player_input == "":
            raise InvalidDialogueRequest(
                request_id=request.request_id,
                message="player_input must not be empty",
            )
        self._validate_context(request)
        self._validate_memory(request)

        client_history_count = (
            len(request.context.dialogue_history) if request.context is not None else 0
        )
        has_memory = request.memory is not None
        merged_history: tuple[DialogueMemoryMessage, ...] | None = None
        retrieved_turn_count = 0
        if request.memory is not None:
            if self._memory_service is None:
                raise RuntimeError("dialogue memory service is unavailable")
            client_history = self._client_history(request)
            try:
                loaded_history = self._memory_service.load_merged_history(
                    scope_id=request.memory.scope_id,
                    npc_id=request.npc_id,
                    client_history=client_history,
                )
            except MemoryRepositoryError:
                LOGGER.error(
                    "Dialogue memory failed request_id=%s npc_id=%s "
                    "has_memory=True stage=read category=memory_error",
                    request.request_id,
                    request.npc_id,
                )
                raise
            merged_history = loaded_history.messages
            retrieved_turn_count = loaded_history.retrieved_turn_count

        LOGGER.info(
            "Dialogue generation started request_id=%s npc_id=%s has_context=%s "
            "history_count=%d has_memory=%s retrieved_turn_count=%d",
            request.request_id,
            request.npc_id,
            request.context is not None,
            client_history_count,
            has_memory,
            retrieved_turn_count,
        )
        result = self._provider.generate(
            build_dialogue_generation_context(
                request,
                dialogue_history=merged_history,
            )
        )
        if not isinstance(result.reply, str) or not result.reply.strip():
            raise ProviderInvalidResponseError(
                result.provider,
                "dialogue provider returned an empty reply",
            )
        response = DialogueResponse(
            request_id=request.request_id,
            npc_id=request.npc_id,
            reply=result.reply,
            provider=result.provider,
        )

        write_result: MemoryWriteResult | None = None
        if request.memory is not None:
            assert self._memory_service is not None
            try:
                write_result = self._memory_service.store_completed_turn(
                    request_id=request.request_id,
                    scope_id=request.memory.scope_id,
                    npc_id=request.npc_id,
                    player_input=request.player_input,
                    npc_reply=response.reply,
                )
            except MemoryRepositoryError:
                LOGGER.error(
                    "Dialogue memory failed request_id=%s npc_id=%s "
                    "has_memory=True stage=write category=memory_error "
                    "retrieved_turn_count=%d",
                    request.request_id,
                    request.npc_id,
                    retrieved_turn_count,
                )
                raise

        LOGGER.info(
            "Dialogue generation completed request_id=%s npc_id=%s provider=%s "
            "has_context=%s history_count=%d has_memory=%s "
            "retrieved_turn_count=%d memory_write_result=%s",
            request.request_id,
            request.npc_id,
            result.provider,
            request.context is not None,
            client_history_count,
            has_memory,
            retrieved_turn_count,
            write_result.value if write_result is not None else "skipped",
        )
        return response

    @staticmethod
    def _client_history(
        request: DialogueRequest,
    ) -> tuple[DialogueMemoryMessage, ...]:
        if request.context is None:
            return ()
        return tuple(
            DialogueMemoryMessage(role=message.role, content=message.content)
            for message in request.context.dialogue_history
        )

    @staticmethod
    def _validate_context(request: DialogueRequest) -> None:
        context = request.context
        if context is None:
            return

        fields = [
            ("context.npc.display_name", context.npc.display_name),
            ("context.npc.role", context.npc.role),
            ("context.npc.speaking_style", context.npc.speaking_style),
            ("context.world.location", context.world.location),
            ("context.world.situation", context.world.situation),
        ]
        fields.extend(
            (f"context.npc.personality[{index}]", value)
            for index, value in enumerate(context.npc.personality)
        )
        fields.extend(
            (f"context.npc.goals[{index}]", value)
            for index, value in enumerate(context.npc.goals)
        )
        fields.extend(
            (f"context.world.facts[{index}]", value)
            for index, value in enumerate(context.world.facts)
        )
        fields.extend(
            (f"context.dialogue_history[{index}].content", message.content)
            for index, message in enumerate(context.dialogue_history)
        )

        for field_name, value in fields:
            if value.strip() == "":
                raise InvalidDialogueRequest(
                    request_id=request.request_id,
                    message=f"{field_name} must not be blank",
                )

    @staticmethod
    def _validate_memory(request: DialogueRequest) -> None:
        memory = request.memory
        if memory is None:
            return

        if memory.scope_id.strip() == "":
            raise InvalidDialogueRequest(
                request_id=request.request_id,
                message="memory.scope_id must not be blank",
            )
        if memory.scope_id != memory.scope_id.strip():
            raise InvalidDialogueRequest(
                request_id=request.request_id,
                message=(
                    "memory.scope_id must not contain leading or trailing whitespace"
                ),
            )
