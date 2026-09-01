#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "SocialSandbox/ZLSocialSandboxCombat.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLSocialSandboxCombatValidationTest,
	"ZL.Social.Sandbox.AttackValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialSandboxCombatValidationTest::RunTest(const FString&)
{
	FZLSocialSandboxAttackValidationContext Context;
	Context.Distance = 200.0f;
	Context.NowSeconds = 10.0;
	Context.bTargetValid = true;
	Context.bPlayerExecutable = true;
	TestTrue(TEXT("In-range executable attack is accepted"), FZLSocialSandboxCombat::ValidatePlayerAttack(Context).bAccepted);

	Context.Distance = 221.0f;
	TestEqual(TEXT("Out-of-range attack is rejected"), FZLSocialSandboxCombat::ValidatePlayerAttack(Context).ReasonCode, FName(TEXT("OutOfAttackRange")));
	Context.Distance = 200.0f;
	Context.LastAttackSeconds = 9.5;
	TestEqual(TEXT("Cooldown attack is rejected"), FZLSocialSandboxCombat::ValidatePlayerAttack(Context).ReasonCode, FName(TEXT("AttackCooldown")));
	Context.LastAttackSeconds = -DBL_MAX;
	Context.bPlayerExecutable = false;
	TestEqual(TEXT("Busy player attack is rejected"), FZLSocialSandboxCombat::ValidatePlayerAttack(Context).ReasonCode, FName(TEXT("PlayerBusy")));
	Context.bPlayerExecutable = true;
	Context.bTargetIncapacitated = true;
	TestEqual(TEXT("Incapacitated target attack is rejected"), FZLSocialSandboxCombat::ValidatePlayerAttack(Context).ReasonCode, FName(TEXT("TargetIncapacitated")));
	return true;
}

#endif
