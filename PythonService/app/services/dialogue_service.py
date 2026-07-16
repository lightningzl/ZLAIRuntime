"""Deterministic dialogue behavior independent of HTTP concerns."""

from app.schemas.dialogue import DialogueRequest, DialogueResponse


STUB_REPLY = "城门刚刚关闭，请稍后再来。"


class InvalidDialogueRequest(ValueError):
    """Raised when a well-formed request violates a business rule."""

    def __init__(self, request_id: str, message: str) -> None:
        super().__init__(message)
        self.request_id = request_id
        self.message = message


def build_dialogue_response(request: DialogueRequest) -> DialogueResponse:
    """Validate business rules and return a deterministic Stub response."""
    if request.player_input == "":
        raise InvalidDialogueRequest(
            request_id=request.request_id,
            message="player_input must not be empty",
        )

    return DialogueResponse(
        request_id=request.request_id,
        npc_id=request.npc_id,
        reply=STUB_REPLY,
    )
