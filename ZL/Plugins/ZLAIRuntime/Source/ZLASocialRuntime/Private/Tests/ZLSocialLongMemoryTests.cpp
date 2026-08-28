#include "Misc/AutomationTest.h"
#include "ZLSocialEventRouter.h"
#include "ZLSocialSimulation.h"
#include "ZLSocialTags.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialLongMemoryTest, "ZL.Social.Memory.ImportantPromotionRetrievalAndBounds", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialLongMemoryTest::RunTest(const FString& Parameters)
{
	FZLSocialSimulation Simulation;
	FZLSocialAgentProfile Level1;
	Level1.AgentId = TEXT("level1");
	Level1.ShortMemoryCapacity = ZLSocialMemoryLimits::MaxShortCapacity;
	TestTrue(TEXT("Level 1 registers"), Simulation.RegisterAgent(Level1));
	const FZLSocialAgentState* Level1State = Simulation.FindAgentState(Level1.AgentId);
	TestTrue(TEXT("Level 1 short memory stays fixed at six"), Level1State != nullptr && Level1State->ShortMemory.GetCapacity() == ZLSocialMemoryLimits::Level1ShortCapacity);
	TestTrue(TEXT("Level 1 has no long memory"), Level1State != nullptr && Level1State->LongMemory.GetCapacity() == 0);

	FZLSocialAgentProfile Important;
	Important.AgentId = TEXT("guard");
	Important.AgentLevel = EZLSocialAgentLevel::Important;
	Important.FactionId = TEXT("guards");
	Important.ShortMemoryCapacity = 16;
	Important.LongMemoryCapacity = 8;
	TestTrue(TEXT("Important NPC registers"), Simulation.RegisterAgent(Important));
	const auto Visible = [](const FVector&, const FVector&) { return true; };
	for (int32 Index = 0; Index < 20; ++Index)
	{
		FZLSocialEvent Event;
		TestTrue(TEXT("Bounded event is created"), Simulation.CreateEvent(ZLSocialTags::Event_Gunshot, FName(*FString::Printf(TEXT("subject_%02d"), Index)), Important.AgentId, FVector(Index * 10.0f, 0.0f, 0.0f), 10.0 + Index, Event));
		Event.bAnchored = Index == 0;
		TArray<FZLSocialIntentCommand> Commands;
		FZLSocialProcessingStats Stats;
		TestTrue(TEXT("Important NPC processes event"), Simulation.ProcessEvent(Event, 10.0 + Index, Commands, Stats, Visible));
	}

	const FZLSocialAgentState* ImportantState = Simulation.FindAgentState(Important.AgentId);
	TestNotNull(TEXT("Important state exists"), ImportantState);
	if (ImportantState == nullptr) { return false; }
	TestEqual(TEXT("Important short memory is bounded"), ImportantState->ShortMemory.Num(), 16);
	TestEqual(TEXT("Important long memory is bounded"), ImportantState->LongMemory.Num(), 8);

	FZLSocialLongMemoryQuery Query;
	Query.TopK = 3;
	const TArray<FZLSocialMemoryEntry> First = ImportantState->LongMemory.Retrieve(Query, 40.0);
	const TArray<FZLSocialMemoryEntry> Second = ImportantState->LongMemory.Retrieve(Query, 40.0);
	TestEqual(TEXT("Top K is bounded"), First.Num(), 3);
	TestEqual(TEXT("Repeated retrieval has same size"), First.Num(), Second.Num());
	for (int32 Index = 0; Index < First.Num(); ++Index)
	{
		TestEqual(TEXT("Repeated retrieval is stable"), First[Index].EventId, Second[Index].EventId);
	}

	FZLSocialLongMemory Standalone(2);
	FZLSocialMemoryEntry Low;
	Low.EventId = FGuid::NewGuid();
	Low.RootEventId = Low.EventId;
	Low.Importance = 0.1f;
	Low.TimestampSeconds = 1.0;
	TestFalse(TEXT("Low importance is not promoted"), Standalone.AddPromoted(Low, 1.0));
	Low.bAnchored = true;
	TestTrue(TEXT("Explicit anchor promotes low importance"), Standalone.AddPromoted(Low, 1.0));

	Query = FZLSocialLongMemoryQuery();
	Query.SubjectId = First[0].SourceId;
	Query.Location = First[0].Position;
	Query.LocationRadius = 1.0f;
	Query.MinTimestampSeconds = First[0].TimestampSeconds;
	Query.MaxTimestampSeconds = First[0].TimestampSeconds;
	Query.TopK = ZLSocialMemoryLimits::MaxTopK + 100;
	const TArray<FZLSocialMemoryEntry> Filtered = ImportantState->LongMemory.Retrieve(Query, 40.0);
	TestEqual(TEXT("Structured filters select the matching memory"), Filtered.Num(), 1);
	return true;
}

#endif
