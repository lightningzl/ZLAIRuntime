#include "Misc/AutomationTest.h"
#include "ZLSocialInputValidation.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialInputValidationTest, "ZL.Social.InputValidation.Boundaries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialInputValidationTest::RunTest(const FString&)
{
	TestEqual(TEXT("Blank is rejected"), FZLSocialInputValidation::ValidateText(TEXT("  \t")), EZLSocialInputValidationResult::Empty);
	TestEqual(TEXT("512 characters are accepted"), FZLSocialInputValidation::ValidateText(FString::ChrN(512, TEXT('a'))), EZLSocialInputValidationResult::Valid);
	TestEqual(TEXT("513 characters are rejected"), FZLSocialInputValidation::ValidateText(FString::ChrN(513, TEXT('a'))), EZLSocialInputValidationResult::TooLong);
	const FString Emoji = FString::Chr(0xD83D) + FString::Chr(0xDE03);
	TestEqual(TEXT("UTF-16 surrogate pair counts as one code point"), FZLSocialInputValidation::CountUnicodeCodePoints(Emoji), 1);
	TestEqual(TEXT("InEar requires target"), FZLSocialInputValidation::ValidateSpeech(TEXT("hello"), EZLSocialSpeechMode::InEar, NAME_None), EZLSocialInputValidationResult::InEarRequiresTarget);
	TestEqual(TEXT("InEar accepts explicit target"), FZLSocialInputValidation::ValidateSpeech(TEXT("hello"), EZLSocialSpeechMode::InEar, TEXT("npc")), EZLSocialInputValidationResult::Valid);
	return true;
}

#endif
