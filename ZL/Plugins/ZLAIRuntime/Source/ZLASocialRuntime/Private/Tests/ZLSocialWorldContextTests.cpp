#include "Misc/AutomationTest.h"
#include "ZLSocialWorldContext.h"
#include "ZLSocialWorldContextJson.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialWorldContextValidationTest, "ZL.Social.WorldContext.Validation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialWorldContextValidationTest::RunTest(const FString&)
{
	FZLSocialWorldContextData Context;
	Context.StableId = TEXT("market_district");
	Context.DisplayName = TEXT("Market District");
	Context.Rules = {{TEXT("curfew"), TEXT("The market closes at night under a posted curfew.")}};
	Context.Factions = {{TEXT("city_guard"), TEXT("City Guard"), TEXT("The city guard keeps public order in the district.")}};
	Context.PublicKnowledge = {{TEXT("market_hours"), TEXT("The market is normally open during daylight.")}};

	FString Error;
	TestTrue(TEXT("A complete static World Context is valid"), Context.IsValid(&Error));
	TestTrue(TEXT("A valid World Context has no error"), Error.IsEmpty());
	FString Json;
	TestTrue(TEXT("A valid World Context serializes"), FZLSocialWorldContextJsonCodec::Serialize(Context, Json, Error));
	FZLSocialWorldContextData RoundTrip;
	TestTrue(TEXT("Serialized World Context parses back"), FZLSocialWorldContextJsonCodec::Deserialize(Json, RoundTrip, Error));
	TestEqual(TEXT("Round trip preserves the static context ID"), RoundTrip.StableId, Context.StableId);
	TestEqual(TEXT("Round trip preserves public knowledge count"), RoundTrip.PublicKnowledge.Num(), Context.PublicKnowledge.Num());
	TestFalse(TEXT("Unknown World Context JSON fields are rejected"), FZLSocialWorldContextJsonCodec::Deserialize(Json.LeftChop(1) + TEXT(",\"unexpected\":true}"), RoundTrip, Error));

	Context.Rules.Add(Context.Rules[0]);
	TestFalse(TEXT("Duplicate static rule IDs are rejected"), Context.IsValid(&Error));
	return true;
}

#endif
