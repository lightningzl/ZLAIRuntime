#if WITH_DEV_AUTOMATION_TESTS

#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ZLAIServiceProtocol.h"

namespace
{
	FZLDecisionRequest MakeValidDecisionRequest()
	{
		FZLDecisionRequest Request;
		Request.RequestId = TEXT("request-1");
		Request.NpcId = TEXT("npc_guard");
		Request.StateVersion = 12;
		Request.TtlMs = 30000;
		Request.Trigger.EventId = TEXT("speech-42");
		Request.Trigger.Kind = TEXT("speech");
		Request.Trigger.SourceId = TEXT("player");
		Request.Trigger.TargetId = TEXT("npc_guard");
		Request.Trigger.Channels = {TEXT("auditory"), TEXT("direct")};
		Request.Trigger.Content = TEXT("Please keep your distance.");
		Request.Trigger.Summary = TEXT("The player spoke to the guard.");
		Request.Trigger.OccurredAtMs = 1725100800000;
		Request.Context.Npc.DisplayName = TEXT("Guard");
		Request.Context.Npc.Role = TEXT("gate guard");
		Request.Context.Npc.Personality = {TEXT("cautious")};
		Request.Context.Npc.SpeakingStyle = TEXT("brief");
		Request.Context.Npc.Goals = {TEXT("keep order")};
		Request.Context.Relationship.Trust = -0.1f;
		Request.Context.Relationship.Fear = 0.2f;
		Request.Context.Relationship.Familiarity = 0.4f;
		Request.Context.InstantState.Fear = 0.2f;
		Request.Context.InstantState.Anger = 0.3f;
		Request.Context.InstantState.Curiosity = 0.1f;
		Request.Context.InstantState.Alert = 0.8f;
		FZLDecisionHistoryItem History;
		History.Kind = TEXT("action_result");
		History.SourceId = TEXT("player");
		History.TargetId = TEXT("npc_guard");
		History.Summary = TEXT("The player stopped approaching.");
		History.OccurredAtMs = 1725100795000;
		Request.Context.RecentHistory.Add(History);
		FZLDecisionAllowedTool MoveAway;
		MoveAway.Name = TEXT("move_away");
		MoveAway.TargetIds = {TEXT("player")};
		Request.AllowedTools.Add(MoveAway);
		FZLDecisionAllowedTool Stop;
		Stop.Name = TEXT("stop");
		Request.AllowedTools.Add(Stop);
		return Request;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLDecisionRequestSerializationTest,
	"ZLAIRuntime.Protocol.SerializeDecisionRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLDecisionRequestSerializationTest::RunTest(const FString& Parameters)
{
	const FZLDecisionRequest Request = MakeValidDecisionRequest();
	FString Json;
	TestTrue(TEXT("Confirmed Decision request serializes"), ZLAIServiceProtocol::SerializeDecisionRequest(Request, Json));

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!TestTrue(TEXT("Serialized Decision is valid JSON"), FJsonSerializer::Deserialize(Reader, Root)))
	{
		return false;
	}
	TestEqual(TEXT("Decision request ID"), Root->GetStringField(TEXT("request_id")), Request.RequestId);
	TestEqual(TEXT("Decision NPC ID"), Root->GetStringField(TEXT("npc_id")), Request.NpcId);
	TestEqual(TEXT("Decision state version"), static_cast<int64>(Root->GetNumberField(TEXT("state_version"))), Request.StateVersion);
	TestEqual(TEXT("Allowed Tool count"), Root->GetArrayField(TEXT("allowed_tools")).Num(), 2);
	const TSharedPtr<FJsonObject> Trigger = Root->GetObjectField(TEXT("trigger"));
	TestEqual(TEXT("Speech content is explicit"), Trigger->GetStringField(TEXT("content")), Request.Trigger.Content);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLDecisionRequestValidationTest,
	"ZLAIRuntime.Protocol.ValidateDecisionBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLDecisionRequestValidationTest::RunTest(const FString& Parameters)
{
	FString Error;
	FZLDecisionRequest Request = MakeValidDecisionRequest();
	TestTrue(TEXT("Valid Decision request passes"), ZLAIServiceProtocol::ValidateDecisionRequest(Request, Error));

	Request.TtlMs = 99;
	TestFalse(TEXT("TTL below contract fails"), ZLAIServiceProtocol::ValidateDecisionRequest(Request, Error));
	Request = MakeValidDecisionRequest();
	Request.Trigger.Channels.Add(TEXT("auditory"));
	TestFalse(TEXT("Duplicate channel fails"), ZLAIServiceProtocol::ValidateDecisionRequest(Request, Error));
	Request = MakeValidDecisionRequest();
	Request.AllowedTools[1].TargetIds.Add(TEXT("player"));
	TestFalse(TEXT("Stop target fails"), ZLAIServiceProtocol::ValidateDecisionRequest(Request, Error));
	Request = MakeValidDecisionRequest();
	Request.Trigger.Kind = TEXT("action_result");
	TestFalse(TEXT("Action result cannot carry player input"), ZLAIServiceProtocol::ValidateDecisionRequest(Request, Error));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLDecisionResponseParsingTest,
	"ZLAIRuntime.Protocol.ParseDecisionResponse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLDecisionResponseParsingTest::RunTest(const FString& Parameters)
{
	const FString ValidJson = TEXT(R"({
		"request_id":"request-1","npc_id":"npc_guard","state_version":12,
		"decision_id":"decision-1","intent":"disengage",
		"speech":{"text":"Keep your distance.","emotion":"wary"},
		"tool_call":{"call_id":"tool-1","name":"move_away","target_id":"player"},
		"confidence":0.82,"provider":"stub"})");
	FZLDecisionResponse Response;
	TestTrue(TEXT("Confirmed Decision response parses"), ZLAIServiceProtocol::TryParseDecisionResponse(ValidJson, Response));
	TestTrue(TEXT("Speech is present"), Response.bHasSpeech);
	TestEqual(TEXT("Speech text"), Response.Speech.Text, FString(TEXT("Keep your distance.")));
	TestTrue(TEXT("Tool is present"), Response.bHasToolCall);
	TestEqual(TEXT("Tool name"), Response.ToolCall.Name, FString(TEXT("move_away")));

	const FString EmptyJson = TEXT(R"({"request_id":"request-1","npc_id":"npc_guard","state_version":12,"decision_id":"decision-1","intent":"hold","confidence":0.5,"provider":"stub"})");
	TestFalse(TEXT("Empty Decision result fails"), ZLAIServiceProtocol::TryParseDecisionResponse(EmptyJson, Response));
	const FString InvalidStop = TEXT(R"({"request_id":"request-1","npc_id":"npc_guard","state_version":12,"decision_id":"decision-1","intent":"hold","tool_call":{"call_id":"tool-1","name":"stop","target_id":"player"},"confidence":0.5,"provider":"stub"})");
	TestFalse(TEXT("Stop with target fails"), ZLAIServiceProtocol::TryParseDecisionResponse(InvalidStop, Response));
	return true;
}

#endif
