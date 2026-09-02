#include "SocialSandbox/World/ZLSocialSandboxGameMode.h"

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
#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/Decision/ZLSocialSandboxDecisionContext.h"
#include "SocialSandbox/Domain/ZLSocialSandboxCombat.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPawn.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPlayerController.h"
#include "TimerManager.h"

AZLSocialSandboxGameMode::AZLSocialSandboxGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AZLSocialSandboxPawn::StaticClass();
	PlayerControllerClass = AZLSocialSandboxPlayerController::StaticClass();
	ToolRegistry.RegisterMilestone8Defaults();
}

void AZLSocialSandboxGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnEnvironment();
	SpawnNpc(TEXT("npc_guard"), FVector(500.0f, -350.0f, 96.0f), FRotator(0.0f, 145.0f, 0.0f));
	SpawnNpc(TEXT("npc_merchant"), FVector(500.0f, 350.0f, 96.0f), FRotator(0.0f, 215.0f, 0.0f));
	SpawnNpc(TEXT("npc_rival"), FVector(950.0f, -350.0f, 96.0f), FRotator(0.0f, 160.0f, 0.0f));
	SpawnNpc(TEXT("npc_civilian"), FVector(950.0f, 350.0f, 96.0f), FRotator(0.0f, 200.0f, 0.0f));
	UpdateGuardDistanceBand();
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->RefreshSandboxTargets();
	}
	const bool bStubSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDecisionSmoke"));
	const bool bFallbackSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDecisionFallbackSmoke"));
	const bool bKimiSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDecisionKimiSmoke"));
	const bool bMultiNpcSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxMultiNpcSmoke"));
	bExpectStaleSmoke = FParse::Param(FCommandLine::Get(), TEXT("ZLSandboxDecisionStaleSmoke"));
	bSmokeSawAcceptedTool = false;
	if (bMultiNpcSmoke)
	{
		GetWorldTimerManager().SetTimer(DemoTimer, this, &AZLSocialSandboxGameMode::RunMultiNpcSandboxDemo, 0.5f, false);
		GetWorldTimerManager().SetTimer(MultiNpcSmokeTimer, this, &AZLSocialSandboxGameMode::FinishMultiNpcSmokeTest, 8.0f, false);
	}
	else if (bStubSmoke || bFallbackSmoke || bKimiSmoke || bExpectStaleSmoke)
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
	GetWorldTimerManager().ClearTimer(GuardDecisionCooldownTimer);
	GetWorldTimerManager().ClearTimer(NpcDecisionCooldownTimer);
	GuardDecisionScheduler.Reset();
	MultiNpcDecision.Reset();
	DecisionDebug = FZLSocialSandboxDecisionDebug();
	NpcDecisionDebug.Reset();
	NpcPublicHistory.Reset();
	NpcExecutionTimes.Reset();
	NpcConflictStates.Reset();
	NpcDistanceBands.Reset();
	NpcLastDistances.Reset();
	GuardPublicHistory.Reset();
	GuardDistanceBand = INDEX_NONE;
	LastGuardDistance = TNumericLimits<float>::Max();
	LastPlayerAttackSeconds = -DBL_MAX;
	GuardConflictState.Reset();
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
	UpdateGuardDistanceBand();
	UpdateNpcDistanceBands();
	RefreshInspector();
}

void AZLSocialSandboxGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateGuardDistanceBand();
	UpdateNpcDistanceBands();
}

void AZLSocialSandboxGameMode::RunSocialSandboxDemo()
{
	const FName GuardId(TEXT("npc_guard"));
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->SelectInspectorTarget(GuardId);
	}
	SubmitSpeech(TEXT("Talk"), GuardId, TEXT("社会交互舞台演示"));
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
		if (Observation.bHeard
			&& (Observation.bHeardClearly
				|| Observation.TargetJudgment == EZLSocialTargetJudgment::ExplicitSelf))
		{
			QueueNpcDecision(
				Npc,
				Observation,
				Event.Text,
				EZLSocialSandboxDecisionTriggerReason::Speech);
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
		return FText::FromString(TEXT("拒绝：仅支持面向、靠近、远离、攻击和停止等受控行为"));
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
	if (Parsed.Action == EZLSocialActionType::Attack)
	{
		const float AttackDistance = FVector::Dist2D(Player->GetActorLocation(), Target->GetActorLocation());
		const double NowSeconds = GetWorld()->GetTimeSeconds();
		FZLSocialSandboxAttackValidationContext AttackContext;
		AttackContext.Distance = AttackDistance;
		AttackContext.NowSeconds = NowSeconds;
		AttackContext.LastAttackSeconds = LastPlayerAttackSeconds;
		AttackContext.bTargetValid = IsValid(Target);
		AttackContext.bPlayerExecutable = !Player->IsScriptedActionActive();
		AttackContext.bTargetIncapacitated = Target->IsIncapacitated();
		const FZLSocialSandboxAttackValidationResult Validation = FZLSocialSandboxCombat::ValidatePlayerAttack(AttackContext);
		if (!Validation.bAccepted)
		{
			return FText::FromString(TEXT("拒绝：攻击条件不满足"));
		}
		FZLSocialSandboxDamageResult DamageResult;
		if (!Target->ApplySandboxDamage(FZLSocialSandboxCombat::AttackDamage, NowSeconds, DamageResult))
		{
			Target->ShowDamageResult(DamageResult);
			return FText::FromString(TEXT("拒绝：目标当前无法受到攻击"));
		}
		LastPlayerAttackSeconds = NowSeconds;
		ApplyGuardConflict(Target, EZLSocialSandboxConflictEvent::Attack);
		FVector Direction = Target->GetActorLocation() - Player->GetActorLocation();
		Direction.Z = 0.0f;
		if (!Direction.IsNearlyZero())
		{
			Player->SetActorRotation(Direction.Rotation());
			if (Player->GetController() != nullptr) { Player->GetController()->SetControlRotation(Direction.Rotation()); }
		}
		DispatchActionObservation(EZLSocialActionType::Attack, EZLSocialActionPhase::Started, TargetId);
		DispatchActionObservation(EZLSocialActionType::Attack, EZLSocialActionPhase::Completed, TargetId);
		Target->ShowDamageResult(DamageResult);
		return FText::GetEmpty();
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
		ApplyGuardConflict(FindSandboxNpc(TEXT("npc_guard")), EZLSocialSandboxConflictEvent::PlayerStop);
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
		if (Phase == EZLSocialActionPhase::Completed && Observation.bSaw)
		{
			QueueNpcDecision(
				Npc,
				Observation,
				FString(),
				Action == EZLSocialActionType::Attack
					? EZLSocialSandboxDecisionTriggerReason::Hit
					: EZLSocialSandboxDecisionTriggerReason::PlayerAction,
				Action != EZLSocialActionType::Attack);
		}
	}
	const AZLSocialSandboxNpc* Target = TargetId.IsNone() ? nullptr : FindSandboxNpc(TargetId);
	Player->ShowActionBubble(Action, Phase, Target == nullptr ? FText::GetEmpty() : Target->GetDisplayName());
	RefreshInspector();
}

void AZLSocialSandboxGameMode::RunMultiNpcSandboxDemo()
{
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->SelectInspectorTarget(TEXT("npc_rival"));
	}
	SubmitSpeech(TEXT("Shout"), NAME_None, TEXT("大家都说说自己的立场。"));
}

void AZLSocialSandboxGameMode::QueueNpcDecision(
	AZLSocialSandboxNpc* Npc,
	const FZLSocialObservation& Trigger,
	const FString& SpeechContent,
	const EZLSocialSandboxDecisionTriggerReason Reason,
	const bool bAdvanceStateVersion)
{
	if (!IsValid(Npc))
	{
		return;
	}
	if (Npc->GetStableId() == TEXT("npc_guard"))
	{
		QueueGuardDecision(Npc, Trigger, SpeechContent, Reason, bAdvanceStateVersion);
		return;
	}
	if (bAdvanceStateVersion)
	{
		Npc->AdvanceAuthorityStateVersion();
	}
	FZLSocialSandboxScheduledDecision Scheduled;
	Scheduled.Observation = Trigger;
	Scheduled.SpeechContent = SpeechContent.Left(512);
	Scheduled.Reason = Reason;
	const EZLSocialSandboxQueueResult Result = MultiNpcDecision.Queue(Npc->GetStableId(), Scheduled);
	FZLSocialSandboxDecisionDebug& Debug = NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.TriggerReason = FName(FZLSocialSandboxDecisionScheduler::ReasonName(Reason));
	Debug.bPending = MultiNpcDecision.HasPending(Npc->GetStableId());
	Debug.CoalescedTriggers = MultiNpcDecision.GetCoalescedCount(Npc->GetStableId());
	Debug.AutomaticReplans = MultiNpcDecision.GetAutomaticReplanCount(Npc->GetStableId());
	if (Result != EZLSocialSandboxQueueResult::AutomaticLimit)
	{
		TryDispatchNpcDecisions();
	}
}

void AZLSocialSandboxGameMode::TryDispatchNpcDecisions()
{
	if (GetWorld() == nullptr)
	{
		return;
	}
	const int32 AvailableSlots = FZLSocialSandboxMultiNpcDecision::MaxInFlight
		- MultiNpcDecision.GetInFlightCount()
		- (GuardDecisionScheduler.IsInFlight() ? 1 : 0);
	if (AvailableSlots <= 0)
	{
		return;
	}
	double ShortestDelay = 0.0;
	for (int32 Slot = 0; Slot < AvailableSlots; ++Slot)
	{
		FZLSocialSandboxNpcDispatch Dispatch;
		double DelaySeconds = 0.0;
		if (!MultiNpcDecision.TakeNext(GetWorld()->GetTimeSeconds(), Dispatch, DelaySeconds))
		{
			if (DelaySeconds > 0.0 && (ShortestDelay <= 0.0 || DelaySeconds < ShortestDelay))
			{
				ShortestDelay = DelaySeconds;
			}
			break;
		}
		if (AZLSocialSandboxNpc* Npc = FindSandboxNpc(Dispatch.NpcId))
		{
			RequestNpcDecision(Npc, Dispatch.Decision);
		}
		else
		{
			MultiNpcDecision.MarkCompleted(Dispatch.NpcId);
		}
	}
	if (ShortestDelay > 0.0)
	{
		GetWorldTimerManager().SetTimer(
			NpcDecisionCooldownTimer,
			this,
			&AZLSocialSandboxGameMode::TryDispatchNpcDecisions,
			FMath::Max(0.01, ShortestDelay),
			false);
	}
}

void AZLSocialSandboxGameMode::RequestNpcDecision(
	AZLSocialSandboxNpc* Npc,
	const FZLSocialSandboxScheduledDecision& Scheduled)
{
	if (!IsValid(Npc))
	{
		return;
	}
	FZLSocialSandboxDecisionContextInput Input;
	Input.NpcId = Npc->GetStableId();
	Input.DisplayName = Npc->GetDisplayName();
	Input.Profile = Npc->GetProfile();
	Input.TriggerObservation = Scheduled.Observation;
	Input.TriggerSpeechContent = Scheduled.SpeechContent;
	Input.PersonalHistory = Npc->GetObservationItems();
	Input.PublicHistory = NpcPublicHistory.FindRef(Npc->GetStableId());
	Input.StateVersion = Npc->GetStateVersion();
	FZLDecisionRequest Request;
	FString BuildError;
	FZLSocialSandboxDecisionDebug& Debug = NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	if (!FZLSocialSandboxDecisionContextBuilder::Build(Input, Request, BuildError))
	{
		Debug.ToolResult = TEXT("ContextRejected");
		Npc->ShowDecisionFallback();
		MultiNpcDecision.MarkCompleted(Npc->GetStableId());
		TryDispatchNpcDecisions();
		return;
	}
	UZLAIServiceSubsystem* Service = GetGameInstance() == nullptr
		? nullptr
		: GetGameInstance()->GetSubsystem<UZLAIServiceSubsystem>();
	if (Service == nullptr)
	{
		Debug.Provider = TEXT("local");
		Debug.ToolResult = TEXT("ServiceUnavailable");
		Npc->ShowDecisionFallback();
		MultiNpcDecision.MarkCompleted(Npc->GetStableId());
		TryDispatchNpcDecisions();
		return;
	}
	Debug = FZLSocialSandboxDecisionDebug();
	Debug.RequestId = Request.RequestId;
	Debug.StateVersion = Request.StateVersion;
	Debug.bInFlight = true;
	Debug.TriggerReason = FName(FZLSocialSandboxDecisionScheduler::ReasonName(Scheduled.Reason));
	Debug.bPending = MultiNpcDecision.HasPending(Npc->GetStableId());
	const int32 RequestGeneration = GuardRequestGeneration;
	const double SentAtSeconds = FPlatformTime::Seconds();
	TWeakObjectPtr<AZLSocialSandboxNpc> WeakNpc(Npc);
	Service->SendDecisionRequest(
		MoveTemp(Request),
		FZLDecisionSuccessDelegate::CreateWeakLambda(this, [this, WeakNpc, RequestGeneration, SentAtSeconds](const FZLDecisionResponse& Response)
		{
			if (RequestGeneration == GuardRequestGeneration && WeakNpc.IsValid())
			{
				HandleNpcDecision(WeakNpc.Get(), Response, SentAtSeconds);
			}
		}),
		FZLDecisionFailureDelegate::CreateWeakLambda(this, [this, WeakNpc, RequestGeneration, SentAtSeconds](const FZLServiceError& Error)
		{
			if (RequestGeneration == GuardRequestGeneration && WeakNpc.IsValid())
			{
				HandleNpcDecisionFailure(WeakNpc.Get(), Error, SentAtSeconds);
			}
		}));
}

void AZLSocialSandboxGameMode::HandleNpcDecision(
	AZLSocialSandboxNpc* Npc,
	const FZLDecisionResponse& Response,
	const double SentAtSeconds)
{
	if (!IsValid(Npc))
	{
		return;
	}
	MultiNpcDecision.MarkCompleted(Npc->GetStableId());
	FZLSocialSandboxDecisionDebug& Debug = NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.bInFlight = false;
	Debug.Provider = Response.Provider.Left(32);
	Debug.Intent = Response.Intent.Left(32);
	Debug.StateVersion = Response.StateVersion;
	Debug.LatencyMs = FMath::Clamp(FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0), 0, 60000);
	Debug.bSpeechAccepted = Response.bHasSpeech;
	if (Response.Intent.Equals(TEXT("engage"), ESearchCase::IgnoreCase))
	{
		ApplyGuardConflict(Npc, EZLSocialSandboxConflictEvent::PlannerEngage);
	}
	else if (Response.Intent.Equals(TEXT("disengage"), ESearchCase::IgnoreCase)
		|| Response.Intent.Equals(TEXT("respond"), ESearchCase::IgnoreCase))
	{
		ApplyGuardConflict(Npc, EZLSocialSandboxConflictEvent::PlannerDisengage);
	}
	Npc->ResetDecisionPresentation();
	if (Response.bHasSpeech)
	{
		Npc->ShowDecisionSpeech(Response.Speech.Text, Response.Provider);
		AppendInteractionRecord(
			FText::FromString(FString::Printf(TEXT("[%s · 对话] %s"), *Npc->GetDisplayName().ToString(), *Response.Speech.Text)),
			FLinearColor(1.0f, 0.84f, 0.45f));
		FZLSocialSandboxPublicHistoryFact Fact;
		Fact.Kind = TEXT("speech");
		Fact.SourceId = Npc->GetStableId();
		Fact.TargetId = TEXT("player");
		Fact.Summary = TEXT("This NPC publicly responded to the player.");
		Fact.OccurredAtSeconds = GetWorld()->GetTimeSeconds();
		TArray<FZLSocialSandboxPublicHistoryFact>& History = NpcPublicHistory.FindOrAdd(Npc->GetStableId());
		History.Add(MoveTemp(Fact));
		if (History.Num() > 16) { History.RemoveAt(0, History.Num() - 16, EAllowShrinking::No); }
	}
	if (Response.bHasToolCall)
	{
		ExecuteNpcTool(Npc, Response);
	}
	else
	{
		Debug.ToolName.Reset();
		Debug.ToolResult = TEXT("NoTool");
	}
	Debug.bPending = MultiNpcDecision.HasPending(Npc->GetStableId());
	TryDispatchGuardDecision();
	TryDispatchNpcDecisions();
	RefreshInspector();
}

void AZLSocialSandboxGameMode::ExecuteNpcTool(
	AZLSocialSandboxNpc* Npc,
	const FZLDecisionResponse& Response)
{
	if (!IsValid(Npc) || GetWorld() == nullptr)
	{
		return;
	}
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	const double NowSeconds = GetWorld()->GetTimeSeconds();
	TArray<double>& ExecutionTimes = NpcExecutionTimes.FindOrAdd(Npc->GetStableId());
	ExecutionTimes.RemoveAll([NowSeconds](const double Value) { return NowSeconds - Value >= 10.0; });

	FZLSocialToolCall Call;
	Call.CallId = Response.ToolCall.CallId;
	Call.Name = FName(*Response.ToolCall.Name);
	Call.TargetId = Response.ToolCall.TargetId.IsEmpty() ? NAME_None : FName(*Response.ToolCall.TargetId);
	Call.StateVersion = Response.StateVersion;
	FZLSocialToolValidationContext Context;
	Context.CurrentStateVersion = Npc->GetStateVersion();
	Context.NowSeconds = NowSeconds;
	Context.DistanceToTarget = Player == nullptr
		? TNumericLimits<float>::Max()
		: FVector::Dist2D(Npc->GetActorLocation(), Player->GetActorLocation());
	Context.bTargetValid = Player != nullptr && (Call.TargetId.IsNone() || Call.TargetId == TEXT("player"));
	Context.bNavigationReachable = Player != nullptr
		&& FMath::Abs(Npc->GetActorLocation().Z - Player->GetActorLocation().Z) <= 200.0f;
	Context.bExecutable = !Npc->IsIncapacitated();
	Context.ExecutionsInWindow = ExecutionTimes.Num();
	Context.Capabilities = {TEXT("Tool.FaceTarget"), TEXT("Tool.MoveToward"), TEXT("Tool.MoveAway"), TEXT("Tool.Stop")};
	const FZLSocialToolValidationResult Validation = ToolRegistry.ValidateAndCommit(Call, Context);
	FZLSocialSandboxDecisionDebug& Debug = NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.ToolName = Response.ToolCall.Name.Left(32);
	Debug.ToolResult = Validation.ReasonCode;
	if (!Validation.bAccepted)
	{
		if (!Response.bHasSpeech) { Npc->ShowDecisionRejection(Validation.ReasonCode); }
		return;
	}

	EZLSocialActionType Action = EZLSocialActionType::Stop;
	if (Call.Name == TEXT("face_target")) { Action = EZLSocialActionType::Face; }
	else if (Call.Name == TEXT("move_toward")) { Action = EZLSocialActionType::Approach; }
	else if (Call.Name == TEXT("move_away")) { Action = EZLSocialActionType::MoveAway; }
	const FName TargetId = Action == EZLSocialActionType::Stop ? NAME_None : FName(TEXT("player"));
	DispatchNpcActionObservation(Npc, Action, EZLSocialActionPhase::Started, TargetId);
	TWeakObjectPtr<AZLSocialSandboxGameMode> WeakThis(this);
	TWeakObjectPtr<AZLSocialSandboxNpc> WeakNpc(Npc);
	if (!Npc->StartDecisionAction(Action, Player, [WeakThis, WeakNpc, Action, TargetId]()
	{
		if (WeakThis.IsValid() && WeakNpc.IsValid())
		{
			const FZLSocialObservation Completed = WeakThis->DispatchNpcActionObservation(
				WeakNpc.Get(),
				Action,
				EZLSocialActionPhase::Completed,
				TargetId);
			WeakNpc->ShowDecisionAction(Action, EZLSocialActionPhase::Completed);
			if (Completed.EventId.IsValid())
			{
				WeakThis->QueueNpcDecision(
					WeakNpc.Get(),
					Completed,
					FString(),
					EZLSocialSandboxDecisionTriggerReason::PlanCompleted);
			}
			WeakThis->RefreshInspector();
		}
	}))
	{
		Debug.ToolResult = TEXT("HandlerRejected");
		if (!Response.bHasSpeech) { Npc->ShowDecisionRejection(Debug.ToolResult); }
		return;
	}
	ExecutionTimes.Add(NowSeconds);
	while (ExecutionTimes.Num() > FZLSocialToolRegistry::MaxExecutionsPerWindow)
	{
		ExecutionTimes.RemoveAt(0, 1, EAllowShrinking::No);
	}
	Debug.ToolResult = ZLSocialToolReason::Accepted;
	if (Npc->IsDecisionActionActive())
	{
		Npc->ShowDecisionAction(Action, EZLSocialActionPhase::Started);
	}
}

void AZLSocialSandboxGameMode::HandleNpcDecisionFailure(
	AZLSocialSandboxNpc* Npc,
	const FZLServiceError& Error,
	const double SentAtSeconds)
{
	if (!IsValid(Npc))
	{
		return;
	}
	MultiNpcDecision.MarkCompleted(Npc->GetStableId());
	FZLSocialSandboxDecisionDebug& Debug = NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.bInFlight = false;
	Debug.Provider = TEXT("local");
	Debug.Intent = TEXT("hold");
	Debug.ToolResult = Error.Code.IsEmpty() ? TEXT("DecisionUnavailable") : FName(*Error.Code.Left(64));
	Debug.LatencyMs = FMath::Clamp(FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0), 0, 60000);
	Debug.bLocalFallback = true;
	ApplyGuardConflict(Npc, EZLSocialSandboxConflictEvent::LocalFallback, true);
	Npc->StopDecisionAction();
	Npc->ShowDecisionFallback();
	TryDispatchGuardDecision();
	TryDispatchNpcDecisions();
	RefreshInspector();
}

void AZLSocialSandboxGameMode::QueueGuardDecision(
	AZLSocialSandboxNpc* Guard,
	const FZLSocialObservation& Trigger,
	const FString& SpeechContent,
	const EZLSocialSandboxDecisionTriggerReason Reason,
	const bool bAdvanceStateVersion)
{
	if (!IsValid(Guard) || Guard->GetStableId() != TEXT("npc_guard"))
	{
		return;
	}
	if (bAdvanceStateVersion)
	{
		Guard->AdvanceAuthorityStateVersion();
	}
	FZLSocialSandboxScheduledDecision Scheduled;
	Scheduled.Observation = Trigger;
	Scheduled.SpeechContent = SpeechContent.Left(512);
	Scheduled.Reason = Reason;
	const EZLSocialSandboxQueueResult Result = GuardDecisionScheduler.Queue(Scheduled);
	DecisionDebug.TriggerReason = FName(FZLSocialSandboxDecisionScheduler::ReasonName(Reason));
	DecisionDebug.bPending = GuardDecisionScheduler.HasPending();
	DecisionDebug.CoalescedTriggers = GuardDecisionScheduler.GetCoalescedCount();
	DecisionDebug.AutomaticReplans = GuardDecisionScheduler.GetAutomaticReplanCount();
	if (Result == EZLSocialSandboxQueueResult::AutomaticLimit)
	{
		DecisionDebug.ToolResult = TEXT("AutomaticReplanLimit");
		RefreshInspector();
		return;
	}
	TryDispatchGuardDecision();
}

void AZLSocialSandboxGameMode::TryDispatchGuardDecision()
{
	if (MultiNpcDecision.GetInFlightCount() >= FZLSocialSandboxMultiNpcDecision::MaxInFlight)
	{
		return;
	}
	if (GetWorld() == nullptr)
	{
		return;
	}
	FZLSocialSandboxScheduledDecision Scheduled;
	double DelaySeconds = 0.0;
	if (!GuardDecisionScheduler.TakeReady(GetWorld()->GetTimeSeconds(), Scheduled, DelaySeconds))
	{
		DecisionDebug.bPending = GuardDecisionScheduler.HasPending();
		if (!GuardDecisionScheduler.IsInFlight() && GuardDecisionScheduler.HasPending() && DelaySeconds > 0.0)
		{
			SchedulePendingGuardDecision(DelaySeconds);
		}
		RefreshInspector();
		return;
	}
	GetWorldTimerManager().ClearTimer(GuardDecisionCooldownTimer);
	GuardDecisionScheduler.MarkDispatched(GetWorld()->GetTimeSeconds(), Scheduled);
	DecisionDebug.bPending = GuardDecisionScheduler.HasPending();
	DecisionDebug.AutomaticReplans = GuardDecisionScheduler.GetAutomaticReplanCount();
	AZLSocialSandboxNpc* Guard = FindSandboxNpc(TEXT("npc_guard"));
	if (!IsValid(Guard))
	{
		GuardDecisionScheduler.MarkCompleted();
		return;
	}
	RequestGuardDecision(Guard, Scheduled.Observation, Scheduled.SpeechContent, Scheduled.Reason);
}

void AZLSocialSandboxGameMode::SchedulePendingGuardDecision(const double DelaySeconds)
{
	GetWorldTimerManager().SetTimer(
		GuardDecisionCooldownTimer,
		this,
		&AZLSocialSandboxGameMode::TryDispatchGuardDecision,
		FMath::Clamp(static_cast<float>(DelaySeconds), 0.01f, 10.0f),
		false);
}

void AZLSocialSandboxGameMode::RequestGuardDecision(
	AZLSocialSandboxNpc* Guard,
	const FZLSocialObservation& Trigger,
	const FString& SpeechContent,
	const EZLSocialSandboxDecisionTriggerReason Reason)
{
	if (!IsValid(Guard) || Guard->GetStableId() != TEXT("npc_guard"))
	{
		GuardDecisionScheduler.MarkCompleted();
		return;
	}

	FZLSocialSandboxDecisionContextInput Input;
	Input.NpcId = Guard->GetStableId();
	Input.DisplayName = Guard->GetDisplayName();
	Input.Profile = Guard->GetProfile();
	Input.TriggerObservation = Trigger;
	Input.TriggerSpeechContent = SpeechContent;
	Input.PersonalHistory = Guard->GetObservationItems();
	Input.PublicHistory = GuardPublicHistory;
	Input.StateVersion = Guard->GetStateVersion();
	FZLDecisionRequest Request;
	FString BuildError;
	if (!FZLSocialSandboxDecisionContextBuilder::Build(Input, Request, BuildError))
	{
		DecisionDebug.ToolResult = TEXT("ContextRejected");
		Guard->ShowDecisionFallback();
		GuardDecisionScheduler.MarkCompleted();
		TryDispatchGuardDecision();
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
		GuardDecisionScheduler.MarkCompleted();
		TryDispatchGuardDecision();
		RefreshInspector();
		return;
	}

	DecisionDebug = FZLSocialSandboxDecisionDebug();
	DecisionDebug.RequestId = Request.RequestId;
	DecisionDebug.StateVersion = Request.StateVersion;
	DecisionDebug.bInFlight = true;
	DecisionDebug.TriggerReason = FName(FZLSocialSandboxDecisionScheduler::ReasonName(Reason));
	DecisionDebug.bPending = GuardDecisionScheduler.HasPending();
	DecisionDebug.CoalescedTriggers = GuardDecisionScheduler.GetCoalescedCount();
	DecisionDebug.AutomaticReplans = GuardDecisionScheduler.GetAutomaticReplanCount();
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
	GuardDecisionScheduler.MarkCompleted();
	DecisionDebug.bInFlight = false;
	DecisionDebug.Provider = Response.Provider.Left(32);
	DecisionDebug.Intent = Response.Intent.Left(32);
	DecisionDebug.StateVersion = Response.StateVersion;
	DecisionDebug.LatencyMs = FMath::Clamp(
		FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0),
		0,
		60000);
	DecisionDebug.bSpeechAccepted = Response.bHasSpeech;
	if (Response.Intent.Equals(TEXT("engage"), ESearchCase::IgnoreCase))
	{
		ApplyGuardConflict(Guard, EZLSocialSandboxConflictEvent::PlannerEngage);
	}
	else if (Response.Intent.Equals(TEXT("disengage"), ESearchCase::IgnoreCase)
		|| Response.Intent.Equals(TEXT("respond"), ESearchCase::IgnoreCase))
	{
		ApplyGuardConflict(Guard, EZLSocialSandboxConflictEvent::PlannerDisengage);
	}
	Guard->ResetDecisionPresentation();
	if (Response.bHasSpeech)
	{
		Guard->ShowDecisionSpeech(Response.Speech.Text, Response.Provider);
		AppendInteractionRecord(
			FText::FromString(FString::Printf(TEXT("[%s · 对话] %s"), *Guard->GetDisplayName().ToString(), *Response.Speech.Text)),
			FLinearColor(1.0f, 0.84f, 0.45f));
		RecordGuardSpeechFact(Response.Speech.Text, GetWorld()->GetTimeSeconds());
	}
	if (Response.bHasToolCall)
	{
		ExecuteGuardTool(Guard, Response);
	}
	else
	{
		DecisionDebug.ToolResult = TEXT("NoTool");
	}
	DecisionDebug.bPending = GuardDecisionScheduler.HasPending();
	DecisionDebug.AutomaticReplans = GuardDecisionScheduler.GetAutomaticReplanCount();
	RefreshInspector();
	TryDispatchGuardDecision();
	TryDispatchNpcDecisions();
}

void AZLSocialSandboxGameMode::HandleGuardDecisionFailure(
	AZLSocialSandboxNpc* Guard,
	const FZLServiceError& Error,
	const double SentAtSeconds)
{
	GuardDecisionScheduler.MarkCompleted();
	DecisionDebug.bInFlight = false;
	DecisionDebug.Provider = TEXT("local");
	DecisionDebug.Intent = TEXT("hold");
	DecisionDebug.ToolName.Reset();
	DecisionDebug.ToolResult = Error.Code.IsEmpty() ? TEXT("DecisionUnavailable") : FName(*Error.Code.Left(64));
	DecisionDebug.LatencyMs = FMath::Clamp(
		FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0),
		0,
		60000);
	ApplyGuardConflict(Guard, EZLSocialSandboxConflictEvent::LocalFallback, true);
	Guard->StopDecisionAction();
	Guard->ShowDecisionFallback();
	DecisionDebug.bPending = GuardDecisionScheduler.HasPending();
	RefreshInspector();
	TryDispatchGuardDecision();
	TryDispatchNpcDecisions();
}

void AZLSocialSandboxGameMode::ApplyGuardConflict(
	AZLSocialSandboxNpc* Guard,
	const EZLSocialSandboxConflictEvent Event,
	const bool bLocalFallback)
{
	if (!IsValid(Guard))
	{
		return;
	}
	FZLSocialSandboxConflictState& ConflictState = Guard->GetStableId() == TEXT("npc_guard")
		? GuardConflictState
		: NpcConflictStates.FindOrAdd(Guard->GetStableId());
	FZLSocialSandboxDecisionDebug& Debug = Guard->GetStableId() == TEXT("npc_guard")
		? DecisionDebug
		: NpcDecisionDebug.FindOrAdd(Guard->GetStableId());
	const FZLSocialSandboxConflictTransition Transition = ConflictState.Apply(Event);
	Guard->SetDefending(Transition.bShouldDefend);
	if (Transition.bChanged)
	{
		Guard->AdvanceAuthorityStateVersion();
	}
	Debug.ConflictLevel = FZLSocialSandboxConflictState::LevelName(Transition.Current);
	Debug.bLocalFallback = bLocalFallback;
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
			const FZLSocialObservation Completed = WeakThis->DispatchNpcActionObservation(
				WeakGuard.Get(),
				Action,
				EZLSocialActionPhase::Completed,
				TargetId);
			WeakGuard->ShowDecisionAction(Action, EZLSocialActionPhase::Completed);
			if (Completed.EventId.IsValid())
			{
				WeakThis->QueueGuardDecision(
					WeakGuard.Get(),
					Completed,
					FString(),
					EZLSocialSandboxDecisionTriggerReason::PlanCompleted);
			}
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
	if (!ExpectedSmokeProvider.IsEmpty())
	{
		bSmokeSawAcceptedTool = true;
	}
	if (Guard->IsDecisionActionActive())
	{
		Guard->ShowDecisionAction(Action, EZLSocialActionPhase::Started);
	}
}

FZLSocialObservation AZLSocialSandboxGameMode::DispatchNpcActionObservation(
	AZLSocialSandboxNpc* Actor,
	const EZLSocialActionType Action,
	const EZLSocialActionPhase Phase,
	const FName TargetId)
{
	FZLSocialObservation SelfObservation;
	if (!IsValid(Actor) || GetWorld() == nullptr) { return SelfObservation; }
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
	if (!Event.IsValid(NowSeconds)) { return SelfObservation; }
	SelfObservation.EventId = Event.EventId;
	SelfObservation.ObserverId = Actor->GetStableId();
	SelfObservation.SourceId = Actor->GetStableId();
	SelfObservation.Source = EZLSocialObservationSource::Action;
	SelfObservation.Action = Action;
	SelfObservation.ActionPhase = Phase;
	SelfObservation.ExplicitTargetId = TargetId;
	SelfObservation.TargetJudgment = TargetId.IsNone()
		? EZLSocialTargetJudgment::Unresolved
		: EZLSocialTargetJudgment::ExplicitOther;
	SelfObservation.bSaw = true;
	SelfObservation.ObservedAtSeconds = NowSeconds;
	Actor->RecordObservation(SelfObservation);
	if (Phase == EZLSocialActionPhase::Started)
	{
		const TCHAR* ActionText = TEXT("停止");
		switch (Action)
		{
		case EZLSocialActionType::Face: ActionText = TEXT("面向"); break;
		case EZLSocialActionType::Approach: ActionText = TEXT("靠近"); break;
		case EZLSocialActionType::MoveAway: ActionText = TEXT("远离"); break;
		default: break;
		}
		AppendInteractionRecord(
			FText::FromString(FString::Printf(TEXT("[%s · 行动] %s玩家"), *Actor->GetDisplayName().ToString(), ActionText)),
			FLinearColor(0.4f, 0.88f, 1.0f));
	}
	if (Actor->GetStableId() == TEXT("npc_guard"))
	{
		RecordGuardActionFact(Action, Phase, NowSeconds);
	}
	else
	{
		FZLSocialSandboxPublicHistoryFact Fact;
		Fact.Kind = TEXT("action_result");
		Fact.SourceId = Actor->GetStableId();
		Fact.TargetId = TargetId;
		Fact.Summary = FString::Printf(
			TEXT("This NPC %s a visible %s action."),
			Phase == EZLSocialActionPhase::Started ? TEXT("started") : TEXT("completed"),
			Action == EZLSocialActionType::Face ? TEXT("Face")
				: Action == EZLSocialActionType::Approach ? TEXT("Approach")
				: Action == EZLSocialActionType::MoveAway ? TEXT("MoveAway")
				: TEXT("Stop"));
		Fact.OccurredAtSeconds = NowSeconds;
		TArray<FZLSocialSandboxPublicHistoryFact>& History = NpcPublicHistory.FindOrAdd(Actor->GetStableId());
		History.Add(MoveTemp(Fact));
		if (History.Num() > 16) { History.RemoveAt(0, History.Num() - 16, EAllowShrinking::No); }
	}
	const FZLSocialObservationEvaluator Evaluator(ObservationSettings);
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (!IsValid(Npc) || Npc == Actor) { continue; }
		FZLSocialObserver Observer;
		Observer.AgentId = Npc->GetStableId();
		Observer.Position = Npc->GetActorLocation();
		Observer.Forward = Npc->GetPlanarForwardVector();
		const FZLSocialObservation Observation = Evaluator.ObserveAction(Event, Observer, NowSeconds);
		Npc->RecordObservation(Observation);
		Npc->ShowActionObservation(Observation);
		if (Phase == EZLSocialActionPhase::Completed && Observation.bSaw)
		{
			QueueNpcDecision(
				Npc,
				Observation,
				FString(),
				EZLSocialSandboxDecisionTriggerReason::PlayerAction);
		}
	}
	return SelfObservation;
}

void AZLSocialSandboxGameMode::RecordGuardSpeechFact(const FString& Text, const double OccurredAtSeconds)
{
	const FString Bounded = Text.TrimStartAndEnd().Left(192);
	if (Bounded.IsEmpty())
	{
		return;
	}
	FZLSocialSandboxPublicHistoryFact Fact;
	Fact.Kind = TEXT("speech");
	Fact.SourceId = TEXT("npc_guard");
	Fact.TargetId = TEXT("player");
	Fact.Summary = FString::Printf(TEXT("The guard publicly said: %s"), *Bounded).Left(256);
	Fact.OccurredAtSeconds = OccurredAtSeconds;
	GuardPublicHistory.Add(MoveTemp(Fact));
	while (GuardPublicHistory.Num() > 16)
	{
		GuardPublicHistory.RemoveAt(0, 1, EAllowShrinking::No);
	}
}

void AZLSocialSandboxGameMode::RecordGuardActionFact(
	const EZLSocialActionType Action,
	const EZLSocialActionPhase Phase,
	const double OccurredAtSeconds)
{
	const TCHAR* ActionText = TEXT("Stop");
	switch (Action)
	{
	case EZLSocialActionType::Face: ActionText = TEXT("Face"); break;
	case EZLSocialActionType::Approach: ActionText = TEXT("Approach"); break;
	case EZLSocialActionType::MoveAway: ActionText = TEXT("MoveAway"); break;
	case EZLSocialActionType::Attack: ActionText = TEXT("Attack"); break;
	default: break;
	}
	FZLSocialSandboxPublicHistoryFact Fact;
	Fact.Kind = TEXT("action_result");
	Fact.SourceId = TEXT("npc_guard");
	Fact.TargetId = Action == EZLSocialActionType::Stop ? NAME_None : FName(TEXT("player"));
	Fact.Summary = FString::Printf(
		TEXT("The guard %s action %s."),
		Phase == EZLSocialActionPhase::Started ? TEXT("started") : TEXT("completed"),
		ActionText);
	Fact.OccurredAtSeconds = OccurredAtSeconds;
	GuardPublicHistory.Add(MoveTemp(Fact));
	while (GuardPublicHistory.Num() > 16)
	{
		GuardPublicHistory.RemoveAt(0, 1, EAllowShrinking::No);
	}
}

void AZLSocialSandboxGameMode::UpdateGuardDistanceBand()
{
	AZLSocialSandboxNpc* Guard = FindSandboxNpc(TEXT("npc_guard"));
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!IsValid(Guard) || !IsValid(Player) || GetWorld() == nullptr)
	{
		return;
	}
	const float Distance = FVector::Dist2D(Guard->GetActorLocation(), Player->GetActorLocation());
	const int32 NewBand = Distance < 250.0f ? 0 : (Distance < 800.0f ? 1 : 2);
	if (GuardDistanceBand == INDEX_NONE)
	{
		GuardDistanceBand = NewBand;
		LastGuardDistance = Distance;
		return;
	}
	if (NewBand == GuardDistanceBand)
	{
		LastGuardDistance = Distance;
		return;
	}

	const bool bMovedNearer = Distance < LastGuardDistance;
	ApplyGuardConflict(
		Guard,
		bMovedNearer ? EZLSocialSandboxConflictEvent::DistanceNear : EZLSocialSandboxConflictEvent::DistanceFar);
	GuardDistanceBand = NewBand;
	LastGuardDistance = Distance;
	const double NowSeconds = GetWorld()->GetTimeSeconds();
	FZLSocialActionEvent Event;
	Event.EventId = FGuid::NewGuid();
	Event.Action = bMovedNearer ? EZLSocialActionType::Approach : EZLSocialActionType::MoveAway;
	Event.Phase = EZLSocialActionPhase::Completed;
	Event.ActorId = TEXT("player");
	Event.TargetId = TEXT("npc_guard");
	Event.Position = Player->GetActorLocation();
	Event.Forward = Player->GetActorForwardVector();
	Event.TimestampSeconds = NowSeconds;
	FZLSocialObserver Observer;
	Observer.AgentId = Guard->GetStableId();
	Observer.Position = Guard->GetActorLocation();
	Observer.Forward = Guard->GetPlanarForwardVector();
	const FZLSocialObservation Observation = FZLSocialObservationEvaluator(ObservationSettings).ObserveAction(
		Event,
		Observer,
		NowSeconds);
	Guard->RecordObservation(Observation);
	if (!Observation.bSaw)
	{
		return;
	}
	QueueGuardDecision(
		Guard,
		Observation,
		FString(),
		bMovedNearer
			? EZLSocialSandboxDecisionTriggerReason::DistanceNear
			: EZLSocialSandboxDecisionTriggerReason::DistanceFar,
		true);
}

void AZLSocialSandboxGameMode::UpdateNpcDistanceBands()
{
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!IsValid(Player) || GetWorld() == nullptr)
	{
		return;
	}
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (!IsValid(Npc) || Npc->GetStableId() == TEXT("npc_guard"))
		{
			continue;
		}
		const FName NpcId = Npc->GetStableId();
		const float Distance = FVector::Dist2D(Npc->GetActorLocation(), Player->GetActorLocation());
		const int32 NewBand = Distance < 250.0f ? 0 : (Distance < 800.0f ? 1 : 2);
		int32* PreviousBand = NpcDistanceBands.Find(NpcId);
		float* PreviousDistance = NpcLastDistances.Find(NpcId);
		if (PreviousBand == nullptr || PreviousDistance == nullptr)
		{
			NpcDistanceBands.Add(NpcId, NewBand);
			NpcLastDistances.Add(NpcId, Distance);
			continue;
		}
		if (*PreviousBand == NewBand)
		{
			*PreviousDistance = Distance;
			continue;
		}
		const bool bMovedNearer = Distance < *PreviousDistance;
		ApplyGuardConflict(
			Npc,
			bMovedNearer ? EZLSocialSandboxConflictEvent::DistanceNear : EZLSocialSandboxConflictEvent::DistanceFar);
		*PreviousBand = NewBand;
		*PreviousDistance = Distance;

		const double NowSeconds = GetWorld()->GetTimeSeconds();
		FZLSocialActionEvent Event;
		Event.EventId = FGuid::NewGuid();
		Event.Action = bMovedNearer ? EZLSocialActionType::Approach : EZLSocialActionType::MoveAway;
		Event.Phase = EZLSocialActionPhase::Completed;
		Event.ActorId = TEXT("player");
		Event.TargetId = NpcId;
		Event.Position = Player->GetActorLocation();
		Event.Forward = Player->GetActorForwardVector();
		Event.TimestampSeconds = NowSeconds;
		FZLSocialObserver Observer;
		Observer.AgentId = NpcId;
		Observer.Position = Npc->GetActorLocation();
		Observer.Forward = Npc->GetPlanarForwardVector();
		const FZLSocialObservation Observation = FZLSocialObservationEvaluator(ObservationSettings).ObserveAction(
			Event,
			Observer,
			NowSeconds);
		Npc->RecordObservation(Observation);
		if (Observation.bSaw)
		{
			QueueNpcDecision(
				Npc,
				Observation,
				FString(),
				bMovedNearer
					? EZLSocialSandboxDecisionTriggerReason::DistanceNear
					: EZLSocialSandboxDecisionTriggerReason::DistanceFar,
				true);
		}
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
				: bSmokeSawAcceptedTool));
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
			? (bVersionChanged && !bLocationChanged)
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

void AZLSocialSandboxGameMode::FinishMultiNpcSmokeTest()
{
	const FName NpcIds[] = {TEXT("npc_guard"), TEXT("npc_merchant"), TEXT("npc_rival"), TEXT("npc_civilian")};
	int32 ValidNpcCount = 0;
	int32 StubSpeechCount = 0;
	for (const FName NpcId : NpcIds)
	{
		if (FindSandboxNpc(NpcId) != nullptr)
		{
			++ValidNpcCount;
		}
		const FZLSocialSandboxDecisionDebug* Debug = NpcId == TEXT("npc_guard")
			? &DecisionDebug
			: NpcDecisionDebug.Find(NpcId);
		if (Debug != nullptr
			&& Debug->Provider.Equals(TEXT("stub"), ESearchCase::IgnoreCase)
			&& Debug->bSpeechAccepted)
		{
			++StubSpeechCount;
		}
	}
	const bool bBounded = MultiNpcDecision.GetInFlightCount()
		+ (GuardDecisionScheduler.IsInFlight() ? 1 : 0)
		<= FZLSocialSandboxMultiNpcDecision::MaxInFlight;
	const bool bPassed = ValidNpcCount == UE_ARRAY_COUNT(NpcIds)
		&& StubSpeechCount == UE_ARRAY_COUNT(NpcIds)
		&& bBounded;
	UE_LOG(
		LogZL,
		Display,
		TEXT("ZL_SANDBOX_MULTI_SMOKE result=%s npc_count=%d stub_speech=%d in_flight=%d bounded=%s"),
		bPassed ? TEXT("Success") : TEXT("Fail"),
		ValidNpcCount,
		StubSpeechCount,
		MultiNpcDecision.GetInFlightCount() + (GuardDecisionScheduler.IsInFlight() ? 1 : 0),
		bBounded ? TEXT("Yes") : TEXT("No"));
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
		return FText::FromString(FString::Printf(TEXT("%s\n尚无个人感知记录"), *Npc->GetDisplayName().ToString()));
	}
	auto YesNo = [](const bool Value) { return Value ? TEXT("是") : TEXT("否"); };
	auto SpeechModeText = [](const EZLSocialSpeechMode Mode)
	{
		switch (Mode) { case EZLSocialSpeechMode::Whisper: return TEXT("小声说话"); case EZLSocialSpeechMode::Shout: return TEXT("大声呼喊"); case EZLSocialSpeechMode::InEar: return TEXT("耳边说话"); default: return TEXT("正常说话"); }
	};
	auto TargetText = [](const EZLSocialTargetJudgment Value)
	{
		switch (Value) { case EZLSocialTargetJudgment::Candidate: return TEXT("可能指向自己"); case EZLSocialTargetJudgment::ExplicitSelf: return TEXT("明确指向自己"); case EZLSocialTargetJudgment::ExplicitOther: return TEXT("明确指向他人"); default: return TEXT("未确定"); }
	};
	auto FilterText = [](const EZLSocialObservationFilterReason Value)
	{
		switch (Value) { case EZLSocialObservationFilterReason::InvalidEvent: return TEXT("事件无效"); case EZLSocialObservationFilterReason::Expired: return TEXT("事件已过期"); case EZLSocialObservationFilterReason::CannotSee: return TEXT("无法看见"); case EZLSocialObservationFilterReason::CannotHear: return TEXT("无法听见"); case EZLSocialObservationFilterReason::OutsideVisualRange: return TEXT("超出视觉范围"); case EZLSocialObservationFilterReason::OutsideFieldOfView: return TEXT("不在视野内"); case EZLSocialObservationFilterReason::OutsideHearingRange: return TEXT("超出听觉范围"); case EZLSocialObservationFilterReason::NotExplicitInEarTarget: return TEXT("不是耳边说话目标"); default: return TEXT("无"); }
	};
	const FString Source = Observation->Source == EZLSocialObservationSource::Speech ? TEXT("说话") : TEXT("行为");
	auto ActionText = [](const EZLSocialActionType Value)
	{
		switch (Value) { case EZLSocialActionType::Face: return TEXT("面向"); case EZLSocialActionType::Approach: return TEXT("靠近"); case EZLSocialActionType::MoveAway: return TEXT("远离"); case EZLSocialActionType::Attack: return TEXT("攻击"); default: return TEXT("停止"); }
	};
	auto IntentText = [](const FString& Value)
	{
		if (Value == TEXT("respond")) { return TEXT("回应"); }
		if (Value == TEXT("engage")) { return TEXT("介入"); }
		if (Value == TEXT("disengage")) { return TEXT("脱离"); }
		if (Value == TEXT("hold")) { return TEXT("保持"); }
		return TEXT("无");
	};
	auto ToolText = [](const FString& Value)
	{
		if (Value == TEXT("face_target")) { return TEXT("面向目标"); }
		if (Value == TEXT("move_toward")) { return TEXT("靠近目标"); }
		if (Value == TEXT("move_away")) { return TEXT("远离目标"); }
		if (Value == TEXT("stop")) { return TEXT("停止"); }
		return TEXT("无");
	};
	auto ToolResultText = [](const FName Value)
	{
		if (Value == TEXT("Accepted")) { return TEXT("已接受"); }
		if (Value == TEXT("NoTool")) { return TEXT("未建议工具"); }
		if (Value == TEXT("ServiceUnavailable")) { return TEXT("服务不可用"); }
		if (Value == TEXT("StateVersionMismatch")) { return TEXT("状态已变化"); }
		return Value.IsNone() ? TEXT("无") : TEXT("未执行");
	};
	const FString SourceDetails = Observation->Source == EZLSocialObservationSource::Speech
		? FString::Printf(TEXT("说话方式：%s · 明确目标：%s\n目标判断：%s · 听觉过滤：%s"), SpeechModeText(Observation->SpeechMode), Observation->ExplicitTargetId.IsNone() ? TEXT("无") : *Observation->ExplicitTargetId.ToString(), TargetText(Observation->TargetJudgment), FilterText(Observation->AuditoryFilter))
		: FString::Printf(TEXT("行为：%s · 阶段：%s · 目标：%s\n目标判断：%s · 输入文本：无"), ActionText(Observation->Action), Observation->ActionPhase == EZLSocialActionPhase::Started ? TEXT("开始") : TEXT("完成"), Observation->ExplicitTargetId.IsNone() ? TEXT("无") : *Observation->ExplicitTargetId.ToString(), TargetText(Observation->TargetJudgment));
	const FZLSocialSandboxDecisionDebug* SelectedDebug = NpcId == TEXT("npc_guard")
		? &DecisionDebug
		: NpcDecisionDebug.Find(NpcId);
	const FString DecisionLine = SelectedDebug != nullptr
		? FString::Printf(
			TEXT("\n冲突：%s · 生命 %.0f/%.0f · 防御：%s · 失能：%s\n决策：%s · 待处理：%s · 触发：%s · 本地降级：%s\n请求：%s · 状态：%lld · 合并：%d · 自动：%d\n来源：%s · 意图：%s · 台词：%s\n工具：%s · 结果：%s · 延迟：%d 毫秒"),
			SelectedDebug->ConflictLevel.IsEmpty() ? TEXT("平静") : *SelectedDebug->ConflictLevel,
			Npc->GetHealth(),
			Npc->GetMaxHealth(),
			Npc->IsDefending() ? TEXT("是") : TEXT("否"),
			Npc->IsIncapacitated() ? TEXT("是") : TEXT("否"),
			SelectedDebug->bInFlight ? TEXT("进行中") : TEXT("空闲"),
			SelectedDebug->bPending ? TEXT("是") : TEXT("否"),
			SelectedDebug->TriggerReason.IsNone() ? TEXT("无") : *SelectedDebug->TriggerReason.ToString(),
			SelectedDebug->bLocalFallback ? TEXT("是") : TEXT("否"),
			SelectedDebug->RequestId.IsEmpty() ? TEXT("无") : *SelectedDebug->RequestId,
			SelectedDebug->StateVersion,
			SelectedDebug->CoalescedTriggers,
			SelectedDebug->AutomaticReplans,
			SelectedDebug->Provider.IsEmpty() ? TEXT("无") : *SelectedDebug->Provider,
			IntentText(SelectedDebug->Intent),
			SelectedDebug->bSpeechAccepted ? TEXT("已接受") : TEXT("无"),
			ToolText(SelectedDebug->ToolName),
			ToolResultText(SelectedDebug->ToolResult),
			SelectedDebug->LatencyMs)
		: FString();
	const TCHAR* FeedbackSource = SelectedDebug != nullptr
		? TEXT("反馈来源：结构化决策")
		: TEXT("反馈来源：规则占位");
	return FText::FromString(FString::Printf(
		TEXT("%s\n来源：%s · 距离：%.0f 厘米\n看见：%s · 视觉过滤：%s\n听见：%s · 听清：%s · 强度：%.2f\n%s\n输入来源：UE 事件 · %s%s"),
		*Npc->GetDisplayName().ToString(), *Source, Observation->Distance,
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

void AZLSocialSandboxGameMode::AppendInteractionRecord(const FText& Text, const FLinearColor& Color) const
{
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->AppendInteractionRecord(Text, Color);
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

void AZLSocialSandboxGameMode::SpawnNpc(const FName StableId, const FVector& Location, const FRotator& Rotation)
{
	FActorSpawnParameters Params;
	Params.Name = StableId;
	AZLSocialSandboxNpc* Npc = GetWorld()->SpawnActor<AZLSocialSandboxNpc>(AZLSocialSandboxNpc::StaticClass(), Location, Rotation, Params);
	if (Npc != nullptr)
	{
	Npc->InitializeSandboxNpc(FZLSocialSandboxNpcProfile::Create(StableId), FTransform(Rotation, Location));
	SandboxNpcs.Add(Npc);
	if (StableId != TEXT("npc_guard"))
	{
		MultiNpcDecision.RegisterNpc(StableId);
	}
	}
}
