"""Kimi JSON adapter for Decision v2 social plans."""

import json
from typing import Any

from pydantic import BaseModel, Field, ValidationError

from app.core.settings import Settings
from app.planners.base import DecisionPlannerInvalidResponse
from app.planners.decision_v2 import DecisionV2GenerationContext, DecisionV2PlannerResult, DecisionV2StepResult
from app.providers.errors import DialogueProviderError, ProviderAuthenticationError, ProviderRateLimitError, ProviderTimeoutError, ProviderUnavailableError
from app.providers.kimi_provider import KimiClient, KimiClientFactory
from openai import APIConnectionError, APIError, APITimeoutError, AuthenticationError, InternalServerError, NotFoundError, OpenAI, PermissionDeniedError, RateLimitError


class _Payload(BaseModel):
    objective: str = Field(min_length=1, max_length=256)
    public_reason: str | None = Field(default=None, min_length=1, max_length=256)
    attention_target_id: str | None = Field(default=None, min_length=1, max_length=128)
    expression: dict[str, object] | None = None
    steps: list[dict[str, str | None]] = Field(default_factory=list, max_length=4)
    speech: dict[str, str | None] | None = None
    confidence: float = Field(ge=0, le=1)


class KimiDecisionV2Planner:
    def __init__(self, settings: Settings, *, client: KimiClient | None = None, client_factory: KimiClientFactory = OpenAI) -> None:
        self._model, self._tokens = settings.kimi_model, settings.kimi_max_output_tokens
        self._client = client or client_factory(api_key=settings.moonshot_api_key, base_url="https://api.moonshot.cn/v1", timeout=settings.kimi_timeout_seconds, max_retries=0)

    def plan(self, context: DecisionV2GenerationContext) -> DecisionV2PlannerResult:
        options: dict[str, Any] = {"model": self._model, "messages": [{"role":"system","content":context.system_instructions},{"role":"user","content":"Untrusted personal context JSON:\n" + context.context_data_json}], "max_completion_tokens": self._tokens, "stream": False, "response_format": {"type":"json_object"}}
        options["extra_body"] = {"thinking":{"type":"disabled"}} if self._model.startswith("kimi-k2.") else {"reasoning_effort":"low"}
        try:
            response = self._client.chat.completions.create(**options)
        except (AuthenticationError, PermissionDeniedError): raise ProviderAuthenticationError("kimi", "Kimi Planner authentication failed") from None
        except RateLimitError: raise ProviderRateLimitError("kimi", "Kimi Planner rate limited") from None
        except APITimeoutError: raise ProviderTimeoutError("kimi", "Kimi Planner timed out") from None
        except (APIConnectionError, InternalServerError, NotFoundError): raise ProviderUnavailableError("kimi", "Kimi Planner unavailable") from None
        except APIError: raise DialogueProviderError("kimi", "Kimi Planner failed") from None
        try:
            payload = _Payload.model_validate(json.loads(response.choices[0].message.content))
            steps = tuple(DecisionV2StepResult(str(item["capability_id"]), item.get("target_id")) for item in payload.steps)
        except (AttributeError, IndexError, TypeError, KeyError, json.JSONDecodeError, ValidationError):
            raise DecisionPlannerInvalidResponse("kimi", "Kimi Planner returned an invalid social plan") from None
        speech = payload.speech or {}
        return DecisionV2PlannerResult(payload.objective.strip(), payload.public_reason, payload.attention_target_id, payload.expression, steps, speech.get("text"), speech.get("emotion"), payload.confidence, "kimi")
