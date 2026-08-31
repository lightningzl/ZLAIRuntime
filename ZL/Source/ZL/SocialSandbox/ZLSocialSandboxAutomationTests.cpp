#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "SocialSandbox/ZLSocialSandboxGameMode.h"
#include "SocialSandbox/ZLSocialSandboxMotion.h"
#include "SocialSandbox/ZLSocialSandboxNpc.h"
#include "SocialSandbox/ZLSocialSandboxPawn.h"
#include "SocialSandbox/ZLSocialSandboxPlayerController.h"
#include "ZLSocialObservation.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSandboxStageTest, "ZL.Social.Sandbox.StageConfiguration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialSandboxStageTest::RunTest(const FString&)
{
	TestTrue(TEXT("Dedicated sandbox map exists"), FPackageName::DoesPackageExist(TEXT("/Game/SocialSandbox/Lvl_SocialSandbox")));
	const AZLSocialSandboxGameMode* Defaults = GetDefault<AZLSocialSandboxGameMode>();
	TestTrue(TEXT("Sandbox uses controllable pawn"), Defaults->DefaultPawnClass == AZLSocialSandboxPawn::StaticClass());
	TestTrue(TEXT("Sandbox uses interaction controller"), Defaults->PlayerControllerClass == AZLSocialSandboxPlayerController::StaticClass());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSandboxMotionTest, "ZL.Social.Sandbox.BoundedMotion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialSandboxMotionTest::RunTest(const FString&)
{
	const FZLSocialSandboxMotionStep Approach = FZLSocialSandboxMotion::Compute(EZLSocialActionType::Approach, FVector::ZeroVector, FVector(1000.0f, 0.0f, 0.0f), 1.0f, 300.0f);
	TestFalse(TEXT("Approach is active outside desired distance"), Approach.bComplete);
	TestEqual(TEXT("Approach advances toward target"), Approach.Translation, FVector(300.0f, 0.0f, 0.0f));
	TestTrue(TEXT("Approach completes at bounded distance"), FZLSocialSandboxMotion::Compute(EZLSocialActionType::Approach, FVector(820.0f, 0.0f, 0.0f), FVector(1000.0f, 0.0f, 0.0f), 1.0f, 300.0f).bComplete);
	const FZLSocialSandboxMotionStep Away = FZLSocialSandboxMotion::Compute(EZLSocialActionType::MoveAway, FVector::ZeroVector, FVector(100.0f, 0.0f, 0.0f), 1.0f, 300.0f);
	TestEqual(TEXT("MoveAway advances opposite target"), Away.Translation, FVector(-300.0f, 0.0f, 0.0f));
	TestTrue(TEXT("MoveAway completes beyond bounded distance"), FZLSocialSandboxMotion::Compute(EZLSocialActionType::MoveAway, FVector(-550.0f, 0.0f, 0.0f), FVector(100.0f, 0.0f, 0.0f), 1.0f, 300.0f).bComplete);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSandboxPerNpcSliceTest, "ZL.Social.Sandbox.PerNpcVerticalSlice", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialSandboxPerNpcSliceTest::RunTest(const FString&)
{
	FZLSocialSpeechEvent Speech;
	Speech.EventId = FGuid::NewGuid();
	Speech.SpeakerId = TEXT("player");
	Speech.Text = TEXT("test payload is never copied into observations");
	Speech.Mode = EZLSocialSpeechMode::Talk;
	Speech.Position = FVector::ZeroVector;
	Speech.Forward = FVector::ForwardVector;
	Speech.TimestampSeconds = 1.0;
	const FZLSocialObservationEvaluator Evaluator;

	FZLSocialObserver Front;
	Front.AgentId = TEXT("npc_front");
	Front.Position = FVector(350.0f, 0.0f, 0.0f);
	Front.Forward = -FVector::ForwardVector;
	FZLSocialObserver Behind = Front;
	Behind.AgentId = TEXT("npc_behind");
	Behind.Position = FVector(600.0f, 0.0f, 0.0f);
	Behind.Forward = FVector::ForwardVector;
	FZLSocialObserver Distant = Front;
	Distant.AgentId = TEXT("npc_distant");
	Distant.Position = FVector(2000.0f, 0.0f, 0.0f);

	FZLSocialObservationBuffer FrontBuffer(2), BehindBuffer(2), DistantBuffer(2);
	FrontBuffer.Add(Evaluator.ObserveSpeech(Speech, Front, 1.1));
	BehindBuffer.Add(Evaluator.ObserveSpeech(Speech, Behind, 1.1));
	DistantBuffer.Add(Evaluator.ObserveSpeech(Speech, Distant, 1.1));
	TestTrue(TEXT("Front NPC sees and hears"), FrontBuffer.Latest()->bSaw && FrontBuffer.Latest()->bHeard);
	TestTrue(TEXT("Behind NPC hears without seeing"), !BehindBuffer.Latest()->bSaw && BehindBuffer.Latest()->bHeard);
	TestTrue(TEXT("Distant NPC neither sees nor hears"), !DistantBuffer.Latest()->bSaw && !DistantBuffer.Latest()->bHeard);
	TestEqual(TEXT("Each NPC stores only its own observation"), BehindBuffer.Latest()->ObserverId, FName(TEXT("npc_behind")));
	FrontBuffer.Reset();
	BehindBuffer.Reset();
	DistantBuffer.Reset();
	TestTrue(TEXT("Scene reset clears all bounded observations"), FrontBuffer.Latest() == nullptr && BehindBuffer.Latest() == nullptr && DistantBuffer.Latest() == nullptr);
	return true;
}

#endif
