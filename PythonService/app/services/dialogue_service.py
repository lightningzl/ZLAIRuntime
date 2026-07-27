"""Dialogue orchestration independent of HTTP and concrete Provider concerns."""

from app.providers.base import DialogueProvider, DialogueProviderRequest
from app.schemas.dialogue import DialogueRequest, DialogueResponse


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

        result = self._provider.generate(
            DialogueProviderRequest(
                npc_id=request.npc_id,
                player_input=request.player_input,
            )
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
