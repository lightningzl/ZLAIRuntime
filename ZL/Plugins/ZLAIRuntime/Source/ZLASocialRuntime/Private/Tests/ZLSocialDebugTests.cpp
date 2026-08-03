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
	return true;
}

#endif
