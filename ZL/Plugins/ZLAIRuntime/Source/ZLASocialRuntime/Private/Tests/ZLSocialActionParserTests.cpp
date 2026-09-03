#include "Misc/AutomationTest.h"
#include "ZLSocialActionParser.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialActionParserTest, "ZL.Social.ActionParser.Whitelist", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialActionParserTest::RunTest(const FString&)
{
	TestEqual(TEXT("Face English"), FZLSocialActionParser::Parse(TEXT(" FACE ")).Action, EZLSocialActionType::Face);
	TestEqual(TEXT("Approach Chinese"), FZLSocialActionParser::Parse(TEXT("靠近")).Action, EZLSocialActionType::Approach);
	TestEqual(TEXT("MoveAway alias"), FZLSocialActionParser::Parse(TEXT("move_away")).Action, EZLSocialActionType::MoveAway);
	TestFalse(TEXT("Attack is not a social behavior"), FZLSocialActionParser::Parse(TEXT("攻击")).bMatched);
	TestEqual(TEXT("Stop Chinese"), FZLSocialActionParser::Parse(TEXT("停下")).Action, EZLSocialActionType::Stop);
	TestFalse(TEXT("Unknown action is rejected"), FZLSocialActionParser::Parse(TEXT("teleport anywhere")).bMatched);
	TestFalse(TEXT("Narrative does not impersonate an action"), FZLSocialActionParser::Parse(TEXT("I approach the guard")).bMatched);
	return true;
}

#endif
