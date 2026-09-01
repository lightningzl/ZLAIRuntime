#include "SocialSandbox/ZLSocialSandboxGameMode.h"

#include "ZL.h"
#include "ZLSocialActionParser.h"
#include "ZLAIServiceSubsystem.h"
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
#include "SocialSandbox/ZLSocialSandboxDecisionContext.h"
#include "SocialSandbox/ZLSocialSandboxPawn.h"
#include "SocialSandbox/ZLSocialSandboxPlayerController.h"
#include "TimerManager.h"

AZLSocialSandboxGameMode::AZLSocialSandboxGameMode()
{
	DefaultPawnClass = AZLSocialSandboxPawn::StaticClass();
	PlayerControllerClass = AZLSocialSandboxPlayerController::StaticClass();
	ToolRegistry.RegisterMilestone8Defaults();
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
	const bool bStubSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDecisionSmoke"));
	const bool bFallbackSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDecisionFallbackSmoke"));
	const bool bKimiSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDecisionKimiSmoke"));
	bExpectStaleSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDecisionStaleSmoke"));
	if (bStubSmoke || bFallbackSmoke || bKimiSmoke || bExpectStaleSmoke)
	{
		ExpectedSmokeProvider = bFallbackSmoke ? TEXT("local") : (bKimiSmoke ? TEXT("kimi") : TEXT("stub"));
		if (const AZLSocialSandboxNpc* Guard = FindSandboxNpc(TEXT("npc_guard")))
		{
			SmokeInitialGuardLocation = Guard->GetActorLocation();
			SmokeInitialGuardVersion = Guard->GetStateVersion();
		}
		GetWorldTimerManager().SetTimer(DemoTimer, this, &AZLSocialSandboxGameMode::RunSocialSandboxDemo, 0.5f, false);
		GetWorldTimerManager().SetTimer(
			DecisionSmokeTimer,
			this,
			&AZLSocialSandboxGameMode::FinishDecisionSmokeTest,
			bFallbackSmoke ? 8.0f : (bKimiSmoke ? 25.0f : 4.0f),
			false);
	}
	else if (FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDemo")))
	{
		GetWorldTimerManager().SetTimer(DemoTimer, this, &AZLSocialSandboxGameMode::RunSocialSandboxDemo, 0.5f, false);
	}
}

void AZLSocialSandboxGameMode::ResetSocialSandbox()
{
	++GuardRequestGeneration;
	DecisionDebug = FZLSocialSandboxDecisionDebug();
	GuardExecutionTimes.Reset();
	ToolRegistry = FZLSocialToolRegistry();
	ToolRegistry.RegisterMilestone8Defaults();
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
		if (Npc == Target && Npc->GetStableId() == TEXT("npc_guard") && Observation.bHeard)
		{
			RequestGuardDecision(Npc, Observation, Event.Text);
		}
		else
		{
			Npc->ShowRuleSpeech(Observation);
		}
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
		const FZLSocialObservation Observation = Evaluator.ObserveAction(Event, Observer, NowSeconds);
		Npc->RecordObservation(Observation);
		Npc->ShowActionObservation(Observation);
		if (Phase == EZLSocialActionPhase::Completed
			&& Npc->GetStableId() == TEXT("npc_guard")
			&& TargetId == Npc->GetStableId()
			&& Observation.bSaw)
		{
			RequestGuardDecision(Npc, Observation, FString());
		}
	}
	const AZLSocialSandboxNpc* Target = TargetId.IsNone() ? nullptr : FindSandboxNpc(TargetId);
	Player->ShowActionBubble(Action, Phase, Target == nullptr ? FText::GetEmpty() : Target->GetDisplayName());
	RefreshInspector();
}

void AZLSocialSandboxGameMode::RequestGuardDecision(
	AZLSocialSandboxNpc* Guard,
	const FZLSocialObservation& Trigger,
	const FString& SpeechContent)
{
	if (!IsValid(Guard) || Guard->GetStableId() != TEXT("npc_guard"))
	{
		return;
	}
	if (DecisionDebug.bInFlight)
	{
		DecisionDebug.ToolResult = TEXT("InFlightLimit");
		RefreshInspector();
		return;
	}

	FZLSocialSandboxDecisionContextInput Input;
	Input.NpcId = Guard->GetStableId();
	Input.DisplayName = Guard->GetDisplayName();
	Input.TriggerObservation = Trigger;
	Input.TriggerSpeechContent = SpeechContent;
	Input.PersonalHistory = Guard->GetObservationItems();
	Input.StateVersion = Guard->GetStateVersion();
	FZLDecisionRequest Request;
	FString BuildError;
	if (!FZLSocialSandboxDecisionContextBuilder::Build(Input, Request, BuildError))
	{
		DecisionDebug.ToolResult = TEXT("ContextRejected");
		Guard->ShowDecisionFallback();
		RefreshInspector();
		return;
	}

	UZLAIServiceSubsystem* Service = GetGameInstance() == nullptr
		? nullptr
		: GetGameInstance()->GetSubsystem<UZLAIServiceSubsystem>();
	if (Service == nullptr)
	{
		DecisionDebug.ToolResult = TEXT("ServiceUnavailable");
		DecisionDebug.Provider = TEXT("local");
		Guard->ShowDecisionFallback();
		RefreshInspector();
		return;
	}

	DecisionDebug = FZLSocialSandboxDecisionDebug();
	DecisionDebug.RequestId = Request.RequestId;
	DecisionDebug.StateVersion = Request.StateVersion;
	DecisionDebug.bInFlight = true;
	const int32 RequestGeneration = GuardRequestGeneration;
	const double SentAtSeconds = FPlatformTime::Seconds();
	TWeakObjectPtr<AZLSocialSandboxNpc> WeakGuard(Guard);
	Service->SendDecisionRequest(
		MoveTemp(Request),
		FZLDecisionSuccessDelegate::CreateWeakLambda(this, [this, WeakGuard, RequestGeneration, SentAtSeconds](const FZLDecisionResponse& Response)
		{
			if (RequestGeneration == GuardRequestGeneration && WeakGuard.IsValid())
			{
				HandleGuardDecision(WeakGuard.Get(), Response, SentAtSeconds);
			}
		}),
		FZLDecisionFailureDelegate::CreateWeakLambda(this, [this, WeakGuard, RequestGeneration, SentAtSeconds](const FZLServiceError& Error)
		{
			if (RequestGeneration == GuardRequestGeneration && WeakGuard.IsValid())
			{
				HandleGuardDecisionFailure(WeakGuard.Get(), Error, SentAtSeconds);
			}
		}));
	if (bExpectStaleSmoke)
	{
		if (AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
		{
			Guard->StartDecisionAction(EZLSocialActionType::Face, Player, TFunction<void()>());
		}
	}
	RefreshInspector();
}

void AZLSocialSandboxGameMode::HandleGuardDecision(
	AZLSocialSandboxNpc* Guard,
	const FZLDecisionResponse& Response,
	const double SentAtSeconds)
{
	DecisionDebug.bInFlight = false;
	DecisionDebug.Provider = Response.Provider.Left(32);
	DecisionDebug.Intent = Response.Intent.Left(32);
	DecisionDebug.StateVersion = Response.StateVersion;
	DecisionDebug.LatencyMs = FMath::Clamp(
		FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0),
		0,
		60000);
	DecisionDebug.bSpeechAccepted = Response.bHasSpeech;
	Guard->ResetDecisionPresentation();
	if (Response.bHasSpeech)
	{
		Guard->ShowDecisionSpeech(Response.Speech.Text, Response.Provider);
	}
	if (Response.bHasToolCall)
	{
		ExecuteGuardTool(Guard, Response);
	}
	else
	{
		DecisionDebug.ToolResult = TEXT("NoTool");
	}
	RefreshInspector();
}

void AZLSocialSandboxGameMode::HandleGuardDecisionFailure(
	AZLSocialSandboxNpc* Guard,
	const FZLServiceError& Error,
	const double SentAtSeconds)
{
	DecisionDebug.bInFlight = false;
	DecisionDebug.Provider = TEXT("local");
	DecisionDebug.Intent = TEXT("hold");
	DecisionDebug.ToolName.Reset();
	DecisionDebug.ToolResult = Error.Code.IsEmpty() ? TEXT("DecisionUnavailable") : FName(*Error.Code.Left(64));
	DecisionDebug.LatencyMs = FMath::Clamp(
		FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0),
		0,
		60000);
	Guard->ShowDecisionFallback();
	RefreshInspector();
}

void AZLSocialSandboxGameMode::ExecuteGuardTool(AZLSocialSandboxNpc* Guard, const FZLDecisionResponse& Response)
{
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	const double NowSeconds = GetWorld()->GetTimeSeconds();
	GuardExecutionTimes.RemoveAll([NowSeconds](const double Value) { return NowSeconds - Value >= 10.0; });

	FZLSocialToolCall Call;
	Call.CallId = Response.ToolCall.CallId;
	Call.Name = FName(*Response.ToolCall.Name);
	Call.TargetId = Response.ToolCall.TargetId.IsEmpty() ? NAME_None : FName(*Response.ToolCall.TargetId);
	Call.StateVersion = Response.StateVersion;
	FZLSocialToolValidationContext Context;
	Context.CurrentStateVersion = Guard->GetStateVersion();
	Context.NowSeconds = NowSeconds;
	Context.DistanceToTarget = Player == nullptr ? TNumericLimits<float>::Max() : FVector::Dist2D(Guard->GetActorLocation(), Player->GetActorLocation());
	Context.bTargetValid = Player != nullptr && (Call.TargetId.IsNone() || Call.TargetId == TEXT("player"));
	Context.bNavigationReachable = Player != nullptr && FMath::Abs(Guard->GetActorLocation().Z - Player->GetActorLocation().Z) <= 200.0f;
	Context.bExecutable = IsValid(Guard) && GetWorld() != nullptr;
	Context.ExecutionsInWindow = GuardExecutionTimes.Num();
	Context.Capabilities = { TEXT("Tool.FaceTarget"), TEXT("Tool.MoveToward"), TEXT("Tool.MoveAway"), TEXT("Tool.Stop") };
	const FZLSocialToolValidationResult Validation = ToolRegistry.ValidateAndCommit(Call, Context);
	DecisionDebug.ToolName = Response.ToolCall.Name.Left(32);
	DecisionDebug.ToolResult = Validation.ReasonCode;
	if (!Validation.bAccepted)
	{
		if (!Response.bHasSpeech) { Guard->ShowDecisionRejection(Validation.ReasonCode); }
		return;
	}

	EZLSocialActionType Action = EZLSocialActionType::Stop;
	if (Call.Name == TEXT("face_target")) { Action = EZLSocialActionType::Face; }
	else if (Call.Name == TEXT("move_toward")) { Action = EZLSocialActionType::Approach; }
	else if (Call.Name == TEXT("move_away")) { Action = EZLSocialActionType::MoveAway; }
	const FName TargetId = Action == EZLSocialActionType::Stop ? NAME_None : FName(TEXT("player"));
	DispatchNpcActionObservation(Guard, Action, EZLSocialActionPhase::Started, TargetId);
	TWeakObjectPtr<AZLSocialSandboxGameMode> WeakThis(this);
	TWeakObjectPtr<AZLSocialSandboxNpc> WeakGuard(Guard);
	if (!Guard->StartDecisionAction(Action, Player, [WeakThis, WeakGuard, Action, TargetId]()
	{
		if (WeakThis.IsValid() && WeakGuard.IsValid())
		{
			WeakThis->DispatchNpcActionObservation(WeakGuard.Get(), Action, EZLSocialActionPhase::Completed, TargetId);
			WeakGuard->ShowDecisionAction(Action, EZLSocialActionPhase::Completed);
			WeakThis->RefreshInspector();
		}
	}))
	{
		DecisionDebug.ToolResult = TEXT("HandlerRejected");
		if (!Response.bHasSpeech) { Guard->ShowDecisionRejection(DecisionDebug.ToolResult); }
		return;
	}
	GuardExecutionTimes.Add(NowSeconds);
	while (GuardExecutionTimes.Num() > FZLSocialToolRegistry::MaxExecutionsPerWindow)
	{
		GuardExecutionTimes.RemoveAt(0, 1, EAllowShrinking::No);
	}
	DecisionDebug.ToolResult = ZLSocialToolReason::Accepted;
	if (Guard->IsDecisionActionActive())
	{
		Guard->ShowDecisionAction(Action, EZLSocialActionPhase::Started);
	}
}

void AZLSocialSandboxGameMode::DispatchNpcActionObservation(
	AZLSocialSandboxNpc* Actor,
	const EZLSocialActionType Action,
	const EZLSocialActionPhase Phase,
	const FName TargetId)
{
	if (!IsValid(Actor) || GetWorld() == nullptr) { return; }
	const double NowSeconds = GetWorld()->GetTimeSeconds();
	FZLSocialActionEvent Event;
	Event.EventId = FGuid::NewGuid();
	Event.Action = Action;
	Event.Phase = Phase;
	Event.ActorId = Actor->GetStableId();
	Event.TargetId = TargetId;
	Event.Position = Actor->GetActorLocation();
	Event.Forward = Actor->GetActorForwardVector();
	Event.TimestampSeconds = NowSeconds;
	if (!Event.IsValid(NowSeconds)) { return; }
	const FZLSocialObservationEvaluator Evaluator(ObservationSettings);
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (!IsValid(Npc) || Npc == Actor) { continue; }
		FZLSocialObserver Observer;
		Observer.AgentId = Npc->GetStableId();
		Observer.Position = Npc->GetActorLocation();
		Observer.Forward = Npc->GetPlanarForwardVector();
		Npc->RecordObservation(Evaluator.ObserveAction(Event, Observer, NowSeconds));
	}
}

void AZLSocialSandboxGameMode::FinishDecisionSmokeTest()
{
	const AZLSocialSandboxNpc* Guard = FindSandboxNpc(TEXT("npc_guard"));
	const bool bProviderMatches = DecisionDebug.Provider.Equals(ExpectedSmokeProvider, ESearchCase::IgnoreCase);
	const bool bFallback = ExpectedSmokeProvider == TEXT("local");
	const bool bKimi = ExpectedSmokeProvider == TEXT("kimi");
	const bool bHasTool = !DecisionDebug.ToolName.IsEmpty();
	const bool bResultMatches = bExpectStaleSmoke
		? DecisionDebug.ToolResult == ZLSocialToolReason::StateVersionMismatch
		: (bFallback
			? !DecisionDebug.ToolResult.IsNone()
			: (bKimi
				? (bHasTool
					? DecisionDebug.ToolResult == ZLSocialToolReason::Accepted
					: DecisionDebug.ToolResult == TEXT("NoTool"))
				: DecisionDebug.ToolResult == ZLSocialToolReason::Accepted));
	const bool bVersionChanged = Guard != nullptr && Guard->GetStateVersion() > SmokeInitialGuardVersion;
	const bool bLocationChanged = Guard != nullptr && !Guard->GetActorLocation().Equals(SmokeInitialGuardLocation, 0.1f);
	const bool bWorldChanged = Guard != nullptr
		&& bVersionChanged
		&& bLocationChanged;
	const bool bWorldUnchanged = Guard != nullptr
		&& Guard->GetStateVersion() == SmokeInitialGuardVersion
		&& Guard->GetActorLocation().Equals(SmokeInitialGuardLocation, 0.1f);
	const bool bStaleToolHadNoLocationSideEffect = bExpectStaleSmoke && bVersionChanged && !bLocationChanged;
	const bool bWorldOutcomeMatches = bExpectStaleSmoke
		? bStaleToolHadNoLocationSideEffect
		: (bFallback
			? bWorldUnchanged
			: (bKimi ? (!bHasTool || bVersionChanged) : bWorldChanged));
	const bool bOutputPresent = bKimi
		? (DecisionDebug.bSpeechAccepted || bHasTool)
		: (bFallback || DecisionDebug.bSpeechAccepted);
	const bool bPassed = Guard != nullptr
		&& !DecisionDebug.bInFlight
		&& bProviderMatches
		&& bResultMatches
		&& bWorldOutcomeMatches
		&& bOutputPresent;
	UE_LOG(
		LogZL,
		Display,
		TEXT("SocialSandboxDecisionSmoke result=%s provider=%s speech=%s tool=%s tool_result=%s version_changed=%s location_changed=%s state_unchanged=%s latency_ms=%d"),
		bPassed ? TEXT("Success") : TEXT("Fail"),
		DecisionDebug.Provider.IsEmpty() ? TEXT("None") : *DecisionDebug.Provider,
		DecisionDebug.bSpeechAccepted ? TEXT("Accepted") : TEXT("None"),
		DecisionDebug.ToolName.IsEmpty() ? TEXT("None") : *DecisionDebug.ToolName,
		DecisionDebug.ToolResult.IsNone() ? TEXT("None") : *DecisionDebug.ToolResult.ToString(),
		bVersionChanged ? TEXT("Yes") : TEXT("No"),
		bLocationChanged ? TEXT("Yes") : TEXT("No"),
		bWorldUnchanged ? TEXT("Yes") : TEXT("No"),
		DecisionDebug.LatencyMs);
	FPlatformMisc::RequestExitWithStatus(true, bPassed ? 0 : 1);
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
	const FString DecisionLine = NpcId == TEXT("npc_guard")
		? FString::Printf(
			TEXT("\nDecision: %s · Request: %s · State: %lld\nProvider: %s · Intent: %s · Speech: %s\nTool: %s · Result: %s · Latency: %d ms"),
			DecisionDebug.bInFlight ? TEXT("InFlight") : TEXT("Idle"),
			DecisionDebug.RequestId.IsEmpty() ? TEXT("None") : *DecisionDebug.RequestId,
			DecisionDebug.StateVersion,
			DecisionDebug.Provider.IsEmpty() ? TEXT("None") : *DecisionDebug.Provider,
			DecisionDebug.Intent.IsEmpty() ? TEXT("None") : *DecisionDebug.Intent,
			DecisionDebug.bSpeechAccepted ? TEXT("Accepted") : TEXT("None"),
			DecisionDebug.ToolName.IsEmpty() ? TEXT("None") : *DecisionDebug.ToolName,
			DecisionDebug.ToolResult.IsNone() ? TEXT("None") : *DecisionDebug.ToolResult.ToString(),
			DecisionDebug.LatencyMs)
		: FString();
	const TCHAR* FeedbackSource = NpcId == TEXT("npc_guard")
		? TEXT("DecisionSource: StructuredDecision")
		: TEXT("RuleSource: RulePlaceholder");
	return FText::FromString(FString::Printf(
		TEXT("%s [%s]\nSource: %s · Distance: %.0f cm\nSaw: %s · VisualFilter: %s\nHeard: %s · Clear: %s · Strength: %.2f\n%s\nInputSource: UE Event · %s%s"),
		*Npc->GetDisplayName().ToString(), *NpcId.ToString(), *Source, Observation->Distance,
		YesNo(Observation->bSaw), FilterText(Observation->VisualFilter), YesNo(Observation->bHeard), YesNo(Observation->bHeardClearly), Observation->HearingStrength,
		*SourceDetails, FeedbackSource, *DecisionLine));
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
