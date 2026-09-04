#include "Misc/AutomationTest.h"
#include "ZLSocialPersona.h"
#include "ZLSocialPersonaJson.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialPersonaValidationTest, "ZL.Social.Persona.Validation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialPersonaValidationTest::RunTest(const FString&)
{
	FZLSocialPersonaData Persona;
	Persona.StableId = TEXT("npc_merchant");
	Persona.DisplayName = TEXT("Merchant");
	Persona.BackgroundSummary = TEXT("A cautious trader who protects the market stall.");
	Persona.Role = TEXT("market merchant");
	Persona.Personality = {TEXT("cautious"), TEXT("pragmatic")};
	Persona.SpeakingStyle = TEXT("courteous and concise");
	Persona.Goals = {TEXT("protect the stall")};

	FString Error;
	TestTrue(TEXT("A complete bounded Persona is valid"), Persona.IsValid(&Error));
	TestTrue(TEXT("A valid Persona has no error"), Error.IsEmpty());

	Persona.BackgroundSummary = FString::ChrN(ZLSocialPersonaLimits::MaxBackgroundSummaryLength + 1, TEXT('a'));
	TestFalse(TEXT("An oversized background is rejected"), Persona.IsValid(&Error));
	TestFalse(TEXT("A rejected Persona provides a stable error"), Error.IsEmpty());

	Persona.BackgroundSummary = TEXT("A cautious trader who protects the market stall.");
	Persona.InitialRelationship.Trust = 1.1f;
	TestFalse(TEXT("Out-of-range initial relationship is rejected"), Persona.IsValid());

	Persona.InitialRelationship.Trust = 0.0f;
	FString Json;
	TestTrue(TEXT("A valid Persona serializes to JSON"), FZLSocialPersonaJsonCodec::Serialize(Persona, Json, Error));
	FZLSocialPersonaData RoundTrip;
	TestTrue(TEXT("Serialized Persona parses back"), FZLSocialPersonaJsonCodec::Deserialize(Json, RoundTrip, Error));
	TestEqual(TEXT("Round trip preserves stable ID"), RoundTrip.StableId, Persona.StableId);
	TestFalse(TEXT("Unknown JSON fields are rejected"), FZLSocialPersonaJsonCodec::Deserialize(Json.LeftChop(1) + TEXT(",\"unexpected\":true}"), RoundTrip, Error));
	FString BatchJson;
	TestTrue(TEXT("A valid batch serializes"), FZLSocialPersonaJsonCodec::SerializeBatch({Persona}, BatchJson, Error));
	TArray<FZLSocialPersonaData> Batch;
	TestTrue(TEXT("A valid batch parses"), FZLSocialPersonaJsonCodec::DeserializeBatch(BatchJson, Batch, Error));
	TestEqual(TEXT("Batch preserves item count"), Batch.Num(), 1);
	TestFalse(TEXT("Batch rejects duplicate stable IDs"), FZLSocialPersonaJsonCodec::SerializeBatch({Persona, Persona}, BatchJson, Error));

	return true;
}

#endif
