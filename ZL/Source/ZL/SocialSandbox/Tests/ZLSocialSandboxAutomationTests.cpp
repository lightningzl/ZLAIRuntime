#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "SocialSandbox/World/ZLSocialSandboxGameMode.h"
#include "SocialSandbox/Domain/ZLSocialSandboxMotion.h"
#include "SocialSandbox/Domain/ZLSocialSandboxPreset.h"
#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPawn.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPlayerController.h"
#include "ZLSocialObservation.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSandboxStageTest, "ZL.Social.Sandbox.StageConfiguration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialSandboxStageTest::RunTest(const FString&)
{
	TestTrue(TEXT("Dedicated sandbox map exists"), FPackageName::DoesPackageExist(TEXT("/Game/SocialSandbox/Lvl_SocialSandbox")));
	const AZLSocialSandboxGameMode* Defaults = GetDefault<AZLSocialSandboxGameMode>();
	TestTrue(TEXT("Sandbox uses controllable pawn"), Defaults->DefaultPawnClass == AZLSocialSandboxPawn::StaticClass());
	TestTrue(TEXT("Sandbox uses interaction controller"), Defaults->PlayerControllerClass == AZLSocialSandboxPlayerController::StaticClass());
	const AZLSocialSandboxNpc* NpcDefaults = GetDefault<AZLSocialSandboxNpc>();
	TestTrue(TEXT("Sandbox NPC is a Character that AIController can possess"), AZLSocialSandboxNpc::StaticClass()->IsChildOf(ACharacter::StaticClass()));
	TestEqual(TEXT("Sandbox NPC auto-possesses AI when spawned"), NpcDefaults->AutoPossessAI, EAutoPossessAI::PlacedInWorldOrSpawned);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSandboxPresetTest, "ZL.Social.Sandbox.PresetValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialSandboxPresetTest::RunTest(const FString&)
{
	FZLSocialSandboxPreset MarketDay;
	FText Error;
	TestTrue(TEXT("Market preset loads from the controlled directory"), FZLSocialSandboxPresetCodec::LoadNamedPreset(TEXT("market_day"), MarketDay, Error));
	TestEqual(TEXT("Market preset has one player"), MarketDay.Player.StableId, FName(TEXT("player_market")));
	TestEqual(TEXT("Market preset has two NPCs"), MarketDay.Npcs.Num(), 2);
	TestEqual(TEXT("Market preset keeps NPC profile context"), MarketDay.Npcs[0].Profile.Role, FString(TEXT("market guard")));
	FZLSocialSandboxPreset NightWatch;
	TestTrue(TEXT("Second controlled preset loads"), FZLSocialSandboxPresetCodec::LoadNamedPreset(TEXT("night_watch"), NightWatch, Error));
	TestNotEqual(TEXT("Presets use independent player identities"), MarketDay.Player.StableId, NightWatch.Player.StableId);
	TestFalse(TEXT("Traversal-like preset names are rejected"), FZLSocialSandboxPresetCodec::LoadNamedPreset(TEXT("../market_day"), NightWatch, Error));
	FString ExportPath;
	TestTrue(TEXT("Validated preset exports only through Saved"), FZLSocialSandboxPresetCodec::ExportToSaved(MarketDay, ExportPath, Error));
	TestTrue(TEXT("Export path is outside the controlled import directory"), ExportPath.Contains(TEXT("Saved")));
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
	TestEqual(TEXT("Observation preserves the perceived source"), FrontBuffer.Latest()->SourceId, FName(TEXT("player")));
	TestTrue(TEXT("Behind NPC hears without seeing"), !BehindBuffer.Latest()->bSaw && BehindBuffer.Latest()->bHeard);
	TestTrue(TEXT("Distant NPC neither sees nor hears"), !DistantBuffer.Latest()->bSaw && !DistantBuffer.Latest()->bHeard);
	TestEqual(TEXT("Each NPC stores only its own observation"), BehindBuffer.Latest()->ObserverId, FName(TEXT("npc_behind")));
	FrontBuffer.Reset();
	BehindBuffer.Reset();
	DistantBuffer.Reset();
	TestTrue(TEXT("Scene reset clears all bounded observations"), FrontBuffer.Latest() == nullptr && BehindBuffer.Latest() == nullptr && DistantBuffer.Latest() == nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialSandboxNpcDecisionActionTest, "ZL.Social.Sandbox.NpcDecisionAction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialSandboxNpcDecisionActionTest::RunTest(const FString&)
{
	const FName WorldName = MakeUniqueObjectName(
		nullptr,
		UWorld::StaticClass(),
		NAME_None,
		EUniqueObjectNameOptions::GloballyUnique);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
	if (!TestNotNull(TEXT("A transient world is available"), World))
	{
		return false;
	}
	World->AddToRoot();
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AZLSocialSandboxNpc* Npc = World->SpawnActor<AZLSocialSandboxNpc>(FVector::ZeroVector, FRotator::ZeroRotator);
	AStaticMeshActor* Target = World->SpawnActor<AStaticMeshActor>(FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Decision NPC spawns"), Npc);
	TestNotNull(TEXT("Decision target spawns"), Target);
	if (Npc != nullptr && Target != nullptr)
	{
		Npc->InitializeSandboxNpc(TEXT("npc_guard"), FText::FromString(TEXT("Guard")), FTransform::Identity);
		Npc->SetActorEnableCollision(false);
		const int64 InitialVersion = Npc->GetStateVersion();
		TestFalse(TEXT("Face handler rejects a zero direction"), Npc->StartDecisionAction(EZLSocialActionType::Face, Npc, TFunction<void()>()));
		TestEqual(TEXT("Rejected Face has no authority state side effect"), Npc->GetStateVersion(), InitialVersion);
		int32 CompletionCount = 0;
		TestTrue(TEXT("Face handler accepts a valid target"), Npc->StartDecisionAction(EZLSocialActionType::Face, Target, [&CompletionCount]() { ++CompletionCount; }));
		TestTrue(TEXT("Face handler changes the NPC transform"), Npc->GetActorForwardVector().X > 0.99f);
		TestEqual(TEXT("Immediate Face completes exactly once"), CompletionCount, 1);
		TestTrue(TEXT("Accepted action advances authority version"), Npc->GetStateVersion() > InitialVersion);

		const FVector BeforeMove = Npc->GetActorLocation();
		TestTrue(TEXT("MoveAway handler starts"), Npc->StartDecisionAction(EZLSocialActionType::MoveAway, Target, [&CompletionCount]() { ++CompletionCount; }));
		Npc->AdvanceDecisionAction(1.0f);
		TestTrue(TEXT("MoveAway produces a real world transform change"), Npc->GetActorLocation().X < BeforeMove.X);
		Npc->StopDecisionAction();
		TestFalse(TEXT("Stop leaves no active NPC Decision action"), Npc->IsDecisionActionActive());

		Npc->SetDefending(true);
		const int64 BeforeDamageVersion = Npc->GetStateVersion();
		FZLSocialSandboxDamageResult Damage;
		TestTrue(TEXT("Valid damage is accepted"), Npc->ApplySandboxDamage(25.0f, 10.0, Damage));
		TestTrue(TEXT("Defense reduces authoritative damage"), Damage.bDefended && FMath::IsNearlyEqual(Damage.AppliedDamage, 9.0f));
		TestTrue(TEXT("Accepted damage advances authority version"), Npc->GetStateVersion() > BeforeDamageVersion);
		const float HealthAfterDamage = Npc->GetHealth();
		const int64 VersionAfterDamage = Npc->GetStateVersion();
		TestFalse(TEXT("Damage invulnerability rejects a repeated hit"), Npc->ApplySandboxDamage(25.0f, 10.1, Damage));
		TestEqual(TEXT("Rejected repeated hit preserves health"), Npc->GetHealth(), HealthAfterDamage);
		TestEqual(TEXT("Rejected repeated hit preserves authority version"), Npc->GetStateVersion(), VersionAfterDamage);
		Npc->SetDefending(false);
		TestTrue(TEXT("Lethal damage is accepted after invulnerability"), Npc->ApplySandboxDamage(200.0f, 11.0, Damage));
		TestTrue(TEXT("Lethal damage incapacitates the NPC"), Npc->IsIncapacitated() && FMath::IsNearlyZero(Npc->GetHealth()));
		const int64 IncapacitatedVersion = Npc->GetStateVersion();
		TestFalse(TEXT("Incapacitated NPC rejects more damage"), Npc->ApplySandboxDamage(25.0f, 12.0, Damage));
		TestEqual(TEXT("Rejected incapacitated damage preserves version"), Npc->GetStateVersion(), IncapacitatedVersion);
	}
	GEngine->ShutdownWorldNetDriver(World);
	World->DestroyWorld(true);
	World->SetPhysicsScene(nullptr);
	GEngine->DestroyWorldContext(World);
	World->RemoveFromRoot();
	return true;
}

#endif
