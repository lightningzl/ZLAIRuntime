#if WITH_DEV_AUTOMATION_TESTS

#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ZLAIServiceProtocol.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLDialogueRequestSerializationTest,
	"ZLAIRuntime.Protocol.SerializeDialogueRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLDialogueRequestSerializationTest::RunTest(const FString& Parameters)
{
	FZLDialogueRequest Request;
	Request.RequestId = TEXT("request-001");
	Request.NpcId = TEXT("npc_guard_01");
	Request.PlayerInput = TEXT("hello");

	FString Json;
	TestTrue(TEXT("Request serializes"), ZLAIServiceProtocol::SerializeDialogueRequest(Request, Json));

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!TestTrue(TEXT("Serialized request is valid JSON"), FJsonSerializer::Deserialize(Reader, RootObject)))
	{
		return false;
	}

	TestEqual(TEXT("Request ID uses protocol field"), RootObject->GetStringField(TEXT("request_id")), Request.RequestId);
	TestEqual(TEXT("NPC ID uses protocol field"), RootObject->GetStringField(TEXT("npc_id")), Request.NpcId);
	TestEqual(TEXT("Player input uses protocol field"), RootObject->GetStringField(TEXT("player_input")), Request.PlayerInput);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLDialogueResponseParsingTest,
	"ZLAIRuntime.Protocol.ParseDialogueResponse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLDialogueResponseParsingTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(
		"{\"request_id\":\"request-001\",\"npc_id\":\"npc_guard_01\","
		"\"reply\":\"hello\",\"provider\":\"stub\",\"future_field\":42}");

	FZLDialogueResponse Response;
	TestTrue(TEXT("Response parses with an unknown field"), ZLAIServiceProtocol::TryParseDialogueResponse(Json, Response));
	TestEqual(TEXT("Request ID parsed"), Response.RequestId, FString(TEXT("request-001")));
	TestEqual(TEXT("NPC ID parsed"), Response.NpcId, FString(TEXT("npc_guard_01")));
	TestEqual(TEXT("Reply parsed"), Response.Reply, FString(TEXT("hello")));
	TestEqual(TEXT("Provider parsed"), Response.Provider, FString(TEXT("stub")));

	const FString MissingReplyJson = TEXT(
		"{\"request_id\":\"request-001\",\"npc_id\":\"npc_guard_01\",\"provider\":\"stub\"}");
	TestFalse(
		TEXT("Response missing a required field fails"),
		ZLAIServiceProtocol::TryParseDialogueResponse(MissingReplyJson, Response));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLServiceErrorParsingTest,
	"ZLAIRuntime.Protocol.ParseServiceError",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLServiceErrorParsingTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(
		"{\"request_id\":\"request-001\",\"error\":{"
		"\"code\":\"invalid_request\",\"message\":\"invalid input\"}}");

	FZLServiceError Error;
	TestTrue(TEXT("Service error parses"), ZLAIServiceProtocol::TryParseServiceError(Json, Error));
	TestEqual(TEXT("Error request ID parsed"), Error.RequestId, FString(TEXT("request-001")));
	TestEqual(TEXT("Error code parsed"), Error.Code, FString(TEXT("invalid_request")));
	TestEqual(TEXT("Error message parsed"), Error.Message, FString(TEXT("invalid input")));
	return true;
}

#endif
