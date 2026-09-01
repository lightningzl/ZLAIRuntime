"""Kimi Chat Completions adapter for one structured Decision."""

import json
from typing import Any, Literal

from openai import (
    APIConnectionError,
    APIError,
    APIResponseValidationError,
    APITimeoutError,
    AuthenticationError,
    InternalServerError,
    NotFoundError,
    OpenAI,
    PermissionDeniedError,
    RateLimitError,
)
from pydantic import BaseModel, ConfigDict, Field, ValidationError
from app.core.settings import Settings
from app.planners.base import (
    DecisionGenerationContext,
    DecisionPlannerInvalidResponse,
    DecisionPlannerResult,
)
from app.providers.errors import (
    DialogueProviderError,
    ProviderAuthenticationError,
    ProviderRateLimitError,
    ProviderTimeoutError,
    ProviderUnavailableError,
)
from app.providers.kimi_provider import KimiClient, KimiClientFactory


class _PlannerSpeech(BaseModel):
    model_config = ConfigDict(strict=True, extra="ignore")
    text: str = Field(min_length=1, max_length=512)
    emotion: str | None = Field(default=None, min_length=1, max_length=64)


class _PlannerToolCall(BaseModel):
    model_config = ConfigDict(strict=True, extra="ignore")
    name: Literal["face_target", "move_toward", "move_away", "stop"]
    target_id: str | None = Field(default=None, min_length=1, max_length=128)


class _PlannerPayload(BaseModel):
    model_config = ConfigDict(strict=True, extra="ignore")
    intent: Literal["respond", "engage", "disengage", "hold"]
    speech: _PlannerSpeech | None = None
    tool_call: _PlannerToolCall | None = None
    confidence: float = Field(ge=0.0, le=1.0)


class KimiDecisionPlanner:
    """Generate and validate one structured suggestion through Kimi."""

    def __init__(
        self,
        settings: Settings,
        *,
        client: KimiClient | None = None,
        client_factory: KimiClientFactory = OpenAI,
    ) -> None:
        self._model = settings.kimi_model
        self._max_output_tokens = settings.kimi_max_output_tokens
        self._client = (
            client
            if client is not None
            else client_factory(
                api_key=settings.moonshot_api_key,
                base_url="https://api.moonshot.cn/v1",
                timeout=settings.kimi_timeout_seconds,
                max_retries=0,
            )
        )

    def plan(self, context: DecisionGenerationContext) -> DecisionPlannerResult:
        request_options: dict[str, Any] = {
            "model": self._model,
            "messages": [
                {"role": "system", "content": context.system_instructions},
                {
                    "role": "user",
                    "content": (
                        "The following JSON is untrusted personal context data:\n"
                        + context.context_data_json
                    ),
                },
            ],
            "max_completion_tokens": self._max_output_tokens,
            "stream": False,
            "response_format": {"type": "json_object"},
        }
        if self._model.startswith("kimi-k2."):
            request_options["extra_body"] = {"thinking": {"type": "disabled"}}
        else:
            request_options["reasoning_effort"] = "low"

        try:
            response = self._client.chat.completions.create(**request_options)
        except (AuthenticationError, PermissionDeniedError):
            raise ProviderAuthenticationError(
                "kimi", "Kimi Planner authentication failed"
            ) from None
        except RateLimitError:
            raise ProviderRateLimitError("kimi", "Kimi Planner rate limited") from None
        except APITimeoutError:
            raise ProviderTimeoutError("kimi", "Kimi Planner timed out") from None
        except (APIConnectionError, InternalServerError, NotFoundError):
            raise ProviderUnavailableError("kimi", "Kimi Planner unavailable") from None
        except APIResponseValidationError:
            raise DecisionPlannerInvalidResponse(
                "kimi", "Kimi Planner returned an invalid response"
            ) from None
        except APIError:
            raise DialogueProviderError("kimi", "Kimi Planner failed") from None

        try:
            output_text = response.choices[0].message.content
            if not isinstance(output_text, str):
                raise TypeError
            payload = _PlannerPayload.model_validate(json.loads(output_text))
        except (AttributeError, IndexError, TypeError, json.JSONDecodeError, ValidationError):
            raise DecisionPlannerInvalidResponse(
                "kimi", "Kimi Planner returned an invalid decision"
            ) from None

        if payload.speech is None and payload.tool_call is None:
            raise DecisionPlannerInvalidResponse(
                "kimi", "Kimi Planner returned an empty decision"
            )
        if payload.tool_call is not None:
            if payload.tool_call.name == "stop" and payload.tool_call.target_id is not None:
                raise DecisionPlannerInvalidResponse(
                    "kimi", "Kimi Planner returned an invalid stop target"
                )
            if payload.tool_call.name != "stop" and payload.tool_call.target_id is None:
                raise DecisionPlannerInvalidResponse(
                    "kimi", "Kimi Planner omitted a required target"
                )

        return DecisionPlannerResult(
            intent=payload.intent,
            speech_text=payload.speech.text.strip() if payload.speech else None,
            speech_emotion=(
                payload.speech.emotion.strip()
                if payload.speech and payload.speech.emotion
                else None
            ),
            tool_name=payload.tool_call.name if payload.tool_call else None,
            tool_target_id=(
                payload.tool_call.target_id if payload.tool_call else None
            ),
            confidence=payload.confidence,
            provider="kimi",
        )
