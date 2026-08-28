#include "Misc/AutomationTest.h"
#include "ZLSocialEventRouter.h"
#include "ZLSocialTags.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSpatialIndexTest, "ZL.Social.SpatialIndex.BoundedQuery", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialSpatialIndexTest::RunTest(const FString& Parameters)
{
	FZLSocialSpatialIndex Index(1000.0f);
	for (int32 AgentIndex = 0; AgentIndex < 120; ++AgentIndex)
	{
		FZLSocialAgentProfile Profile;
		Profile.AgentId = FName(*FString::Printf(TEXT("agent_%03d"), AgentIndex));
		Profile.Position = FVector(AgentIndex * 1000.0f, 0.0f, 0.0f);
		TestTrue(TEXT("Agent registers"), Index.RegisterAgent(Profile));
	}

	TArray<FZLSocialAgentProfile> Results;
	FZLSocialSpatialQueryStats Stats;
	Index.QueryRadius(FVector::ZeroVector, 1100.0f, Results, &Stats);
	TestEqual(TEXT("Only nearby agents returned"), Results.Num(), 2);
	TestTrue(TEXT("Query examines fewer than all registered agents"), Stats.CandidatesExamined < Stats.RegisteredAgents);
	TestTrue(TEXT("Moving an agent across cells succeeds"), Index.UpdateAgentPosition(TEXT("agent_119"), FVector(500.0f, 0.0f, 0.0f)));
	Index.QueryRadius(FVector::ZeroVector, 1100.0f, Results, &Stats);
	TestEqual(TEXT("Moved agent becomes queryable"), Results.Num(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialEventRouterTest, "ZL.Social.EventRouter.LifecycleAndDedup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialEventRouterTest::RunTest(const FString& Parameters)
{
	FZLSocialEventRouter Router;
	FZLSocialAgentProfile Profile;
	Profile.AgentId = TEXT("witness");
	Profile.Position = FVector(100.0f, 0.0f, 0.0f);
	TestTrue(TEXT("Witness registers"), Router.RegisterAgent(Profile));

	FZLSocialEvent Event;
	TestTrue(TEXT("Punch uses a valid preset"), Router.CreateEvent(ZLSocialTags::Event_Punch, TEXT("player"), NAME_None, FVector::ZeroVector, 10.0, Event));
	TestTrue(TEXT("Root event identifies itself"), Event.IsRootEvent());
	TestEqual(TEXT("Root event starts at depth zero"), Event.ChainDepth, 0);
	TestEqual(TEXT("Root event has bounded budget"), Event.ChainBudget, ZLSocialEventLimits::DefaultChainBudget);
	TestEqual(TEXT("Punch radius is configured"), Event.Radius, 1000.0f);
	FZLSocialEventRouteResult Result;
	TestTrue(TEXT("Valid event routes"), Router.RouteEvent(Event, 10.5, Result));
	TestEqual(TEXT("Witness receives event once"), Result.Candidates.Num(), 1);
	TestTrue(TEXT("Repeated route is accepted"), Router.RouteEvent(Event, 10.5, Result));
	TestEqual(TEXT("Repeated route is deduplicated"), Result.Candidates.Num(), 0);
	TestEqual(TEXT("Duplicate is counted"), Result.DuplicateCount, 1);
	TestFalse(TEXT("Expired event is rejected"), Router.RouteEvent(Event, 13.0, Result));
	FZLSocialEvent Gunshot;
	TestTrue(TEXT("Gunshot uses a valid preset"), Router.CreateEvent(ZLSocialTags::Event_Gunshot, TEXT("player"), NAME_None, FVector::ZeroVector, 10.0, Gunshot));
	TestEqual(TEXT("Gunshot radius is configured"), Gunshot.Radius, 10000.0f);
	TestEqual(TEXT("Gunshot severity is extreme"), Gunshot.Severity, 1.0f);
	FZLSocialEvent Help;
	TestTrue(TEXT("Help uses a valid preset"), Router.CreateEvent(ZLSocialTags::Event_Help, TEXT("player"), TEXT("witness"), FVector::ZeroVector, 10.0, Help));
	TestEqual(TEXT("Help radius is configured"), Help.Radius, 1200.0f);
	TestTrue(TEXT("Targeted Help includes Direct channel"), Help.HasChannel(EZLSocialPerceptionChannel::Direct));
	TestFalse(TEXT("Reserved event without milestone preset is rejected"), Router.CreateEvent(ZLSocialTags::Event_Steal, TEXT("player"), NAME_None, FVector::ZeroVector, 10.0, Event));

	FZLSocialEvent InvalidChain = Help;
	InvalidChain.ChainDepth = ZLSocialEventLimits::MaxChainDepth + 1;
	TestFalse(TEXT("Depth beyond hard limit is invalid"), InvalidChain.IsValid(10.0));
	InvalidChain = Help;
	InvalidChain.ChainBudget = ZLSocialEventLimits::MaxChainBudget + 1;
	TestFalse(TEXT("Budget beyond hard limit is invalid"), InvalidChain.IsValid(10.0));
	InvalidChain = Help;
	InvalidChain.RootEventId.Invalidate();
	TestFalse(TEXT("Missing root identifier is invalid"), InvalidChain.IsValid(10.0));
	return true;
}

#endif
