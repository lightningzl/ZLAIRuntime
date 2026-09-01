#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "SocialSandbox/ZLSocialSandboxConflictState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSandboxConflictStateTest, "ZL.Social.Sandbox.ConflictState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialSandboxConflictStateTest::RunTest(const FString& Parameters)
{
	FZLSocialSandboxConflictState State;
	TestEqual(TEXT("Initial stance is calm"), State.GetLevel(), EZLSocialSandboxConflictLevel::Calm);
	const FZLSocialSandboxConflictTransition Attack = State.Apply(EZLSocialSandboxConflictEvent::Attack);
	TestEqual(TEXT("Attack escalates"), Attack.Current, EZLSocialSandboxConflictLevel::Escalated);
	TestTrue(TEXT("Escalation enables defense"), Attack.bShouldDefend);
	const FZLSocialSandboxConflictTransition Retreat = State.Apply(EZLSocialSandboxConflictEvent::DistanceFar);
	TestEqual(TEXT("Retreat begins recovery"), Retreat.Current, EZLSocialSandboxConflictLevel::Recovering);
	TestFalse(TEXT("Recovery disables defense"), Retreat.bShouldDefend);
	TestEqual(TEXT("Second calming event returns calm"), State.Apply(EZLSocialSandboxConflictEvent::PlayerStop).Current, EZLSocialSandboxConflictLevel::Calm);
	TestEqual(TEXT("Planner engagement escalates"), State.Apply(EZLSocialSandboxConflictEvent::PlannerEngage).Current, EZLSocialSandboxConflictLevel::Escalated);
	TestEqual(TEXT("Planner disengagement recovers"), State.Apply(EZLSocialSandboxConflictEvent::PlannerDisengage).Current, EZLSocialSandboxConflictLevel::Recovering);
	return true;
}

#endif
