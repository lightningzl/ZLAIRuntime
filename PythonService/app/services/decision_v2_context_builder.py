"""Build supplier-neutral personal social-plan context."""

import json

from app.planners.decision_v2 import DecisionV2GenerationContext
from app.schemas.decision_v2 import DecisionV2Request


DECISION_V2_SYSTEM_INSTRUCTIONS = """Return exactly one JSON object with objective, public_reason, attention_target_id, expression, steps, speech, confidence. Use only supplied personal data. The objective is an open high-level goal, not a completed action. steps is an array of at most four objects with capability_id and optional target_id; use only capability IDs and targets supplied by UE. Never claim a step, report, trade, damage, or animation has already happened. Choose a continuous strategy based on social_situation, role, personality, goals, relationship and state. Speech must be Simplified Chinese when present. Do not output reasoning, markdown, scripts, extra fields, or invented facts."""


def build_decision_v2_generation_context(request: DecisionV2Request) -> DecisionV2GenerationContext:
    data = {
        "npc_id": request.npc_id,
        "state_version": request.state_version,
        "trigger": request.trigger.model_dump(mode="json", exclude_none=True),
        "context": request.context.model_dump(mode="json", exclude_none=True),
        "available_capabilities": [item.model_dump(mode="json") for item in request.available_capabilities],
    }
    return DecisionV2GenerationContext(
        system_instructions=DECISION_V2_SYSTEM_INSTRUCTIONS,
        context_data_json=json.dumps(data, ensure_ascii=False, separators=(",", ":")),
    )
