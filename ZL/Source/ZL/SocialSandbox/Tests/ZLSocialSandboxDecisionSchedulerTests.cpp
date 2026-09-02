#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "SocialSandbox/Decision/ZLSocialSandboxDecisionScheduler.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLSocialSandboxDecisionSchedulerTest,
	"ZL.Social.Sandbox.ContinuousDecisionScheduler",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialSandboxDecisionSchedulerTest::RunTest(const FString&)
{
	FZLSocialSandboxDecisionScheduler Scheduler(1.0);
	FZLSocialSandboxScheduledDecision Speech;
	Speech.Reason = EZLSocialSandboxDecisionTriggerReason::Speech;
	Speech.Observation.EventId = FGuid::NewGuid();
	TestTrue(TEXT("First trigger is accepted"), Scheduler.Queue(Speech) == EZLSocialSandboxQueueResult::Accepted);

	FZLSocialSandboxScheduledDecision Ready;
	double Delay = 0.0;
	TestTrue(TEXT("First trigger dispatches immediately"), Scheduler.TakeReady(10.0, Ready, Delay));
	Scheduler.MarkDispatched(10.0, Ready);
	TestTrue(TEXT("Scheduler records one in-flight request"), Scheduler.IsInFlight());

	FZLSocialSandboxScheduledDecision Near = Speech;
	Near.Reason = EZLSocialSandboxDecisionTriggerReason::DistanceNear;
	FZLSocialSandboxScheduledDecision Hit = Speech;
	Hit.Reason = EZLSocialSandboxDecisionTriggerReason::Hit;
	TestTrue(TEXT("New trigger queues while request is in flight"), Scheduler.Queue(Near) == EZLSocialSandboxQueueResult::Accepted);
	TestTrue(TEXT("Latest trigger coalesces the pending slot"), Scheduler.Queue(Hit) == EZLSocialSandboxQueueResult::Coalesced);
	TestEqual(TEXT("Pending storage remains bounded to one slot"), Scheduler.GetCoalescedCount(), 1);
	TestFalse(TEXT("Pending trigger cannot dispatch while in flight"), Scheduler.TakeReady(11.0, Ready, Delay));

	Scheduler.MarkCompleted();
	TestTrue(TEXT("Latest coalesced trigger dispatches after completion"), Scheduler.TakeReady(11.0, Ready, Delay));
	TestTrue(TEXT("Latest trigger wins"), Ready.Reason == EZLSocialSandboxDecisionTriggerReason::Hit);
	Scheduler.MarkDispatched(11.0, Ready);
	Scheduler.MarkCompleted();

	for (int32 Index = 0; Index < FZLSocialSandboxDecisionScheduler::MaxAutomaticReplans; ++Index)
	{
		FZLSocialSandboxScheduledDecision Automatic = Speech;
		Automatic.Reason = EZLSocialSandboxDecisionTriggerReason::PlanCompleted;
		TestTrue(TEXT("Bounded automatic replan is accepted"), Scheduler.Queue(Automatic) == EZLSocialSandboxQueueResult::Accepted);
		TestFalse(TEXT("Cooldown delays automatic replan"), Scheduler.TakeReady(11.1 + Index, Ready, Delay));
		TestTrue(TEXT("A positive cooldown delay is reported"), Delay > 0.0);
		TestTrue(TEXT("Automatic replan becomes ready"), Scheduler.TakeReady(12.0 + Index, Ready, Delay));
		Scheduler.MarkDispatched(12.0 + Index, Ready);
		Scheduler.MarkCompleted();
	}
	FZLSocialSandboxScheduledDecision Excess = Speech;
	Excess.Reason = EZLSocialSandboxDecisionTriggerReason::PlanCompleted;
	TestTrue(TEXT("Automatic replanning stops at the hard limit"), Scheduler.Queue(Excess) == EZLSocialSandboxQueueResult::AutomaticLimit);
	TestFalse(TEXT("Rejected automatic replan does not add pending work"), Scheduler.HasPending());

	TestTrue(TEXT("External input resets the automatic budget"), Scheduler.Queue(Speech) == EZLSocialSandboxQueueResult::Accepted);
	TestEqual(TEXT("Automatic budget resets after external input"), Scheduler.GetAutomaticReplanCount(), 0);
	return true;
}

#endif
