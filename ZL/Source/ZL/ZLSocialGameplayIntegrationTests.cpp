#include "Misc/AutomationTest.h"
#include "ZLSocialGameplayAdapter.h"
#include "ZLSocialTags.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialImportantNpcVerticalSliceTest, "ZL.Social.Gameplay.ImportantNpcReportVerticalSlice", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialImportantNpcVerticalSliceTest::RunTest(const FString& Parameters)
{
	FZLSocialGameplayAdapter Adapter;
	FZLSocialAgentProfile Victim;
	Victim.AgentId = TEXT("victim");
	Victim.Position = FVector::ZeroVector;
	TestTrue(TEXT("Victim registers"), Adapter.RegisterLevel1Agent(Victim));
	FZLSocialAgentProfile Witness;
	Witness.AgentId = TEXT("witness");
	Witness.Position = FVector(100.0f, 0.0f, 0.0f);
	Witness.Personality.Curiosity = 0.0f;
	Witness.Personality.FearSensitivity = 0.0f;
	Witness.Personality.Justice = 1.0f;
	Witness.Personality.Social = 1.0f;
	Witness.bCanAssist = false;
	Witness.bCanConfront = false;
	TestTrue(TEXT("Witness registers"), Adapter.RegisterLevel1Agent(Witness));

	for (int32 Index = 0; Index < 5; ++Index)
	{
		FZLSocialAgentProfile Important;
		Important.AgentId = FName(*FString::Printf(TEXT("important_guard_%d"), Index));
		Important.AgentLevel = EZLSocialAgentLevel::Important;
		Important.Position = FVector(5000.0f + Index * 100.0f, 0.0f, 0.0f);
		Important.FactionId = TEXT("guards");
		Important.OccupationId = TEXT("guard");
		Important.bCanReceiveReports = true;
		Important.bHasFactionAuthority = Index == 0;
		Important.Personality.Justice = 0.8f;
		Important.Personality.Curiosity = 0.6f;
		TestTrue(TEXT("Important NPC registers"), Adapter.RegisterImportantAgent(Important));
	}
	TestEqual(TEXT("Scenario contains two Level 1 and five Important NPCs"), Adapter.GetSimulation().GetRegisteredAgentCount(), 7);

	TArray<FZLSocialIntentCommand> Delivered;
	Adapter.SetIntentHandler([&Delivered](const FZLSocialIntentCommand& Command) { Delivered.Add(Command); });
	FZLSocialEvent Root;
	TestTrue(TEXT("Authoritative punch is produced"), Adapter.ProducePunch(TEXT("player"), Victim.AgentId, FVector::ZeroVector, 10.0, nullptr, &Root));
	const FZLSocialIntentCommand* WitnessReport = Delivered.FindByPredicate([&Witness](const FZLSocialIntentCommand& Command)
	{
		return Command.AgentId == Witness.AgentId && Command.Intent == ZLSocialTags::Intent_Report;
	});
	TestNotNull(TEXT("Witness produces report intent"), WitnessReport);

	FZLSocialReportConfirmation Confirmation;
	Confirmation.SourceEvent = Root;
	Confirmation.ReporterId = Witness.AgentId;
	Confirmation.ReceiverIds = {TEXT("important_guard_0")};
	Confirmation.CausationId = FGuid::NewGuid();
	Confirmation.ConfirmedAtSeconds = 10.5;
	Confirmation.bAnchorDerivedEvents = true;
	FZLSocialPropagationResult Propagation;
	TestTrue(TEXT("Gameplay explicitly confirms report completion"), Adapter.ConfirmReport(Confirmation, Propagation));
	TestEqual(TEXT("One social event reaches the guard"), Propagation.DerivedEvents.Num(), 1);
	const FZLSocialEvent& SocialEvent = Propagation.DerivedEvents[0];
	TestTrue(TEXT("Derived event preserves root"), SocialEvent.RootEventId == Root.EventId && SocialEvent.ParentEventId == Root.EventId);
	TestTrue(TEXT("Derived event has social source"), SocialEvent.ReporterId == Witness.AgentId && SocialEvent.HasChannel(EZLSocialPerceptionChannel::Social));

	const FZLSocialAgentState* GuardState = Adapter.GetSimulation().FindAgentState(TEXT("important_guard_0"));
	TestTrue(TEXT("Guard receives second-hand short memory"), GuardState != nullptr && GuardState->ShortMemory.Num() == 1);
	TestTrue(TEXT("Anchored report enters bounded long memory"), GuardState != nullptr && GuardState->LongMemory.Num() == 1);
	const TArray<FZLSocialMemoryEntry> GuardShortMemory = GuardState != nullptr ? GuardState->ShortMemory.GetChronological() : TArray<FZLSocialMemoryEntry>();
	TestTrue(TEXT("Guard memory retains social channel and confidence"), GuardShortMemory.Num() == 1 && GuardShortMemory[0].SourceChannel == EZLSocialPerceptionChannel::Social && FMath::IsNearlyEqual(GuardShortMemory[0].Confidence, SocialEvent.Confidence));

	const FZLSocialIntentCommand* GuardIntent = Delivered.FindByPredicate([](const FZLSocialIntentCommand& Command) { return Command.AgentId == TEXT("important_guard_0"); });
	TestTrue(TEXT("Guard receives deterministic local intent"), GuardIntent != nullptr && GuardIntent->Intent.IsValid());
	TestTrue(TEXT("Authority confirmation updates faction standing"), Adapter.ConfirmAuthorityAssessment(SocialEvent, TEXT("important_guard_0"), 10.5));
	const FZLSocialFactionStandingState* Standing = Adapter.GetSimulation().GetRelationshipStore().FindFactionStanding(TEXT("guards"), TEXT("player"));
	TestTrue(TEXT("Punch lowers guard faction standing"), Standing != nullptr && Standing->Standing < 0.0f);
	const float StandingValue = Standing != nullptr ? Standing->Standing : 0.0f;
	const FZLSocialRelationshipState* Relationship = Adapter.GetSimulation().GetRelationshipStore().FindRelationship(TEXT("important_guard_0"), TEXT("player"));
	TestNotNull(TEXT("Guard relationship edge is created"), Relationship);
	const float TrustValue = Relationship != nullptr ? Relationship->Trust : 0.0f;

	FZLSocialPropagationResult Replay;
	TestFalse(TEXT("Same reporter/root replay is rejected"), Adapter.ConfirmReport(Confirmation, Replay));
	TestFalse(TEXT("Same root cannot update faction twice"), Adapter.ConfirmAuthorityAssessment(SocialEvent, TEXT("important_guard_0"), 10.6));
	Standing = Adapter.GetSimulation().GetRelationshipStore().FindFactionStanding(TEXT("guards"), TEXT("player"));
	Relationship = Adapter.GetSimulation().GetRelationshipStore().FindRelationship(TEXT("important_guard_0"), TEXT("player"));
	TestTrue(TEXT("Replay leaves authority and relationship state unchanged"), Standing != nullptr && Relationship != nullptr && FMath::IsNearlyEqual(Standing->Standing, StandingValue) && FMath::IsNearlyEqual(Relationship->Trust, TrustValue));
	return true;
}

#endif
