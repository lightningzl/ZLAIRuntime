"""Structured Decision orchestration independent of HTTP and concrete Planner."""

import logging
import math
from uuid import uuid4

from app.planners.base import (
    DecisionPlanner,
    DecisionPlannerInvalidResponse,
    DecisionPlannerResult,
)
from app.schemas.decision import (
    DecisionRequest,
    DecisionResponse,
    DecisionSpeech,
    DecisionToolCall,
)
from app.services.decision_context_builder import build_decision_generation_context


LOGGER = logging.getLogger(__name__)


class InvalidDecisionRequest(ValueError):
    """Raised when a well-formed Decision request violates a business rule."""

    def __init__(self, request_id: str, message: str) -> None:
        super().__init__(message)
        self.request_id = request_id
        self.message = message


class DecisionService:
    """Validate personal input and delegate one suggestion to a Planner."""

    def __init__(self, planner: DecisionPlanner) -> None:
        self._planner = planner

    def build_response(self, request: DecisionRequest) -> DecisionResponse:
        self._validate_non_blank_data(request)
        LOGGER.info(
            "Decision generation started request_id=%s npc_id=%s "
            "state_version=%d trigger_kind=%s history_count=%d tool_count=%d",
            request.request_id,
            request.npc_id,
            request.state_version,
            request.trigger.kind,
            len(request.context.recent_history),
            len(request.allowed_tools),
        )
        result = self._planner.plan(build_decision_generation_context(request))
        self._validate_planner_result(request, result)

        speech = (
            DecisionSpeech(text=result.speech_text, emotion=result.speech_emotion)
            if result.speech_text is not None
            else None
        )
        tool_call = (
            DecisionToolCall(
                call_id=str(uuid4()),
                name=result.tool_name,
                target_id=result.tool_target_id,
            )
            if result.tool_name is not None
            else None
        )
        response = DecisionResponse(
            request_id=request.request_id,
            npc_id=request.npc_id,
            state_version=request.state_version,
            decision_id=str(uuid4()),
            intent=result.intent,
            speech=speech,
            tool_call=tool_call,
            confidence=result.confidence,
            provider=result.provider,
        )
        LOGGER.info(
            "Decision generation completed request_id=%s npc_id=%s provider=%s "
            "state_version=%d has_speech=%s has_tool=%s intent=%s",
            request.request_id,
            request.npc_id,
            result.provider,
            request.state_version,
            speech is not None,
            tool_call is not None,
            result.intent,
        )
        return response

    @staticmethod
    def _validate_non_blank_data(request: DecisionRequest) -> None:
        fields: list[tuple[str, str]] = [
            ("request_id", request.request_id),
            ("npc_id", request.npc_id),
            ("trigger.event_id", request.trigger.event_id),
            ("trigger.source_id", request.trigger.source_id),
            ("trigger.summary", request.trigger.summary),
            ("context.npc.display_name", request.context.npc.display_name),
            ("context.npc.role", request.context.npc.role),
            ("context.npc.speaking_style", request.context.npc.speaking_style),
        ]
        if request.trigger.target_id is not None:
            fields.append(("trigger.target_id", request.trigger.target_id))
        if request.trigger.content is not None:
            fields.append(("trigger.content", request.trigger.content))
        fields.extend(
            (f"context.npc.personality[{index}]", value)
            for index, value in enumerate(request.context.npc.personality)
        )
        fields.extend(
            (f"context.npc.goals[{index}]", value)
            for index, value in enumerate(request.context.npc.goals)
        )
        for index, item in enumerate(request.context.recent_history):
            fields.extend(
                [
                    (f"context.recent_history[{index}].source_id", item.source_id),
                    (f"context.recent_history[{index}].summary", item.summary),
                ]
            )
            if item.target_id is not None:
                fields.append(
                    (f"context.recent_history[{index}].target_id", item.target_id)
                )
        for tool_index, tool in enumerate(request.allowed_tools):
            fields.extend(
                (f"allowed_tools[{tool_index}].target_ids[{target_index}]", target)
                for target_index, target in enumerate(tool.target_ids)
            )
        for field_name, value in fields:
            if not value.strip():
                raise InvalidDecisionRequest(
                    request.request_id,
                    f"{field_name} must not be blank",
                )

    @staticmethod
    def _validate_planner_result(
        request: DecisionRequest,
        result: DecisionPlannerResult,
    ) -> None:
        provider = result.provider
        if result.speech_text is not None and not result.speech_text.strip():
            raise DecisionPlannerInvalidResponse(
                provider, "Planner returned blank speech"
            )
        if result.speech_emotion is not None and not result.speech_emotion.strip():
            raise DecisionPlannerInvalidResponse(
                provider, "Planner returned blank emotion"
            )
        if result.speech_text is None and result.tool_name is None:
            raise DecisionPlannerInvalidResponse(
                provider, "Planner returned an empty decision"
            )
        if not math.isfinite(result.confidence) or not 0.0 <= result.confidence <= 1.0:
            raise DecisionPlannerInvalidResponse(
                provider, "Planner returned invalid confidence"
            )
        tool_name = result.tool_name
        target_id = result.tool_target_id
        if tool_name is None:
            return
        allowed = next(
            (tool for tool in request.allowed_tools if tool.name == tool_name),
            None,
        )
        if allowed is None:
            raise DecisionPlannerInvalidResponse(
                provider, "Planner suggested a Tool that UE did not allow"
            )
        if tool_name == "stop":
            if target_id is not None:
                raise DecisionPlannerInvalidResponse(
                    provider, "Planner suggested an invalid stop target"
                )
            return
        if target_id not in allowed.target_ids:
            raise DecisionPlannerInvalidResponse(
                provider, "Planner suggested a target that UE did not allow"
            )
