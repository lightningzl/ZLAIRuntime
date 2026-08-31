#include "SocialSandbox/ZLSocialSandboxGameMode.h"

#include "ZLSocialActionParser.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "SocialSandbox/ZLSocialSandboxNpc.h"
#include "SocialSandbox/ZLSocialSandboxPawn.h"
#include "SocialSandbox/ZLSocialSandboxPlayerController.h"
#include "TimerManager.h"

AZLSocialSandboxGameMode::AZLSocialSandboxGameMode()
{
	DefaultPawnClass = AZLSocialSandboxPawn::StaticClass();
	PlayerControllerClass = AZLSocialSandboxPlayerController::StaticClass();
}

void AZLSocialSandboxGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnEnvironment();
	SpawnNpc(TEXT("npc_guard"), TEXT("守卫 Guard"), FVector(500.0f, -350.0f, 96.0f), FRotator(0.0f, 145.0f, 0.0f));
	SpawnNpc(TEXT("npc_merchant"), TEXT("商人 Merchant"), FVector(500.0f, 350.0f, 96.0f), FRotator(0.0f, 215.0f, 0.0f));
	SpawnNpc(TEXT("npc_scout"), TEXT("斥候 Scout"), FVector(950.0f, -350.0f, 96.0f), FRotator(0.0f, 160.0f, 0.0f));
	SpawnNpc(TEXT("npc_civilian"), TEXT("居民 Civilian"), FVector(950.0f, 350.0f, 96.0f), FRotator(0.0f, 200.0f, 0.0f));
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->RefreshSandboxTargets();
	}
	if (FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDemo")))
	{
		GetWorldTimerManager().SetTimer(DemoTimer, this, &AZLSocialSandboxGameMode::RunSocialSandboxDemo, 0.5f, false);
	}
}

void AZLSocialSandboxGameMode::ResetSocialSandbox()
{
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (IsValid(Npc))
		{
			Npc->ResetToSandboxStart();
		}
	}
	if (AZLSocialSandboxPawn* Pawn = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Pawn->ResetToSandboxStart();
	}
	RefreshInspector();
}

void AZLSocialSandboxGameMode::RunSocialSandboxDemo()
{
	const FName GuardId(TEXT("npc_guard"));
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->SelectInspectorTarget(GuardId);
	}
	SubmitSpeech(TEXT("Talk"), GuardId, TEXT("Social sandbox demo"));
}

AZLSocialSandboxNpc* AZLSocialSandboxGameMode::FindSandboxNpc(const FName StableId) const
{
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (IsValid(Npc) && Npc->GetStableId() == StableId)
		{
			return Npc;
		}
	}
	return nullptr;
}

FText AZLSocialSandboxGameMode::SubmitSpeech(const FName SpeechMode, const FName TargetId, const FString& Text)
{
	const TMap<FName, EZLSocialSpeechMode> Modes = {
		{ TEXT("Whisper"), EZLSocialSpeechMode::Whisper },
		{ TEXT("Talk"), EZLSocialSpeechMode::Talk },
		{ TEXT("Shout"), EZLSocialSpeechMode::Shout },
		{ TEXT("InEar"), EZLSocialSpeechMode::InEar }
	};
	const EZLSocialSpeechMode* Mode = Modes.Find(SpeechMode);
	if (Mode == nullptr)
	{
		return FText::FromString(TEXT("拒绝：未知说话模式"));
	}
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player == nullptr)
	{
		return FText::FromString(TEXT("拒绝：玩家角色不可用"));
	}
	AZLSocialSandboxNpc* Target = TargetId.IsNone() ? nullptr : FindSandboxNpc(TargetId);
	if (!TargetId.IsNone() && Target == nullptr)
	{
		return FText::FromString(TEXT("拒绝：目标已失效"));
	}
	FZLSocialObservationSettings Settings = ObservationSettings;
	Settings.Clamp();
	if (*Mode == EZLSocialSpeechMode::InEar && (Target == nullptr || FVector::Dist2D(Player->GetActorLocation(), Target->GetActorLocation()) > Settings.InEarRange))
	{
		return FText::FromString(TEXT("拒绝：耳边说话目标必须在近距离内"));
	}

	FZLSocialSpeechEvent Event;
	Event.EventId = FGuid::NewGuid();
	Event.SpeakerId = TEXT("player");
	Event.Text = Text;
	Event.Mode = *Mode;
	Event.ExplicitTargetId = TargetId;
	Event.Position = Player->GetActorLocation();
	Event.Forward = Player->GetActorForwardVector();
	Event.TimestampSeconds = GetWorld()->GetTimeSeconds();
	Event.LifetimeSeconds = 4.0f;
	if (!Event.IsValid(Event.TimestampSeconds))
	{
		return FText::FromString(TEXT("拒绝：说话事件边界无效"));
	}

	const FZLSocialObservationEvaluator Evaluator(Settings);
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (!IsValid(Npc))
		{
			continue;
		}
		FZLSocialObserver Observer;
		Observer.AgentId = Npc->GetStableId();
		Observer.Position = Npc->GetActorLocation();
		Observer.Forward = Npc->GetPlanarForwardVector();
		const FZLSocialObservation Observation = Evaluator.ObserveSpeech(Event, Observer, Event.TimestampSeconds);
		Npc->RecordObservation(Observation);
		Npc->ShowRuleSpeech(Observation);
	}
	Player->ShowSpeechBubble(Text);
	RefreshInspector();
	return FText::GetEmpty();
}

FText AZLSocialSandboxGameMode::SubmitAction(const FName TargetId, const FString& Text)
{
	const FZLSocialActionParseResult Parsed = FZLSocialActionParser::Parse(Text);
	if (!Parsed.bMatched)
	{
		return FText::FromString(TEXT("拒绝：仅支持 Face、Approach、MoveAway 和 Stop 的受控别名"));
	}
	const bool bNeedsTarget = Parsed.Action != EZLSocialActionType::Stop;
	AZLSocialSandboxNpc* Target = TargetId.IsNone() ? nullptr : FindSandboxNpc(TargetId);
	if (bNeedsTarget && Target == nullptr)
	{
		return FText::FromString(TEXT("拒绝：该行为必须选择有效目标"));
	}
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player == nullptr)
	{
		return FText::FromString(TEXT("拒绝：玩家角色不可用"));
	}
	if (Target != nullptr && FVector::Dist2D(Player->GetActorLocation(), Target->GetActorLocation()) > 3000.0f)
	{
		return FText::FromString(TEXT("拒绝：目标超出行为执行范围"));
	}

	if (Parsed.Action == EZLSocialActionType::Face)
	{
		FVector Direction = Target->GetActorLocation() - Player->GetActorLocation();
		Direction.Z = 0.0f;
		if (Direction.IsNearlyZero())
		{
			return FText::FromString(TEXT("拒绝：目标方向无效"));
		}
		DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Started, TargetId);
		Player->SetActorRotation(Direction.Rotation());
		if (Player->GetController() != nullptr) { Player->GetController()->SetControlRotation(Direction.Rotation()); }
		DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Completed, TargetId);
		return FText::GetEmpty();
	}
	if (Parsed.Action == EZLSocialActionType::Stop)
	{
		DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Started, NAME_None);
		Player->StopScriptedAction();
		DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Completed, NAME_None);
		return FText::GetEmpty();
	}

	TWeakObjectPtr<AZLSocialSandboxGameMode> WeakThis(this);
	if (!Player->StartScriptedAction(Parsed.Action, Target, [WeakThis, Action = Parsed.Action, TargetId]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->DispatchActionObservation(Action, EZLSocialActionPhase::Completed, TargetId);
		}
	}))
	{
		return FText::FromString(TEXT("拒绝：玩家当前无法执行该行为"));
	}
	DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Started, TargetId);
	return FText::GetEmpty();
}

void AZLSocialSandboxGameMode::DispatchActionObservation(const EZLSocialActionType Action, const EZLSocialActionPhase Phase, const FName TargetId)
{
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player == nullptr)
	{
		return;
	}
	const double NowSeconds = GetWorld()->GetTimeSeconds();
	FZLSocialActionEvent Event;
	Event.EventId = FGuid::NewGuid();
	Event.Action = Action;
	Event.Phase = Phase;
	Event.ActorId = TEXT("player");
	Event.TargetId = TargetId;
	Event.Position = Player->GetActorLocation();
	Event.Forward = Player->GetActorForwardVector();
	Event.TimestampSeconds = NowSeconds;
	if (!Event.IsValid(NowSeconds))
	{
		return;
	}
	const FZLSocialObservationEvaluator Evaluator(ObservationSettings);
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (!IsValid(Npc)) { continue; }
		FZLSocialObserver Observer;
		Observer.AgentId = Npc->GetStableId();
		Observer.Position = Npc->GetActorLocation();
		Observer.Forward = Npc->GetPlanarForwardVector();
		Npc->RecordObservation(Evaluator.ObserveAction(Event, Observer, NowSeconds));
		Npc->ShowActionObservation(*Npc->GetLatestObservation());
	}
	const AZLSocialSandboxNpc* Target = TargetId.IsNone() ? nullptr : FindSandboxNpc(TargetId);
	Player->ShowActionBubble(Action, Phase, Target == nullptr ? FText::GetEmpty() : Target->GetDisplayName());
	RefreshInspector();
}

FText AZLSocialSandboxGameMode::BuildInspectorText(const FName NpcId) const
{
	const AZLSocialSandboxNpc* Npc = FindSandboxNpc(NpcId);
	if (Npc == nullptr)
	{
		return FText::FromString(TEXT("选择一个 NPC 查看个人感知。"));
	}
	const FZLSocialObservation* Observation = Npc->GetLatestObservation();
	if (Observation == nullptr)
	{
		return FText::FromString(FString::Printf(TEXT("%s [%s]\n尚无 Observation"), *Npc->GetDisplayName().ToString(), *NpcId.ToString()));
	}
	auto YesNo = [](const bool Value) { return Value ? TEXT("是 Yes") : TEXT("否 No"); };
	auto SpeechModeText = [](const EZLSocialSpeechMode Mode)
	{
		switch (Mode) { case EZLSocialSpeechMode::Whisper: return TEXT("Whisper"); case EZLSocialSpeechMode::Shout: return TEXT("Shout"); case EZLSocialSpeechMode::InEar: return TEXT("InEar"); default: return TEXT("Talk"); }
	};
	auto TargetText = [](const EZLSocialTargetJudgment Value)
	{
		switch (Value) { case EZLSocialTargetJudgment::Candidate: return TEXT("Candidate"); case EZLSocialTargetJudgment::ExplicitSelf: return TEXT("ExplicitSelf"); case EZLSocialTargetJudgment::ExplicitOther: return TEXT("ExplicitOther"); default: return TEXT("Unresolved"); }
	};
	auto FilterText = [](const EZLSocialObservationFilterReason Value)
	{
		switch (Value) { case EZLSocialObservationFilterReason::InvalidEvent: return TEXT("InvalidEvent"); case EZLSocialObservationFilterReason::Expired: return TEXT("Expired"); case EZLSocialObservationFilterReason::CannotSee: return TEXT("CannotSee"); case EZLSocialObservationFilterReason::CannotHear: return TEXT("CannotHear"); case EZLSocialObservationFilterReason::OutsideVisualRange: return TEXT("OutsideVisualRange"); case EZLSocialObservationFilterReason::OutsideFieldOfView: return TEXT("OutsideFieldOfView"); case EZLSocialObservationFilterReason::OutsideHearingRange: return TEXT("OutsideHearingRange"); case EZLSocialObservationFilterReason::NotExplicitInEarTarget: return TEXT("NotExplicitInEarTarget"); default: return TEXT("None"); }
	};
	const FString Source = Observation->Source == EZLSocialObservationSource::Speech ? TEXT("Speech") : TEXT("Action");
	auto ActionText = [](const EZLSocialActionType Value)
	{
		switch (Value) { case EZLSocialActionType::Face: return TEXT("Face"); case EZLSocialActionType::Approach: return TEXT("Approach"); case EZLSocialActionType::MoveAway: return TEXT("MoveAway"); default: return TEXT("Stop"); }
	};
	const FString SourceDetails = Observation->Source == EZLSocialObservationSource::Speech
		? FString::Printf(TEXT("SpeechMode: %s · ExplicitTarget: %s\nTargetJudgment: %s · AuditoryFilter: %s"), SpeechModeText(Observation->SpeechMode), Observation->ExplicitTargetId.IsNone() ? TEXT("None") : *Observation->ExplicitTargetId.ToString(), TargetText(Observation->TargetJudgment), FilterText(Observation->AuditoryFilter))
		: FString::Printf(TEXT("Action: %s · Phase: %s · Target: %s\nTargetJudgment: %s · InputTextAvailable: No"), ActionText(Observation->Action), Observation->ActionPhase == EZLSocialActionPhase::Started ? TEXT("Started") : TEXT("Completed"), Observation->ExplicitTargetId.IsNone() ? TEXT("None") : *Observation->ExplicitTargetId.ToString(), TargetText(Observation->TargetJudgment));
	return FText::FromString(FString::Printf(
		TEXT("%s [%s]\nSource: %s · Distance: %.0f cm\nSaw: %s · VisualFilter: %s\nHeard: %s · Clear: %s · Strength: %.2f\n%s\nInputSource: UE Event · RuleSource: RulePlaceholder"),
		*Npc->GetDisplayName().ToString(), *NpcId.ToString(), *Source, Observation->Distance,
		YesNo(Observation->bSaw), FilterText(Observation->VisualFilter), YesNo(Observation->bHeard), YesNo(Observation->bHeardClearly), Observation->HearingStrength,
		*SourceDetails));
}

void AZLSocialSandboxGameMode::RefreshInspector() const
{
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->RefreshObservationInspector();
	}
}

void AZLSocialSandboxGameMode::SpawnEnvironment()
{
	if (GetWorld() == nullptr)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Name = TEXT("SocialSandboxFloor");
	AStaticMeshActor* Floor = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0.0f, 0.0f, -55.0f), FRotator::ZeroRotator, Params);
	if (Floor != nullptr)
	{
		Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
		Floor->SetActorScale3D(FVector(35.0f, 35.0f, 1.0f));
		Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
		if (UMaterialInstanceDynamic* Material = Floor->GetStaticMeshComponent()->CreateDynamicMaterialInstance(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.025f, 0.045f, 0.075f));
		}
	}

	if (!GetWorld()->GetAuthGameMode()->FindPlayerStart(nullptr))
	{
		GetWorld()->SpawnActor<APlayerStart>(FVector::ZeroVector, FRotator::ZeroRotator);
	}

	ADirectionalLight* Light = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-50.0f, -35.0f, 0.0f));
	if (Light != nullptr)
	{
		Light->SetMobility(EComponentMobility::Movable);
		Light->GetLightComponent()->SetIntensity(2.0f);
		Light->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.92f, 0.78f));
	}
	APointLight* FillLight = GetWorld()->SpawnActor<APointLight>(FVector(500.0f, 0.0f, 1100.0f), FRotator::ZeroRotator);
	if (FillLight != nullptr)
	{
		FillLight->SetMobility(EComponentMobility::Movable);
		if (UPointLightComponent* Component = Cast<UPointLightComponent>(FillLight->GetLightComponent()))
		{
			Component->SetIntensity(65000.0f);
			Component->SetAttenuationRadius(4500.0f);
			Component->SetLightColor(FLinearColor(0.55f, 0.68f, 1.0f));
		}
	}
}

void AZLSocialSandboxGameMode::SpawnNpc(const FName StableId, const TCHAR* DisplayName, const FVector& Location, const FRotator& Rotation)
{
	FActorSpawnParameters Params;
	Params.Name = StableId;
	AZLSocialSandboxNpc* Npc = GetWorld()->SpawnActor<AZLSocialSandboxNpc>(AZLSocialSandboxNpc::StaticClass(), Location, Rotation, Params);
	if (Npc != nullptr)
	{
		Npc->InitializeSandboxNpc(StableId, FText::FromString(DisplayName), FTransform(Rotation, Location));
		SandboxNpcs.Add(Npc);
	}
}
