#include "Misc/AutomationTest.h"
#include "ZLSocialDebug.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialBenchmarkTest, "ZL.Social.Debug.Benchmark120", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialBenchmarkTest::RunTest(const FString& Parameters)
{
	const FZLSocialBenchmarkResult Result = ZLSocialDebug::RunDeterministicBenchmark(120);
	AddInfo(FString::Printf(TEXT("benchmark agents=%d cells=%d candidates=%d perceived=%d rules=%d processing_ms=%.3f"),
		Result.RegisteredAgents, Result.Processing.Spatial.CellsVisited, Result.Processing.Spatial.CandidatesExamined,
		Result.Processing.PerceivedAgents, Result.Processing.RuleEvaluations, Result.Processing.ProcessingMilliseconds));
	TestEqual(TEXT("Benchmark registers 120 agents"), Result.RegisteredAgents, 120);
	TestEqual(TEXT("Gunshot spatial query reaches 120 agents"), Result.Processing.Spatial.ResultsReturned, 120);
	TestEqual(TEXT("All agents pass perception"), Result.Processing.PerceivedAgents, 120);
	TestEqual(TEXT("All agents receive rule evaluation"), Result.Processing.RuleEvaluations, 120);
	TestTrue(TEXT("Processing time is measured"), Result.Processing.ProcessingMilliseconds >= 0.0);

	FZLSocialAgentDebugSnapshot Snapshot;
	Snapshot.AgentId = TEXT("safe_agent");
	const FString Formatted = ZLSocialDebug::FormatAgent(Snapshot);
	TestTrue(TEXT("Inspector includes selected agent"), Formatted.Contains(TEXT("safe_agent")));
	TestFalse(TEXT("Inspector excludes dialogue data"), Formatted.Contains(TEXT("dialogue"), ESearchCase::IgnoreCase));
	TestFalse(TEXT("Inspector excludes credentials"), Formatted.Contains(TEXT("api_key"), ESearchCase::IgnoreCase));

	const FZLSocialMilestone6BenchmarkResult Milestone6 = ZLSocialDebug::RunMilestone6Benchmark(120, 5);
	AddInfo(FString::Printf(TEXT("m6_benchmark level1=%d important=%d registered=%d propagation=%d rejected=%d duplicates=%d relationships=%d factions=%d long_memory=%d rules=%d rule_ms=%.3f processing_ms=%.3f"),
		Milestone6.Level1Agents, Milestone6.ImportantAgents, Milestone6.RegisteredAgents, Milestone6.Aggregate.PropagationCreated,
		Milestone6.Aggregate.PropagationRejected, Milestone6.Aggregate.RootDuplicates, Milestone6.Aggregate.RelationshipEdges,
		Milestone6.Aggregate.FactionStandings, Milestone6.Aggregate.LongMemoryItems, Milestone6.Processing.RuleEvaluations,
		Milestone6.Processing.RuleEvaluationMilliseconds, Milestone6.Processing.ProcessingMilliseconds));
	TestEqual(TEXT("M6 benchmark registers 120 Level 1 agents"), Milestone6.Level1Agents, 120);
	TestEqual(TEXT("M6 benchmark registers five Important NPCs"), Milestone6.ImportantAgents, 5);
	TestEqual(TEXT("M6 benchmark registers 125 total agents"), Milestone6.RegisteredAgents, 125);
	TestEqual(TEXT("M6 propagation stays within requested fan-out"), Milestone6.Aggregate.PropagationCreated, 5);
	TestTrue(TEXT("M6 benchmark creates sparse relationship edges"), Milestone6.Aggregate.RelationshipEdges > 0 && Milestone6.Aggregate.RelationshipEdges < Milestone6.RegisteredAgents * 2);
	TestEqual(TEXT("Each Important NPC receives one anchored long memory"), Milestone6.Aggregate.LongMemoryItems, 5);
	TestEqual(TEXT("Authority creates one faction standing"), Milestone6.Aggregate.FactionStandings, 1);

	Snapshot.AgentLevel = EZLSocialAgentLevel::Important;
	Snapshot.FactionId = TEXT("guards");
	Snapshot.OccupationId = TEXT("guard");
	Snapshot.RootEventId = FGuid::NewGuid();
	Snapshot.ParentEventId = FGuid::NewGuid();
	Snapshot.ChainDepth = 1;
	Snapshot.ChainBudget = 5;
	Snapshot.SourceChannel = EZLSocialPerceptionChannel::Social;
	Snapshot.SourceConfidence = 0.8f;
	Snapshot.bHasRelationship = true;
	Snapshot.bHasFactionStanding = true;
	Snapshot.ReasonCodes = {TEXT("source.uncertainty")};
	const FString M6Formatted = ZLSocialDebug::FormatAgent(Snapshot);
	TestTrue(TEXT("Inspector includes chain metadata"), M6Formatted.Contains(TEXT("root=")) && M6Formatted.Contains(TEXT("depth=1")) && M6Formatted.Contains(TEXT("budget=5")));
	TestTrue(TEXT("Inspector includes relationship faction memory and reasons"), M6Formatted.Contains(TEXT("relationship=[present=1")) && M6Formatted.Contains(TEXT("faction_standing=[present=1")) && M6Formatted.Contains(TEXT("memory=[short=")) && M6Formatted.Contains(TEXT("source.uncertainty")));
	return true;
}

#endif
