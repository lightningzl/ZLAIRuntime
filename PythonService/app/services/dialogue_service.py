"""Dialogue orchestration independent of HTTP and concrete Provider concerns."""

import logging

from app.providers.base import DialogueProvider
from app.schemas.dialogue import DialogueRequest, DialogueResponse
from app.services.context_builder import build_dialogue_generation_context


LOGGER = logging.getLogger(__name__)


class InvalidDialogueRequest(ValueError):
    """Raised when a well-formed request violates a business rule."""

    def __init__(self, request_id: str, message: str) -> None:
        super().__init__(message)
        self.request_id = request_id
        self.message = message


class DialogueService:
    """Validate one dialogue turn and delegate generation to an injected Provider."""

    def __init__(self, provider: DialogueProvider) -> None:
        self._provider = provider

    def build_response(self, request: DialogueRequest) -> DialogueResponse:
        if request.player_input == "":
            raise InvalidDialogueRequest(
                request_id=request.request_id,
                message="player_input must not be empty",
            )
        self._validate_context(request)

        history_count = (
            len(request.context.dialogue_history) if request.context is not None else 0
        )
        LOGGER.info(
            "Dialogue generation started request_id=%s npc_id=%s has_context=%s "
            "history_count=%d",
            request.request_id,
            request.npc_id,
            request.context is not None,
            history_count,
        )
        result = self._provider.generate(build_dialogue_generation_context(request))
        LOGGER.info(
            "Dialogue generation completed request_id=%s npc_id=%s provider=%s "
            "has_context=%s history_count=%d",
            request.request_id,
            request.npc_id,
            result.provider,
            request.context is not None,
            history_count,
        )
        return DialogueResponse(
            request_id=request.request_id,
            npc_id=request.npc_id,
            reply=result.reply,
            provider=result.provider,
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
