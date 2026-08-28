#include "Misc/AutomationTest.h"
#include "ZLSocialPropagation.h"
#include "ZLSocialEventRouter.h"
#include "ZLSocialSimulation.h"
#include "ZLSocialTags.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialPropagationTest, "ZL.Social.Propagation.BoundsAndConfirmation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialPropagationTest::RunTest(const FString& Parameters)
{
	FZLSocialEventRouter Router;
	FZLSocialEvent Root;
	TestTrue(TEXT("Root event is created"), Router.CreateEvent(ZLSocialTags::Event_Punch, TEXT("player"), TEXT("victim"), FVector::ZeroVector, 10.0, Root));

	FZLSocialPropagation Propagation;
	FZLSocialReportConfirmation Confirmation;
	Confirmation.SourceEvent = Root;
	Confirmation.ReporterId = TEXT("witness");
	Confirmation.CausationId = FGuid::NewGuid();
	Confirmation.ConfirmedAtSeconds = 10.5;
	Confirmation.ReporterConfidence = 0.9f;
	for (int32 Index = 0; Index < 8; ++Index)
	{
		Confirmation.ReceiverIds.Add(FName(*FString::Printf(TEXT("receiver_%d"), Index)));
	}

	FZLSocialPropagationResult Result;
	TestTrue(TEXT("Explicit confirmation creates propagation"), Propagation.ConfirmReport(Confirmation, Result));
	TestEqual(TEXT("Fan-out is capped"), Result.DerivedEvents.Num(), ZLSocialEventLimits::MaxFanOut);
	TestEqual(TEXT("Excess receivers are rejected"), Result.RejectedReceivers, 2);
	TestEqual(TEXT("Budget is consumed per derived event"), Result.RemainingRootBudget, Root.ChainBudget - ZLSocialEventLimits::MaxFanOut);
	const FZLSocialEvent& Derived = Result.DerivedEvents[0];
	TestEqual(TEXT("Root identity is preserved"), Derived.RootEventId, Root.EventId);
	TestEqual(TEXT("Parent is the reported event"), Derived.ParentEventId, Root.EventId);
	TestEqual(TEXT("Original source is immutable"), Derived.SourceId, Root.SourceId);
	TestEqual(TEXT("Original target is immutable"), Derived.TargetId, Root.TargetId);
	TestEqual(TEXT("Reporter is explicit"), Derived.ReporterId, Confirmation.ReporterId);
	TestEqual(TEXT("Derived event uses social channel"), Derived.Channels, static_cast<int32>(EZLSocialPerceptionChannel::Social));
	TestTrue(TEXT("Confidence decays deterministically"), FMath::IsNearlyEqual(Derived.Confidence, 0.72f, 0.0001f));
	TestTrue(TEXT("Derived event remains valid"), Derived.IsValid(10.5));

	FZLSocialPropagationResult DuplicateResult;
	TestFalse(TEXT("Same reporter cannot report the root twice"), Propagation.ConfirmReport(Confirmation, DuplicateResult));
	TestTrue(TEXT("Duplicate reporter is identified"), DuplicateResult.bDuplicateReporter);

	FZLSocialReportConfirmation SecondHop;
	SecondHop.SourceEvent = Derived;
	SecondHop.ReporterId = TEXT("receiver_0");
	SecondHop.ReceiverIds = {TEXT("authority")};
	SecondHop.CausationId = FGuid::NewGuid();
	SecondHop.ConfirmedAtSeconds = 10.75;
	FZLSocialPropagationResult SecondHopResult;
	TestTrue(TEXT("Second hop is allowed"), Propagation.ConfirmReport(SecondHop, SecondHopResult));
	TestEqual(TEXT("Second hop reaches maximum depth"), SecondHopResult.DerivedEvents[0].ChainDepth, ZLSocialEventLimits::MaxChainDepth);

	FZLSocialReportConfirmation ThirdHop = SecondHop;
	ThirdHop.SourceEvent = SecondHopResult.DerivedEvents[0];
	ThirdHop.ReporterId = TEXT("authority");
	ThirdHop.ReceiverIds = {TEXT("beyond")};
	ThirdHop.CausationId = FGuid::NewGuid();
	ThirdHop.ConfirmedAtSeconds = 11.0;
	FZLSocialPropagationResult ThirdHopResult;
	TestFalse(TEXT("Propagation beyond maximum depth is rejected"), Propagation.ConfirmReport(ThirdHop, ThirdHopResult));

	FZLSocialReportConfirmation Expired = Confirmation;
	Expired.SourceEvent = Root;
	Expired.ReporterId = TEXT("late_witness");
	Expired.ReceiverIds = {TEXT("authority")};
	Expired.CausationId = FGuid::NewGuid();
	Expired.ConfirmedAtSeconds = Root.ExpiresAtSeconds + 0.01;
	TestFalse(TEXT("Expired report is rejected"), Propagation.ConfirmReport(Expired, ThirdHopResult));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialReportIntentBoundaryTest, "ZL.Social.Propagation.IntentRequiresConfirmation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialReportIntentBoundaryTest::RunTest(const FString& Parameters)
{
	FZLSocialSimulation Simulation;
	FZLSocialAgentProfile Witness;
	Witness.AgentId = TEXT("witness");
	Witness.Personality.Brave = 0.2f;
	Witness.Personality.FearSensitivity = 0.0f;
	Witness.Personality.Curiosity = 0.0f;
	Witness.Personality.Justice = 1.0f;
	Witness.Personality.Aggression = 0.0f;
	Witness.Personality.Social = 1.0f;
	Witness.bCanAssist = false;
	Witness.bCanConfront = false;
	TestTrue(TEXT("Witness registers"), Simulation.RegisterAgent(Witness));

	FZLSocialEvent Root;
	TestTrue(TEXT("Punch root is created"), Simulation.CreateEvent(ZLSocialTags::Event_Punch, TEXT("player"), Witness.AgentId, FVector::ZeroVector, 20.0, Root));
	TArray<FZLSocialIntentCommand> Commands;
	FZLSocialProcessingStats Stats;
	const auto Visible = [](const FVector&, const FVector&) { return true; };
	TestTrue(TEXT("Root is processed"), Simulation.ProcessEvent(Root, 20.0, Commands, Stats, Visible));
	TestEqual(TEXT("Witness emits one intent"), Commands.Num(), 1);
	TestTrue(TEXT("Rule selects report"), Commands[0].Intent == ZLSocialTags::Intent_Report);

	FZLSocialReportConfirmation Confirmation;
	Confirmation.SourceEvent = Root;
	Confirmation.ReporterId = Witness.AgentId;
	Confirmation.ReceiverIds = {TEXT("guard")};
	Confirmation.CausationId = FGuid::NewGuid();
	Confirmation.ConfirmedAtSeconds = 20.5;
	FZLSocialPropagationResult PropagationResult;
	TestTrue(TEXT("Report intent did not consume confirmation side effect"), Simulation.ConfirmReport(Confirmation, PropagationResult));
	TestEqual(TEXT("Explicit confirmation creates the event"), PropagationResult.DerivedEvents.Num(), 1);
	return true;
}

#endif
