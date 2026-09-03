"""Decision v2 orchestration and response validation."""

import math
from uuid import uuid4

from pydantic import ValidationError
from app.planners.base import DecisionPlannerInvalidResponse
from app.planners.decision_v2 import DecisionV2Planner, DecisionV2PlannerResult
from app.schemas.decision_v2 import DecisionPlan, DecisionV2Request, DecisionV2Response, DecisionV2Speech, PlanExpression, PlanStep
from app.services.decision_v2_context_builder import build_decision_v2_generation_context


class DecisionV2Service:
    def __init__(self, planner: DecisionV2Planner) -> None:
        self._planner = planner

    def build_response(self, request: DecisionV2Request) -> DecisionV2Response:
        result = self._planner.plan(build_decision_v2_generation_context(request))
        self._validate(request, result)
        try:
            expression = PlanExpression.model_validate(result.expression) if result.expression else None
        except ValidationError:
            # Presentation is advisory. A malformed expression must not make a
            # otherwise valid social plan claim a world action.
            expression = None
        return DecisionV2Response(
            request_id=request.request_id, npc_id=request.npc_id, state_version=request.state_version,
            decision_id=str(uuid4()),
            plan=DecisionPlan(objective=result.objective, public_reason=result.public_reason,
                attention_target_id=result.attention_target_id, expression=expression,
                steps=[PlanStep(step_id=str(uuid4()), capability_id=item.capability_id, target_id=item.target_id) for item in result.steps]),
            speech=DecisionV2Speech(text=result.speech_text, emotion=result.speech_emotion) if result.speech_text else None,
            confidence=result.confidence, provider=result.provider,
        )

    @staticmethod
    def _validate(request: DecisionV2Request, result: DecisionV2PlannerResult) -> None:
        if not result.objective.strip() or not math.isfinite(result.confidence) or not 0 <= result.confidence <= 1:
            raise DecisionPlannerInvalidResponse(result.provider, "Planner returned an invalid social plan")
        if len(result.steps) > 4:
            raise DecisionPlannerInvalidResponse(result.provider, "Planner exceeded plan step budget")
        allowed = {item.capability_id: item for item in request.available_capabilities}
        for step in result.steps:
            capability = allowed.get(step.capability_id)
            if capability is None or step.target_id not in capability.target_ids and step.target_id is not None:
                raise DecisionPlannerInvalidResponse(result.provider, "Planner used an unavailable capability or target")
