"""Deterministic offline dialogue Provider."""

from app.providers.base import DialogueGenerationContext, DialogueProviderResult


STUB_REPLY = "城门刚刚关闭，请稍后再来。"


class StubDialogueProvider:
    """Return a deterministic reply without credentials or external I/O."""

    def generate(self, _context: DialogueGenerationContext) -> DialogueProviderResult:
        return DialogueProviderResult(reply=STUB_REPLY, provider="stub")
