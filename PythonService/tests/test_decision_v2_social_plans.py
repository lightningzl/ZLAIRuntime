from app.planners.stub_decision_v2_planner import StubDecisionV2Planner
from app.schemas.decision_v2 import DecisionV2Request
from app.services.decision_v2_context_builder import build_decision_v2_generation_context
from app.services.decision_v2_service import DecisionV2Service


def payload(*facts: dict) -> dict:
    return {"request_id":"req-1","npc_id":"npc_merchant","state_version":1,"ttl_ms":30000,
      "trigger":{"event_id":"hit-1","kind":"action_result","source_id":"player","target_id":"npc_merchant","channels":["direct","visual"],"summary":"玩家攻击商人","occurred_at_ms":1},
      "context":{"npc":{"display_name":"商人","role":"merchant","personality":["谨慎"],"speaking_style":"克制","goals":["保护货物"]},"relationship":{"trust":-0.5,"affinity":-0.2,"fear":0.7,"familiarity":0.2},"instant_state":{"fear":0.7,"anger":0.5,"curiosity":0,"alert":1},"recent_history":[],"social_situation":list(facts)},
      "available_capabilities":[{"capability_id":"keep_distance_from_player","kind":"move_away","target_ids":["player"]},{"capability_id":"refuse_trade","kind":"set_interaction_stance","target_ids":["player"]},{"capability_id":"seek_nearby_guard","kind":"move_toward","target_ids":["npc_guard"]}]}


def fact(kind: str, summary: str) -> dict:
    return {"kind":kind,"subject_id":"player","summary":summary,"occurred_at_ms":1,"salience":0.9}


def test_repeated_harm_changes_merchant_plan_and_uses_only_available_capabilities() -> None:
    request = DecisionV2Request.model_validate(payload(fact("received_harm", "玩家第一次攻击你"), fact("received_harm", "玩家再次攻击你")))
    response = DecisionV2Service(StubDecisionV2Planner()).build_response(request)
    assert "不会" in response.speech.text
    assert {step.capability_id for step in response.plan.steps} <= {item.capability_id for item in request.available_capabilities}


def test_context_preserves_personal_social_facts_without_world_expansion() -> None:
    request = DecisionV2Request.model_validate(payload(fact("apology_received", "玩家向你道歉")))
    context = build_decision_v2_generation_context(request)
    assert "apology_received" in context.context_data_json
    assert "other_npc_private" not in context.context_data_json


def test_civilian_harm_plan_prefers_self_protection() -> None:
    data = payload(fact("received_harm", "玩家攻击你"))
    data["npc_id"] = "npc_civilian"
    data["context"]["npc"]["role"] = "civilian"
    response = DecisionV2Service(StubDecisionV2Planner()).build_response(DecisionV2Request.model_validate(data))
    assert any(step.capability_id == "keep_distance_from_player" for step in response.plan.steps)


def test_first_harm_and_apology_are_not_collapsed_into_the_repeat_case() -> None:
    first = DecisionV2Service(StubDecisionV2Planner()).build_response(
        DecisionV2Request.model_validate(payload(fact("received_harm", "玩家第一次攻击你")))
    )
    apology = DecisionV2Service(StubDecisionV2Planner()).build_response(
        DecisionV2Request.model_validate(payload(fact("apology_received", "玩家直接向你道歉")))
    )
    repeated = DecisionV2Service(StubDecisionV2Planner()).build_response(
        DecisionV2Request.model_validate(payload(fact("received_harm", "第一次"), fact("received_harm", "第二次")))
    )
    assert first.plan.objective != apology.plan.objective
    assert apology.speech is not None and "道歉" in apology.speech.text
    assert repeated.speech is not None and "不会" in repeated.speech.text


def test_witnessed_violence_does_not_become_personal_harm() -> None:
    data = payload(fact("witnessed_violence", "你看见玩家攻击了另一个人"))
    data["npc_id"] = "npc_civilian"
    data["context"]["npc"]["role"] = "civilian"
    response = DecisionV2Service(StubDecisionV2Planner()).build_response(DecisionV2Request.model_validate(data))
    assert response.speech is not None
    assert "有人能帮帮我" not in response.speech.text
