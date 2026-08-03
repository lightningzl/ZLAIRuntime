#include "Misc/AutomationTest.h"
#include "ZLSocialGameplayAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialGameplayIntegrationTest, "ZL.Social.Gameplay.HeadlessVerticalSlice", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialGameplayIntegrationTest::RunTest(const FString& Parameters)
{
	FZLSocialGameplayAdapter Adapter;
	for (int32 Index = 0; Index < 4; ++Index)
	{
		FZLSocialAgentProfile Agent;
		Agent.AgentId = FName(*FString::Printf(TEXT("npc_%d"), Index));
		Agent.Position = FVector(Index * 100.0f, 0.0f, 0.0f);
		Agent.Personality.Curiosity = Index == 0 ? 1.0f : 0.1f;
		Agent.Personality.FearSensitivity = Index == 1 ? 1.0f : 0.1f;
		Agent.Personality.Justice = Index == 2 ? 1.0f : 0.1f;
		Agent.Personality.Aggression = Index == 3 ? 1.0f : 0.1f;
		Agent.Personality.Brave = Index == 3 ? 1.0f : 0.2f;
		TestTrue(TEXT("Gameplay agent registers"), Adapter.RegisterLevel1Agent(Agent));
	}

	TArray<FZLSocialIntentCommand> Delivered;
	Adapter.SetIntentHandler([&Delivered](const FZLSocialIntentCommand& Command) { Delivered.Add(Command); });
	FZLSocialProcessingStats Stats;
	TestTrue(TEXT("Gameplay explicitly produces punch"), Adapter.ProducePunch(TEXT("player"), TEXT("npc_0"), FVector::ZeroVector, 10.0, &Stats));
	TestEqual(TEXT("All nearby agents perceive and decide"), Delivered.Num(), 4);
	TestEqual(TEXT("One rule evaluation per perceived agent"), Stats.RuleEvaluations, 4);
	for (const FZLSocialIntentCommand& Command : Delivered)
	{
		TestTrue(TEXT("Intent command has stable event id"), Command.EventId.IsValid());
		TestTrue(TEXT("Intent command has a gameplay tag"), Command.Intent.IsValid());
		const FZLSocialAgentState* State = Adapter.GetSimulation().FindAgentState(Command.AgentId);
		TestTrue(TEXT("State exists"), State != nullptr);
		TestEqual(TEXT("Perceived event entered short memory"), State == nullptr ? 0 : State->ShortMemory.Num(), 1);
	}
	return true;
}

#endif
