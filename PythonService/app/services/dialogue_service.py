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
