#include "Misc/AutomationTest.h"
#include "ZLSocialPersona.h"

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

	return true;
}

#endif
