#include "Misc/AutomationTest.h"
#include "ZLSocialState.h"
#include "ZLSocialTags.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialStateTest, "ZL.Social.State.BoundsDecayAndMemory", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialStateTest::RunTest(const FString& Parameters)
{
	FZLSocialPersonalityTraits Personality;
	Personality.FearSensitivity = 1.0f;
	Personality.Curiosity = 0.4f;
	Personality.Justice = 0.6f;
	Personality.Aggression = 0.3f;

	FZLSocialAgentState State(3);
	FZLSocialPerceptionResult Perception;
	Perception.bPerceived = true;
	Perception.Channel = EZLSocialPerceptionChannel::Visual;
	Perception.EffectiveIntensity = 1.0f;

	FGuid FirstEventId;
	for (int32 Index = 0; Index < 5; ++Index)
	{
		FZLSocialEvent Event;
		Event.EventId = FGuid::NewGuid();
		if (Index == 0) { FirstEventId = Event.EventId; }
		Event.Type = ZLSocialTags::Event_Gunshot;
		Event.Severity = 1.0f;
		Event.CreatedAtSeconds = Index;
		State.ApplyPerception(Event, Perception, Personality);
	}

	TestTrue(TEXT("Fear is bounded"), State.Instant.Fear <= 1.0f);
	TestTrue(TEXT("Alert is bounded"), State.Instant.Alert <= 1.0f);
	TestEqual(TEXT("Memory stays at fixed capacity"), State.ShortMemory.Num(), 3);
	TestNull(TEXT("Oldest entry is evicted"), State.ShortMemory.FindByEvent(FirstEventId));
	const TArray<FZLSocialMemoryEntry> Memory = State.ShortMemory.GetChronological();
	TestEqual(TEXT("Memory remains chronological"), Memory[0].TimestampSeconds, 2.0);

	State.Decay(20.0f);
	TestEqual(TEXT("Fear decays to zero without underflow"), State.Instant.Fear, 0.0f);
	TestEqual(TEXT("Alert decays to zero without underflow"), State.Instant.Alert, 0.0f);
	return true;
}

#endif
