#include "Misc/AutomationTest.h"
#include "ZLSocialObservation.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FZLSocialSpeechEvent MakeSpeech(const EZLSocialSpeechMode Mode, const FVector& Position = FVector::ZeroVector, const FName Target = NAME_None)
{
	FZLSocialSpeechEvent Event;
	Event.EventId = FGuid::NewGuid();
	Event.SpeakerId = TEXT("player");
	Event.Text = TEXT("bounded test input");
	Event.Mode = Mode;
	Event.ExplicitTargetId = Target;
	Event.Position = Position;
	Event.Forward = FVector::ForwardVector;
	Event.TimestampSeconds = 10.0;
	return Event;
}

FZLSocialObserver MakeObserver(const FName Id, const FVector& Position, const FVector& Forward = -FVector::ForwardVector)
{
	FZLSocialObserver Observer;
	Observer.AgentId = Id;
	Observer.Position = Position;
	Observer.Forward = Forward;
	return Observer;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSpeechRangesTest, "ZL.Social.Observation.SpeechRangesAndTargets", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialSpeechRangesTest::RunTest(const FString&)
{
	const FZLSocialObservationEvaluator Evaluator;
	TestTrue(TEXT("Whisper includes 200 cm boundary"), Evaluator.ObserveSpeech(MakeSpeech(EZLSocialSpeechMode::Whisper), MakeObserver(TEXT("near"), FVector(200.0f, 0.0f, 0.0f)), 10.1).bHeard);
	TestFalse(TEXT("Whisper excludes beyond boundary"), Evaluator.ObserveSpeech(MakeSpeech(EZLSocialSpeechMode::Whisper), MakeObserver(TEXT("far"), FVector(201.0f, 0.0f, 0.0f)), 10.1).bHeard);
	TestTrue(TEXT("Talk reaches 800 cm"), Evaluator.ObserveSpeech(MakeSpeech(EZLSocialSpeechMode::Talk), MakeObserver(TEXT("talk"), FVector(800.0f, 0.0f, 0.0f)), 10.1).bHeard);
	TestTrue(TEXT("Shout reaches 2500 cm"), Evaluator.ObserveSpeech(MakeSpeech(EZLSocialSpeechMode::Shout), MakeObserver(TEXT("shout"), FVector(2500.0f, 0.0f, 0.0f)), 10.1).bHeard);

	const FZLSocialSpeechEvent Directed = MakeSpeech(EZLSocialSpeechMode::InEar, FVector::ZeroVector, TEXT("target"));
	TestTrue(TEXT("InEar target hears in range"), Evaluator.ObserveSpeech(Directed, MakeObserver(TEXT("target"), FVector(150.0f, 0.0f, 0.0f)), 10.1).bHeard);
	const FZLSocialObservation Bystander = Evaluator.ObserveSpeech(Directed, MakeObserver(TEXT("bystander"), FVector(20.0f, 0.0f, 0.0f)), 10.1);
	TestFalse(TEXT("InEar bystander never hears"), Bystander.bHeard);
	TestEqual(TEXT("InEar filter is explicit"), Bystander.AuditoryFilter, EZLSocialObservationFilterReason::NotExplicitInEarTarget);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialDirectionalVisionTest, "ZL.Social.Observation.DirectionalVision", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialDirectionalVisionTest::RunTest(const FString&)
{
	const FZLSocialObservationEvaluator Evaluator;
	FZLSocialActionEvent Event;
	Event.EventId = FGuid::NewGuid();
	Event.ActorId = TEXT("player");
	Event.Action = EZLSocialActionType::Stop;
	Event.Position = FVector(1000.0f, 0.0f, 0.0f);
	Event.Forward = FVector::ForwardVector;
	Event.TimestampSeconds = 1.0;

	TestTrue(TEXT("Observer sees action inside 120 degree cone"), Evaluator.ObserveAction(Event, MakeObserver(TEXT("front"), FVector::ZeroVector, FVector::ForwardVector), 1.1).bSaw);
	const FZLSocialObservation Behind = Evaluator.ObserveAction(Event, MakeObserver(TEXT("back"), FVector::ZeroVector, -FVector::ForwardVector), 1.1);
	TestFalse(TEXT("Observer rejects action behind"), Behind.bSaw);
	TestEqual(TEXT("Direction reason is explicit"), Behind.VisualFilter, EZLSocialObservationFilterReason::OutsideFieldOfView);
	TestFalse(TEXT("Observer rejects outside visual distance"), Evaluator.ObserveAction(Event, MakeObserver(TEXT("distant"), FVector(-600.0f, 0.0f, 0.0f), FVector::ForwardVector), 1.1).bSaw);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialObservationBoundsTest, "ZL.Social.Observation.BoundsAndIsolation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialObservationBoundsTest::RunTest(const FString&)
{
	FZLSocialObservationBuffer Buffer(2);
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FZLSocialObservation Observation;
		Observation.EventId = FGuid::NewGuid();
		Observation.ObserverId = FName(*FString::Printf(TEXT("npc_%d"), Index));
		Buffer.Add(Observation);
	}
	TestEqual(TEXT("Observation buffer remains bounded"), Buffer.GetItems().Num(), 2);
	TestEqual(TEXT("Latest belongs only to added observer"), Buffer.Latest()->ObserverId, FName(TEXT("npc_2")));

	FZLSocialSpeechEvent Invalid = MakeSpeech(EZLSocialSpeechMode::InEar);
	TestFalse(TEXT("InEar without target is invalid"), Invalid.IsValid(10.1));
	FZLSocialActionEvent Action;
	Action.EventId = FGuid::NewGuid();
	Action.ActorId = TEXT("player");
	Action.Action = EZLSocialActionType::Approach;
	Action.Forward = FVector::ForwardVector;
	Action.TimestampSeconds = 10.0;
	TestFalse(TEXT("Targeted action without target is invalid"), Action.IsValid(10.1));
	return true;
}

#endif
