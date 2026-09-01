#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "SocialSandbox/ZLSocialSandboxMultiNpcDecision.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLSocialSandboxMultiNpcDecisionTest,
	"ZL.Social.Sandbox.MultiNpcDecisionBounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialSandboxMultiNpcDecisionTest::RunTest(const FString& Parameters)
{
	FZLSocialSandboxMultiNpcDecision Coordinator;
	TestTrue(TEXT("Guard registers"), Coordinator.RegisterNpc(TEXT("npc_guard")));
	TestTrue(TEXT("Merchant registers"), Coordinator.RegisterNpc(TEXT("npc_merchant")));
	TestTrue(TEXT("Rival registers"), Coordinator.RegisterNpc(TEXT("npc_rival")));
	TestTrue(TEXT("Civilian registers"), Coordinator.RegisterNpc(TEXT("npc_civilian")));
	TestFalse(TEXT("Registry is bounded"), Coordinator.RegisterNpc(TEXT("npc_extra")));

	FZLSocialSandboxScheduledDecision Scheduled;
	Scheduled.Observation.EventId = FGuid::NewGuid();
	TestEqual(TEXT("First queue accepted"), Coordinator.Queue(TEXT("npc_guard"), Scheduled), EZLSocialSandboxQueueResult::Accepted);
	TestEqual(TEXT("Latest pending coalesces"), Coordinator.Queue(TEXT("npc_guard"), Scheduled), EZLSocialSandboxQueueResult::Coalesced);
	Coordinator.Queue(TEXT("npc_merchant"), Scheduled);
	Coordinator.Queue(TEXT("npc_rival"), Scheduled);

	FZLSocialSandboxNpcDispatch First;
	double Delay = 0.0;
	TestTrue(TEXT("First request dispatches"), Coordinator.TakeNext(1.0, First, Delay));
	FZLSocialSandboxNpcDispatch Second;
	TestTrue(TEXT("Second request dispatches"), Coordinator.TakeNext(1.0, Second, Delay));
	TestEqual(TEXT("Global in-flight cap reached"), Coordinator.GetInFlightCount(), 2);
	FZLSocialSandboxNpcDispatch Blocked;
	TestFalse(TEXT("Third request waits for capacity"), Coordinator.TakeNext(1.0, Blocked, Delay));
	TestNotEqual(TEXT("Stable rotation does not duplicate NPC"), First.NpcId, Second.NpcId);

	Coordinator.MarkCompleted(First.NpcId);
	TestEqual(TEXT("Completion releases one slot"), Coordinator.GetInFlightCount(), 1);
	TestTrue(TEXT("Pending NPC dispatches after completion"), Coordinator.TakeNext(1.0, Blocked, Delay));
	TestEqual(TEXT("Cap remains bounded"), Coordinator.GetInFlightCount(), 2);
	Coordinator.Reset();
	TestEqual(TEXT("Reset clears in-flight count"), Coordinator.GetInFlightCount(), 0);
	TestFalse(TEXT("Reset clears pending"), Coordinator.HasPending(TEXT("npc_guard")));
	return true;
}

#endif
