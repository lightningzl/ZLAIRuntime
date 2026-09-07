#include "SocialSandbox/Systems/ZLSocialSandboxDecisionComponent.h"
#include "SocialSandbox/Systems/ZLSocialSandboxInteractionComponent.h"
#include "SocialSandbox/World/ZLSocialSandboxGameMode.h"

#include "ZL.h"
#include "ZLAIServiceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPawn.h"
#include "SocialSandbox/Decision/ZLSocialSandboxDecisionContext.h"
#include "TimerManager.h"

UZLSocialSandboxDecisionComponent::UZLSocialSandboxDecisionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ToolRegistry.RegisterMilestone8Defaults();
}

void UZLSocialSandboxDecisionComponent::ResetState()
{
	++RequestGeneration;
	MultiNpcDecision.Reset();
	NpcDecisionDebug.Reset();
	NpcPublicHistory.Reset();
	NpcSocialFacts.Reset();
	NpcTradeStances.Reset();
	NpcExecutionTimes.Reset();
	NpcConflictStates.Reset();
	NpcDistanceBands.Reset();
	NpcLastDistances.Reset();
	ToolRegistry = FZLSocialToolRegistry();
	ToolRegistry.RegisterMilestone8Defaults();
}

AZLSocialSandboxGameMode* UZLSocialSandboxDecisionComponent::GetSandboxGameMode() const
{
	return Cast<AZLSocialSandboxGameMode>(GetOwner());
}

AZLSocialSandboxNpc* UZLSocialSandboxDecisionComponent::FindSandboxNpc(const FName StableId) const
{
	return GetSandboxGameMode() == nullptr ? nullptr : GetSandboxGameMode()->FindSandboxNpc(StableId);
}

FZLSocialObservation UZLSocialSandboxDecisionComponent::DispatchNpcActionObservation(
	AZLSocialSandboxNpc* Actor,
	const EZLSocialActionType Action,
	const EZLSocialActionPhase Phase,
	const FName TargetId)
{
	return GetSandboxGameMode() == nullptr || GetSandboxGameMode()->GetInteractionComponent() == nullptr
		? FZLSocialObservation()
		: GetSandboxGameMode()->GetInteractionComponent()->DispatchNpcActionObservation(Actor, Action, Phase, TargetId);
}

void UZLSocialSandboxDecisionComponent::RefreshInspector() const
{
	if (AZLSocialSandboxGameMode* GameMode = GetSandboxGameMode()) { GameMode->RefreshInspector(); }
}

void UZLSocialSandboxDecisionComponent::AppendInteractionRecord(const FText& Text, const FLinearColor& Color) const
{
	if (AZLSocialSandboxGameMode* GameMode = GetSandboxGameMode()) { GameMode->AppendInteractionRecord(Text, Color); }
}

void UZLSocialSandboxDecisionComponent::QueueNpcDecision(
	AZLSocialSandboxNpc* Npc,
	const FZLSocialObservation& Trigger,
	const FString& SpeechContent,
	const EZLSocialSandboxDecisionTriggerReason Reason,
	const bool bAdvanceStateVersion)
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
	if (!IsValid(Npc))
	{
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
	const EZLSocialSandboxQueueResult Result = DecisionComponent->MultiNpcDecision.Queue(Npc->GetStableId(), Scheduled);
	FZLSocialSandboxDecisionDebug& Debug = DecisionComponent->NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.TriggerReason = FName(FZLSocialSandboxDecisionScheduler::ReasonName(Reason));
	Debug.bPending = DecisionComponent->MultiNpcDecision.HasPending(Npc->GetStableId());
	Debug.CoalescedTriggers = DecisionComponent->MultiNpcDecision.GetCoalescedCount(Npc->GetStableId());
	Debug.AutomaticReplans = DecisionComponent->MultiNpcDecision.GetAutomaticReplanCount(Npc->GetStableId());
	if (Result != EZLSocialSandboxQueueResult::AutomaticLimit)
	{
		TryDispatchNpcDecisions();
	}
}

void UZLSocialSandboxDecisionComponent::TryDispatchNpcDecisions()
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
	if (GetWorld() == nullptr)
	{
		return;
	}
	const int32 AvailableSlots = FZLSocialSandboxMultiNpcDecision::MaxInFlight
		- DecisionComponent->MultiNpcDecision.GetInFlightCount();
	if (AvailableSlots <= 0)
	{
		return;
	}
	double ShortestDelay = 0.0;
	for (int32 Slot = 0; Slot < AvailableSlots; ++Slot)
	{
		FZLSocialSandboxNpcDispatch Dispatch;
		double DelaySeconds = 0.0;
		if (!DecisionComponent->MultiNpcDecision.TakeNext(GetWorld()->GetTimeSeconds(), Dispatch, DelaySeconds))
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
			DecisionComponent->MultiNpcDecision.MarkCompleted(Dispatch.NpcId);
		}
	}
	if (ShortestDelay > 0.0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			DecisionComponent->CooldownTimer,
			this,
			&UZLSocialSandboxDecisionComponent::TryDispatchNpcDecisions,
			FMath::Max(0.01, ShortestDelay),
			false);
	}
}

void UZLSocialSandboxDecisionComponent::RequestNpcDecision(
	AZLSocialSandboxNpc* Npc,
	const FZLSocialSandboxScheduledDecision& Scheduled)
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
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
	Input.PublicHistory = DecisionComponent->NpcPublicHistory.FindRef(Npc->GetStableId());
	Input.SocialSituation = DecisionComponent->NpcSocialFacts.FindRef(Npc->GetStableId());
	Input.AvailableCapabilities = {
		{TEXT("face_player"), TEXT("face"), {TEXT("player")}},
		{TEXT("keep_distance_from_player"), TEXT("move_away"), {TEXT("player")}},
		{TEXT("become_defensive"), TEXT("set_defending"), {}},
		{TEXT("refuse_trade"), TEXT("set_interaction_stance"), {TEXT("player")}}
	};
	if (Scheduled.Observation.Source == EZLSocialObservationSource::Speech)
	{
		const FString LowerSpeech = Scheduled.SpeechContent.ToLower();
		if (Scheduled.SpeechContent.Contains(TEXT("对不起")) || Scheduled.SpeechContent.Contains(TEXT("抱歉")) || LowerSpeech.Contains(TEXT("sorry")))
		{
			RecordNpcSocialFact(Npc->GetStableId(), TEXT("apology_received"), TEXT("player"), Npc->GetStableId(), TEXT("The player directly apologized to this NPC."), 0.7f);
			Input.SocialSituation = DecisionComponent->NpcSocialFacts.FindRef(Npc->GetStableId());
		}
	}
	Input.StateVersion = Npc->GetStateVersion();
	FZLDecisionV2Request Request;
	FString BuildError;
	FZLSocialSandboxDecisionDebug& Debug = DecisionComponent->NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	if (!FZLSocialSandboxDecisionContextBuilder::BuildV2(Input, Request, BuildError))
	{
		Debug.ToolResult = TEXT("ContextRejected");
		Npc->ShowDecisionFallback();
		DecisionComponent->MultiNpcDecision.MarkCompleted(Npc->GetStableId());
		TryDispatchNpcDecisions();
		return;
	}
	UZLAIServiceSubsystem* Service = GetWorld() == nullptr || GetWorld()->GetGameInstance() == nullptr
		? nullptr
		: GetWorld()->GetGameInstance()->GetSubsystem<UZLAIServiceSubsystem>();
	if (Service == nullptr)
	{
		Debug.Provider = TEXT("local");
		Debug.ToolResult = TEXT("ServiceUnavailable");
		Npc->ShowDecisionFallback();
		DecisionComponent->MultiNpcDecision.MarkCompleted(Npc->GetStableId());
		TryDispatchNpcDecisions();
		return;
	}
	Debug = FZLSocialSandboxDecisionDebug();
	Debug.RequestId = Request.RequestId;
	Debug.StateVersion = Request.StateVersion;
	Debug.bInFlight = true;
	Debug.TriggerReason = FName(FZLSocialSandboxDecisionScheduler::ReasonName(Scheduled.Reason));
	Debug.bPending = DecisionComponent->MultiNpcDecision.HasPending(Npc->GetStableId());
	const int32 RequestGenerationSnapshot = DecisionComponent->RequestGeneration;
	const double SentAtSeconds = FPlatformTime::Seconds();
	TWeakObjectPtr<AZLSocialSandboxNpc> WeakNpc(Npc);
	const FZLDecisionV2Request RequestSnapshot = Request;
	Service->SendDecisionV2Request(
		MoveTemp(Request),
		FZLDecisionV2SuccessDelegate::CreateWeakLambda(this, [this, WeakNpc, RequestGenerationSnapshot, SentAtSeconds, RequestSnapshot](const FZLDecisionV2Response& Response)
		{
			if (RequestGenerationSnapshot == RequestGeneration && WeakNpc.IsValid())
			{
				HandleNpcDecisionV2(WeakNpc.Get(), Response, RequestSnapshot, SentAtSeconds);
			}
		}),
		FZLDecisionFailureDelegate::CreateWeakLambda(this, [this, WeakNpc, RequestGenerationSnapshot, SentAtSeconds](const FZLServiceError& Error)
		{
			if (RequestGenerationSnapshot == RequestGeneration && WeakNpc.IsValid())
			{
				HandleNpcDecisionFailure(WeakNpc.Get(), Error, SentAtSeconds);
			}
		}));
}

void UZLSocialSandboxDecisionComponent::HandleNpcDecisionV2(
	AZLSocialSandboxNpc* Npc,
	const FZLDecisionV2Response& Response,
	const FZLDecisionV2Request& Request,
	const double SentAtSeconds)
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
	if (!IsValid(Npc))
	{
		return;
	}
	DecisionComponent->MultiNpcDecision.MarkCompleted(Npc->GetStableId());
	FZLSocialSandboxDecisionDebug& Debug = DecisionComponent->NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.bInFlight = false;
	Debug.Provider = Response.Provider.Left(32);
	Debug.Intent = Response.Objective.Left(64);
	Debug.StateVersion = Response.StateVersion;
	Debug.LatencyMs = FMath::Clamp(FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0), 0, 60000);
	Debug.bSpeechAccepted = Response.bHasSpeech;
	Npc->ResetDecisionPresentation();
	if (Response.bHasSpeech)
	{
		Npc->ShowDecisionSpeech(Response.Speech.Text, Response.Provider);
		AppendInteractionRecord(
			FText::FromString(FString::Printf(TEXT("[%s · 对话] %s"), *Npc->GetDisplayName().ToString(), *Response.Speech.Text)),
			FLinearColor(1.0f, 0.84f, 0.45f));
	}

	const FZLDecisionV2PlanStep* Step = Response.Steps.Num() > 0 ? &Response.Steps[0] : nullptr;
	const FZLDecisionV2Capability* Capability = Step == nullptr ? nullptr : Request.AvailableCapabilities.FindByPredicate(
		[Step](const FZLDecisionV2Capability& Candidate) { return Candidate.CapabilityId == Step->CapabilityId; });
	const bool bTargetAllowed = Capability != nullptr
		&& ((Step->TargetId.IsEmpty() && Capability->TargetIds.IsEmpty()) || Capability->TargetIds.Contains(Step->TargetId));
	if (Capability == nullptr || !bTargetAllowed)
	{
		Debug.ToolResult = Step == nullptr ? TEXT("NoStep") : TEXT("CapabilityRejected");
	}
	else if (Response.StateVersion != Npc->GetStateVersion())
	{
		Debug.ToolResult = TEXT("StaleState");
	}
	else
	{
		ExecuteNpcPlanStep(Npc, Response, *Capability, *Step);
	}
	Debug.bPending = DecisionComponent->MultiNpcDecision.HasPending(Npc->GetStableId());
	TryDispatchNpcDecisions();
	RefreshInspector();
}

void UZLSocialSandboxDecisionComponent::ExecuteNpcPlanStep(
	AZLSocialSandboxNpc* Npc,
	const FZLDecisionV2Response& Response,
	const FZLDecisionV2Capability& Capability,
	const FZLDecisionV2PlanStep& Step)
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
	FZLSocialSandboxDecisionDebug& Debug = DecisionComponent->NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.ToolName = Capability.CapabilityId.Left(32);
	if (Npc->IsIncapacitated())
	{
		Debug.ToolResult = TEXT("Incapacitated");
		return;
	}
	if (Capability.CapabilityId == TEXT("become_defensive"))
	{
		Npc->SetDefending(true);
		RecordNpcSocialFact(Npc->GetStableId(), TEXT("defensive_posture"), Npc->GetStableId(), TEXT("player"), TEXT("This NPC entered a defensive posture."), 0.55f);
		Debug.ToolResult = TEXT("Accepted");
		return;
	}
	if (Capability.CapabilityId == TEXT("refuse_trade"))
	{
		DecisionComponent->NpcTradeStances.Add(Npc->GetStableId(), TEXT("refused"));
		RecordNpcSocialFact(Npc->GetStableId(), TEXT("trade_stance_changed"), Npc->GetStableId(), TEXT("player"), TEXT("This NPC currently refuses trade with the player."), 0.8f);
		Debug.ToolResult = TEXT("Accepted");
		return;
	}

	AActor* Target = nullptr;
	if (Step.TargetId == TEXT("player"))
	{
		Target = UGameplayStatics::GetPlayerPawn(this, 0);
	}
	else if (!Step.TargetId.IsEmpty())
	{
		Target = FindSandboxNpc(FName(*Step.TargetId));
	}
	if (!IsValid(Target))
	{
		Debug.ToolResult = TEXT("InvalidTarget");
		return;
	}
	EZLSocialActionType Action = EZLSocialActionType::Stop;
	if (Capability.CapabilityId == TEXT("face_player")) { Action = EZLSocialActionType::Face; }
	else if (Capability.CapabilityId == TEXT("keep_distance_from_player")) { Action = EZLSocialActionType::MoveAway; }
	else
	{
		Debug.ToolResult = TEXT("MissingHandler");
		return;
	}
	const FName TargetId = FName(*Step.TargetId);
	DispatchNpcActionObservation(Npc, Action, EZLSocialActionPhase::Started, TargetId);
	TWeakObjectPtr<UZLSocialSandboxDecisionComponent> WeakThis(this);
	TWeakObjectPtr<AZLSocialSandboxNpc> WeakNpc(Npc);
	if (!Npc->StartDecisionAction(Action, Target, [WeakThis, WeakNpc, Action, TargetId]()
	{
		if (WeakThis.IsValid() && WeakNpc.IsValid())
		{
			WeakThis->DispatchNpcActionObservation(WeakNpc.Get(), Action, EZLSocialActionPhase::Completed, TargetId);
			WeakNpc->ShowDecisionAction(Action, EZLSocialActionPhase::Completed);
			WeakThis->RecordNpcSocialFact(WeakNpc->GetStableId(), TEXT("executed_action"), WeakNpc->GetStableId(), TargetId, TEXT("This NPC completed an approved social action."), 0.45f);
			WeakThis->RefreshInspector();
		}
	}))
	{
		Debug.ToolResult = TEXT("HandlerRejected");
		return;
	}
	Debug.ToolResult = TEXT("Accepted");
	Npc->ShowDecisionAction(Action, EZLSocialActionPhase::Started);
}

void UZLSocialSandboxDecisionComponent::HandleNpcDecision(
	AZLSocialSandboxNpc* Npc,
	const FZLDecisionResponse& Response,
	const double SentAtSeconds)
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
	if (!IsValid(Npc))
	{
		return;
	}
	DecisionComponent->MultiNpcDecision.MarkCompleted(Npc->GetStableId());
	FZLSocialSandboxDecisionDebug& Debug = DecisionComponent->NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.bInFlight = false;
	Debug.Provider = Response.Provider.Left(32);
	Debug.Intent = Response.Intent.Left(32);
	Debug.StateVersion = Response.StateVersion;
	Debug.LatencyMs = FMath::Clamp(FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0), 0, 60000);
	Debug.bSpeechAccepted = Response.bHasSpeech;
	if (Response.Intent.Equals(TEXT("engage"), ESearchCase::IgnoreCase))
	{
		ApplyNpcConflict(Npc, EZLSocialSandboxConflictEvent::PlannerEngage);
	}
	else if (Response.Intent.Equals(TEXT("disengage"), ESearchCase::IgnoreCase)
		|| Response.Intent.Equals(TEXT("respond"), ESearchCase::IgnoreCase))
	{
		ApplyNpcConflict(Npc, EZLSocialSandboxConflictEvent::PlannerDisengage);
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
		TArray<FZLSocialSandboxPublicHistoryFact>& History = DecisionComponent->NpcPublicHistory.FindOrAdd(Npc->GetStableId());
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
	Debug.bPending = DecisionComponent->MultiNpcDecision.HasPending(Npc->GetStableId());
	TryDispatchNpcDecisions();
	RefreshInspector();
}

void UZLSocialSandboxDecisionComponent::ExecuteNpcTool(
	AZLSocialSandboxNpc* Npc,
	const FZLDecisionResponse& Response)
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
	if (!IsValid(Npc) || GetWorld() == nullptr)
	{
		return;
	}
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	const double NowSeconds = GetWorld()->GetTimeSeconds();
	TArray<double>& ExecutionTimes = DecisionComponent->NpcExecutionTimes.FindOrAdd(Npc->GetStableId());
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
	const FZLSocialToolValidationResult Validation = DecisionComponent->ToolRegistry.ValidateAndCommit(Call, Context);
	FZLSocialSandboxDecisionDebug& Debug = DecisionComponent->NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
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
	TWeakObjectPtr<UZLSocialSandboxDecisionComponent> WeakThis(this);
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

void UZLSocialSandboxDecisionComponent::HandleNpcDecisionFailure(
	AZLSocialSandboxNpc* Npc,
	const FZLServiceError& Error,
	const double SentAtSeconds)
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
	if (!IsValid(Npc))
	{
		return;
	}
	DecisionComponent->MultiNpcDecision.MarkCompleted(Npc->GetStableId());
	FZLSocialSandboxDecisionDebug& Debug = DecisionComponent->NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	Debug.bInFlight = false;
	Debug.Provider = TEXT("local");
	Debug.Intent = TEXT("hold");
	Debug.ToolResult = Error.Code.IsEmpty() ? TEXT("DecisionUnavailable") : FName(*Error.Code.Left(64));
	Debug.LatencyMs = FMath::Clamp(FMath::RoundToInt((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0), 0, 60000);
	Debug.bLocalFallback = true;
	ApplyNpcConflict(Npc, EZLSocialSandboxConflictEvent::LocalFallback, true);
	Npc->StopDecisionAction();
	Npc->ShowDecisionFallback();
	TryDispatchNpcDecisions();
	RefreshInspector();
}

void UZLSocialSandboxDecisionComponent::RecordNpcSocialFact(
	const FName NpcId,
	FString Kind,
	const FName SubjectId,
	const FName TargetId,
	FString Summary,
	const float Salience)
{
	if (NpcId.IsNone() || Kind.IsEmpty() || SubjectId.IsNone() || Summary.TrimStartAndEnd().IsEmpty() || GetWorld() == nullptr)
	{
		return;
	}
	FZLDecisionV2SocialFact Fact;
	Fact.Kind = MoveTemp(Kind);
	Fact.SubjectId = SubjectId.ToString();
	Fact.TargetId = TargetId.ToString();
	Fact.Summary = Summary.TrimStartAndEnd().Left(256);
	Fact.OccurredAtMs = FMath::Max<int64>(0, FMath::RoundToInt64(GetWorld()->GetTimeSeconds() * 1000.0));
	Fact.Salience = FMath::Clamp(Salience, 0.0f, 1.0f);
	TArray<FZLDecisionV2SocialFact>& Facts = NpcSocialFacts.FindOrAdd(NpcId);
	const bool bDuplicate = Facts.ContainsByPredicate([&Fact](const FZLDecisionV2SocialFact& Existing)
	{
		return Existing.Kind == Fact.Kind && Existing.SubjectId == Fact.SubjectId && Existing.TargetId == Fact.TargetId
			&& Existing.Summary == Fact.Summary && Fact.OccurredAtMs - Existing.OccurredAtMs < 1000;
	});
	if (!bDuplicate)
	{
		Facts.Add(MoveTemp(Fact));
	}
	while (Facts.Num() > 12)
	{
		Facts.RemoveAt(0, 1, EAllowShrinking::No);
	}
}

void UZLSocialSandboxDecisionComponent::ApplyNpcConflict(
	AZLSocialSandboxNpc* Npc,
	const EZLSocialSandboxConflictEvent Event,
	const bool bLocalFallback)
{
	UZLSocialSandboxDecisionComponent* DecisionComponent = this;
	if (!IsValid(Npc))
	{
		return;
	}
	FZLSocialSandboxConflictState& ConflictState = DecisionComponent->NpcConflictStates.FindOrAdd(Npc->GetStableId());
	FZLSocialSandboxDecisionDebug& Debug = DecisionComponent->NpcDecisionDebug.FindOrAdd(Npc->GetStableId());
	const FZLSocialSandboxConflictTransition Transition = ConflictState.Apply(Event);
	Npc->SetDefending(Transition.bShouldDefend);
	if (Transition.bChanged)
	{
		Npc->AdvanceAuthorityStateVersion();
	}
	Debug.ConflictLevel = FZLSocialSandboxConflictState::LevelName(Transition.Current);
	Debug.bLocalFallback = bLocalFallback;
}
