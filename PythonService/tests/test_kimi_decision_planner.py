from types import SimpleNamespace
from typing import Any

import httpx
import openai
import pytest

from app.core.settings import Settings
from app.planners.base import DecisionGenerationContext, DecisionPlannerInvalidResponse
from app.planners.kimi_planner import KimiDecisionPlanner
from app.providers.errors import ProviderTimeoutError


class FakeCompletions:
    def __init__(self, content: object = None, error: Exception | None = None) -> None:
        self._content = content
        self._error = error
        self.calls: list[dict[str, Any]] = []

    def create(self, **kwargs: Any) -> object:
        self.calls.append(kwargs)
        if self._error is not None:
            raise self._error
        return SimpleNamespace(
            choices=[SimpleNamespace(message=SimpleNamespace(content=self._content))]
        )


class FakeClient:
    def __init__(self, content: object = None, error: Exception | None = None) -> None:
        self.chat = SimpleNamespace(completions=FakeCompletions(content, error))


def settings() -> Settings:
    return Settings.from_env(
        {
            "MOONSHOT_API_KEY": "test-placeholder",
            "ZL_KIMI_MODEL": "test-model",
            "ZL_KIMI_MAX_OUTPUT_TOKENS": "321",
        }
    )


def context() -> DecisionGenerationContext:
    return DecisionGenerationContext(
        system_instructions="Return one JSON decision.",
        context_data_json='{"npc_id":"npc_guard"}',
    )


def test_kimi_planner_maps_one_structured_json_result() -> None:
    client = FakeClient(
        '{"intent":"disengage","speech":{"text":"Stay back.",'
        '"emotion":"wary"},"tool_call":{"name":"move_away",'
        '"target_id":"player"},"confidence":0.82}'
    )
    planner = KimiDecisionPlanner(settings(), client=client)

    result = planner.plan(context())

    assert result.intent == "disengage"
    assert result.speech_text == "Stay back."
    assert result.tool_name == "move_away"
    assert result.tool_target_id == "player"
    assert result.provider == "kimi"
    call = client.chat.completions.calls[0]
    assert call["response_format"] == {"type": "json_object"}
    assert call["stream"] is False
    assert call["max_completion_tokens"] == 321
    assert "tools" not in call


@pytest.mark.parametrize(
    "content",
    [
        "not json",
        '{"intent":"hold","confidence":0.5}',
        '{"intent":"hold","tool_call":{"name":"stop",'
        '"target_id":"player"},"confidence":0.5}',
    ],
)
def test_kimi_planner_rejects_invalid_or_empty_structures(content: str) -> None:
    planner = KimiDecisionPlanner(settings(), client=FakeClient(content))

    with pytest.raises(DecisionPlannerInvalidResponse):
        planner.plan(context())


def test_kimi_planner_classifies_timeout_without_raw_detail() -> None:
    error = openai.APITimeoutError(
        httpx.Request("POST", "https://api.moonshot.cn/v1/chat/completions")
    )
    planner = KimiDecisionPlanner(settings(), client=FakeClient(error=error))

    with pytest.raises(ProviderTimeoutError) as captured:
        planner.plan(context())

    assert captured.value.provider == "kimi"
    assert "moonshot" not in str(captured.value).lower()
