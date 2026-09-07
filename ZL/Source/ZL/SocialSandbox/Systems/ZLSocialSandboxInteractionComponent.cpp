#include "SocialSandbox/Systems/ZLSocialSandboxInteractionComponent.h"

#include "Kismet/GameplayStatics.h"
#include "ZLSocialActionParser.h"
#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPawn.h"
#include "SocialSandbox/Domain/ZLSocialSandboxCombat.h"
#include "SocialSandbox/Systems/ZLSocialSandboxDecisionComponent.h"
#include "SocialSandbox/World/ZLSocialSandboxGameMode.h"

UZLSocialSandboxInteractionComponent::UZLSocialSandboxInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AZLSocialSandboxGameMode* UZLSocialSandboxInteractionComponent::GetSandboxGameMode() const
{
	return Cast<AZLSocialSandboxGameMode>(GetOwner());
}

void UZLSocialSandboxInteractionComponent::UpdateNpcDistanceBands()
{
	AZLSocialSandboxGameMode* GameMode = Cast<AZLSocialSandboxGameMode>(GetOwner());
	if (GameMode == nullptr || GameMode->GetWorld() == nullptr) return;
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(GameMode, 0));
	if (!IsValid(Player)) return;
	UZLSocialSandboxDecisionComponent* Decision = GameMode->GetDecisionComponent();
	if (Decision == nullptr) return;
	for (AZLSocialSandboxNpc* Npc : GameMode->GetSandboxNpcs())
	{
		if (!IsValid(Npc)) continue;
		const FName NpcId = Npc->GetStableId();
		const float Distance = FVector::Dist2D(Npc->GetActorLocation(), Player->GetActorLocation());
		const int32 NewBand = Distance < 250.0f ? 0 : (Distance < 800.0f ? 1 : 2);
		int32* PreviousBand = Decision->NpcDistanceBands.Find(NpcId);
		float* PreviousDistance = Decision->NpcLastDistances.Find(NpcId);
		if (PreviousBand == nullptr || PreviousDistance == nullptr)
		{
			Decision->NpcDistanceBands.Add(NpcId, NewBand);
			Decision->NpcLastDistances.Add(NpcId, Distance);
			continue;
		}
		if (*PreviousBand == NewBand) { *PreviousDistance = Distance; continue; }
		const bool bMovedNearer = Distance < *PreviousDistance;
		Decision->ApplyNpcConflict(Npc, bMovedNearer ? EZLSocialSandboxConflictEvent::DistanceNear : EZLSocialSandboxConflictEvent::DistanceFar);
		*PreviousBand = NewBand;
		*PreviousDistance = Distance;
		const double NowSeconds = GameMode->GetWorld()->GetTimeSeconds();
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
		const FZLSocialObservation Observation = FZLSocialObservationEvaluator(GameMode->GetObservationSettings()).ObserveAction(Event, Observer, NowSeconds);
		Npc->RecordObservation(Observation);
		if (Observation.bSaw)
		{
			Decision->QueueNpcDecision(Npc, Observation, FString(), bMovedNearer ? EZLSocialSandboxDecisionTriggerReason::DistanceNear : EZLSocialSandboxDecisionTriggerReason::DistanceFar, true);
		}
	}
}

FZLSocialObservation UZLSocialSandboxInteractionComponent::DispatchNpcActionObservation(AZLSocialSandboxNpc* Actor, const EZLSocialActionType Action, const EZLSocialActionPhase Phase, const FName TargetId)
{
	FZLSocialObservation SelfObservation;
	AZLSocialSandboxGameMode* GameMode = Cast<AZLSocialSandboxGameMode>(GetOwner());
	if (!IsValid(Actor) || GameMode == nullptr || GetWorld() == nullptr) return SelfObservation;
	const double NowSeconds = GetWorld()->GetTimeSeconds();
	FZLSocialActionEvent Event;
	Event.EventId = FGuid::NewGuid(); Event.Action = Action; Event.Phase = Phase; Event.ActorId = Actor->GetStableId(); Event.TargetId = TargetId;
	Event.Position = Actor->GetActorLocation(); Event.Forward = Actor->GetActorForwardVector(); Event.TimestampSeconds = NowSeconds;
	if (!Event.IsValid(NowSeconds)) return SelfObservation;
	SelfObservation.EventId = Event.EventId; SelfObservation.ObserverId = Actor->GetStableId(); SelfObservation.SourceId = Actor->GetStableId(); SelfObservation.Source = EZLSocialObservationSource::Action; SelfObservation.Action = Action; SelfObservation.ActionPhase = Phase; SelfObservation.ExplicitTargetId = TargetId; SelfObservation.TargetJudgment = TargetId.IsNone() ? EZLSocialTargetJudgment::Unresolved : EZLSocialTargetJudgment::ExplicitOther; SelfObservation.bSaw = true; SelfObservation.ObservedAtSeconds = NowSeconds;
	Actor->RecordObservation(SelfObservation);
	if (Phase == EZLSocialActionPhase::Started)
	{
		const TCHAR* ActionText = Action == EZLSocialActionType::Face ? TEXT("面向") : Action == EZLSocialActionType::Approach ? TEXT("靠近") : Action == EZLSocialActionType::MoveAway ? TEXT("远离") : TEXT("停止");
		GameMode->AppendInteractionRecord(FText::FromString(FString::Printf(TEXT("[%s · 行动] %s玩家"), *Actor->GetDisplayName().ToString(), ActionText)), FLinearColor(0.4f, 0.88f, 1.0f));
	}
	FZLSocialSandboxPublicHistoryFact Fact; Fact.Kind = TEXT("action_result"); Fact.SourceId = Actor->GetStableId(); Fact.TargetId = TargetId; Fact.Summary = TEXT("This NPC completed a visible social action."); Fact.OccurredAtSeconds = NowSeconds;
	TArray<FZLSocialSandboxPublicHistoryFact>& History = GameMode->GetDecisionComponent()->NpcPublicHistory.FindOrAdd(Actor->GetStableId()); History.Add(MoveTemp(Fact)); if (History.Num() > 16) History.RemoveAt(0, History.Num() - 16, EAllowShrinking::No);
	const FZLSocialObservationEvaluator Evaluator(GameMode->GetObservationSettings());
	for (AZLSocialSandboxNpc* Npc : GameMode->GetSandboxNpcs())
	{
		if (!IsValid(Npc) || Npc == Actor) continue;
		FZLSocialObserver Observer; Observer.AgentId = Npc->GetStableId(); Observer.Position = Npc->GetActorLocation(); Observer.Forward = Npc->GetPlanarForwardVector();
		const FZLSocialObservation Observation = Evaluator.ObserveAction(Event, Observer, NowSeconds); Npc->RecordObservation(Observation); Npc->ShowActionObservation(Observation);
		if (Phase == EZLSocialActionPhase::Completed && Observation.bSaw) GameMode->GetDecisionComponent()->QueueNpcDecision(Npc, Observation, FString(), EZLSocialSandboxDecisionTriggerReason::PlayerAction);
	}
	return SelfObservation;
}

FText UZLSocialSandboxInteractionComponent::SubmitSpeech(const FName SpeechMode, const FName TargetId, const FString& Text)
{
	AZLSocialSandboxGameMode* GameMode = Cast<AZLSocialSandboxGameMode>(GetOwner());
	if (GameMode == nullptr || GetWorld() == nullptr) return FText::FromString(TEXT("拒绝：交互场景不可用"));
	const TMap<FName, EZLSocialSpeechMode> Modes = {{TEXT("Whisper"), EZLSocialSpeechMode::Whisper}, {TEXT("Talk"), EZLSocialSpeechMode::Talk}, {TEXT("Shout"), EZLSocialSpeechMode::Shout}, {TEXT("InEar"), EZLSocialSpeechMode::InEar}};
	const EZLSocialSpeechMode* Mode = Modes.Find(SpeechMode);
	if (Mode == nullptr) return FText::FromString(TEXT("拒绝：未知说话模式"));
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(GameMode, 0));
	if (Player == nullptr) return FText::FromString(TEXT("拒绝：玩家角色不可用"));
	AZLSocialSandboxNpc* Target = TargetId.IsNone() ? nullptr : GameMode->FindSandboxNpc(TargetId);
	if (!TargetId.IsNone() && Target == nullptr) return FText::FromString(TEXT("拒绝：目标已失效"));
	FZLSocialObservationSettings Settings = GameMode->GetObservationSettings(); Settings.Clamp();
	if (*Mode == EZLSocialSpeechMode::InEar && (Target == nullptr || FVector::Dist2D(Player->GetActorLocation(), Target->GetActorLocation()) > Settings.InEarRange)) return FText::FromString(TEXT("拒绝：耳边说话目标必须在近距离内"));
	FZLSocialSpeechEvent Event; Event.EventId = FGuid::NewGuid(); Event.SpeakerId = TEXT("player"); Event.Text = Text; Event.Mode = *Mode; Event.ExplicitTargetId = TargetId; Event.Position = Player->GetActorLocation(); Event.Forward = Player->GetActorForwardVector(); Event.TimestampSeconds = GetWorld()->GetTimeSeconds(); Event.LifetimeSeconds = 4.0f;
	if (!Event.IsValid(Event.TimestampSeconds)) return FText::FromString(TEXT("拒绝：说话事件边界无效"));
	const FZLSocialObservationEvaluator Evaluator(Settings);
	for (AZLSocialSandboxNpc* Npc : GameMode->GetSandboxNpcs())
	{
		if (!IsValid(Npc)) continue;
		FZLSocialObserver Observer; Observer.AgentId = Npc->GetStableId(); Observer.Position = Npc->GetActorLocation(); Observer.Forward = Npc->GetPlanarForwardVector();
		const FZLSocialObservation Observation = Evaluator.ObserveSpeech(Event, Observer, Event.TimestampSeconds); Npc->RecordObservation(Observation);
		if (Observation.bHeard && (Observation.bHeardClearly || Observation.TargetJudgment == EZLSocialTargetJudgment::ExplicitSelf)) GameMode->GetDecisionComponent()->QueueNpcDecision(Npc, Observation, Event.Text, EZLSocialSandboxDecisionTriggerReason::Speech);
		else Npc->ShowRuleSpeech(Observation);
	}
	Player->ShowSpeechBubble(Text); GameMode->RefreshInspector();
	return FText::GetEmpty();
}

FText UZLSocialSandboxInteractionComponent::SubmitAction(const FName TargetId, const FString& Text)
{
	AZLSocialSandboxGameMode* GameMode = GetSandboxGameMode();
	if (GameMode == nullptr) return FText::FromString(TEXT("拒绝：交互场景不可用"));
	const FString NormalizedInput = Text.TrimStartAndEnd();
	if (NormalizedInput.Equals(TEXT("交易"), ESearchCase::IgnoreCase)
		|| NormalizedInput.Equals(TEXT("trade"), ESearchCase::IgnoreCase))
	{
		return SubmitTradeAttempt(TargetId);
	}
	const FZLSocialActionParseResult Parsed = FZLSocialActionParser::Parse(Text);
	if (!Parsed.bMatched) return FText::FromString(TEXT("拒绝：仅支持面向、靠近、远离和停止等受控行为"));
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(GameMode, 0));
	if (Player == nullptr) return FText::FromString(TEXT("拒绝：玩家角色不可用"));
	const bool bNeedsTarget = Parsed.Action != EZLSocialActionType::Stop;
	AZLSocialSandboxNpc* Target = TargetId.IsNone() ? nullptr : GameMode->FindSandboxNpc(TargetId);
	if (bNeedsTarget && Target == nullptr) return FText::FromString(TEXT("拒绝：该行为必须选择有效目标"));
	if (Target != nullptr && FVector::Dist2D(Player->GetActorLocation(), Target->GetActorLocation()) > 3000.0f)
	{
		return FText::FromString(TEXT("拒绝：目标超出行为执行范围"));
	}
	if (Parsed.Action == EZLSocialActionType::Face)
	{
		FVector Direction = Target->GetActorLocation() - Player->GetActorLocation();
		Direction.Z = 0.0f;
		if (Direction.IsNearlyZero()) return FText::FromString(TEXT("拒绝：目标方向无效"));
		DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Started, TargetId);
		Player->SetActorRotation(Direction.Rotation());
		if (Player->GetController() != nullptr) Player->GetController()->SetControlRotation(Direction.Rotation());
		DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Completed, TargetId);
		return FText::GetEmpty();
	}
	if (Parsed.Action == EZLSocialActionType::Stop)
	{
		DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Started, NAME_None);
		Player->StopScriptedAction();
		DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Completed, NAME_None);
		for (AZLSocialSandboxNpc* Npc : GameMode->GetSandboxNpcs())
		{
			GameMode->GetDecisionComponent()->ApplyNpcConflict(Npc, EZLSocialSandboxConflictEvent::PlayerStop);
		}
		return FText::GetEmpty();
	}
	TWeakObjectPtr<UZLSocialSandboxInteractionComponent> WeakThis(this);
	if (!Player->StartScriptedAction(Parsed.Action, Target, [WeakThis, Action = Parsed.Action, TargetId]()
	{
		if (WeakThis.IsValid()) WeakThis->DispatchActionObservation(Action, EZLSocialActionPhase::Completed, TargetId);
	}))
	{
		return FText::FromString(TEXT("拒绝：玩家当前无法执行该行为"));
	}
	DispatchActionObservation(Parsed.Action, EZLSocialActionPhase::Started, TargetId);
	return FText::GetEmpty();
}

FText UZLSocialSandboxInteractionComponent::SubmitTradeAttempt(const FName TargetId)
{
	AZLSocialSandboxGameMode* GameMode = GetSandboxGameMode();
	if (GameMode == nullptr || GetWorld() == nullptr) return FText::FromString(TEXT("拒绝：交互场景不可用"));
	AZLSocialSandboxNpc* Merchant = GameMode->FindSandboxNpc(TargetId);
	if (!IsValid(Merchant) || !Merchant->GetProfile().Role.Contains(TEXT("merchant"), ESearchCase::IgnoreCase))
	{
		return FText::FromString(TEXT("拒绝：交易尝试必须指定商人"));
	}
	const FString Stance = GameMode->GetDecisionComponent()->NpcTradeStances.FindRef(TargetId);
	const FString Result = Stance == TEXT("refused") ? TEXT("拒绝") : (Stance == TEXT("cautious") ? TEXT("暂缓") : TEXT("可交谈"));
	GameMode->GetDecisionComponent()->RecordNpcSocialFact(
		TargetId, TEXT("trade_attempt"), TEXT("player"), TargetId,
		FString::Printf(TEXT("The player attempted to trade; UE returned stance: %s."), *Result), 0.6f);
	FZLSocialObservation TradeObservation;
	TradeObservation.EventId = FGuid::NewGuid();
	TradeObservation.ObserverId = TargetId;
	TradeObservation.SourceId = TEXT("player");
	TradeObservation.Source = EZLSocialObservationSource::Action;
	TradeObservation.Action = EZLSocialActionType::Stop;
	TradeObservation.ActionPhase = EZLSocialActionPhase::Completed;
	TradeObservation.ExplicitTargetId = TargetId;
	TradeObservation.TargetJudgment = EZLSocialTargetJudgment::ExplicitSelf;
	TradeObservation.bSaw = true;
	TradeObservation.ObservedAtSeconds = GetWorld()->GetTimeSeconds();
	Merchant->RecordObservation(TradeObservation);
	GameMode->GetDecisionComponent()->QueueNpcDecision(Merchant, TradeObservation, FString(), EZLSocialSandboxDecisionTriggerReason::PlayerAction);
	GameMode->AppendInteractionRecord(
		FText::FromString(FString::Printf(TEXT("[%s · 交易] %s"), *Merchant->GetDisplayName().ToString(), *Result)),
		Stance == TEXT("refused") ? FLinearColor(1.0f, 0.38f, 0.3f) : FLinearColor(0.4f, 0.88f, 1.0f));
	GameMode->RefreshInspector();
	return FText::GetEmpty();
}

void UZLSocialSandboxInteractionComponent::DispatchActionObservation(const EZLSocialActionType Action, const EZLSocialActionPhase Phase, const FName TargetId)
{
	AZLSocialSandboxGameMode* GameMode = GetSandboxGameMode();
	if (GameMode == nullptr || GetWorld() == nullptr) return;
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(GameMode, 0));
	if (Player == nullptr) return;
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
	if (!Event.IsValid(NowSeconds)) return;
	const FZLSocialObservationEvaluator Evaluator(GameMode->GetObservationSettings());
	for (AZLSocialSandboxNpc* Npc : GameMode->GetSandboxNpcs())
	{
		if (!IsValid(Npc)) continue;
		FZLSocialObserver Observer;
		Observer.AgentId = Npc->GetStableId();
		Observer.Position = Npc->GetActorLocation();
		Observer.Forward = Npc->GetPlanarForwardVector();
		const FZLSocialObservation Observation = Evaluator.ObserveAction(Event, Observer, NowSeconds);
		Npc->RecordObservation(Observation);
		Npc->ShowActionObservation(Observation);
		if (Phase == EZLSocialActionPhase::Completed && Observation.bSaw)
		{
			GameMode->GetDecisionComponent()->QueueNpcDecision(
				Npc, Observation, FString(),
				Action == EZLSocialActionType::Attack ? EZLSocialSandboxDecisionTriggerReason::Hit : EZLSocialSandboxDecisionTriggerReason::PlayerAction,
				Action != EZLSocialActionType::Attack);
		}
	}
	const AZLSocialSandboxNpc* Target = TargetId.IsNone() ? nullptr : GameMode->FindSandboxNpc(TargetId);
	Player->ShowActionBubble(Action, Phase, Target == nullptr ? FText::GetEmpty() : Target->GetDisplayName());
	GameMode->RefreshInspector();
}

void UZLSocialSandboxInteractionComponent::ResolvePlayerAttackFromAnimNotify(AZLSocialSandboxPawn* Player, const FName DamageSourceBone)
{
	AZLSocialSandboxGameMode* GameMode = GetSandboxGameMode();
	if (!IsValid(Player) || GameMode == nullptr || GetWorld() == nullptr || !Player->ConsumePendingAttackHit()) return;
	const USkeletalMeshComponent* PlayerMesh = Player->GetMesh();
	const FVector TraceStart = PlayerMesh != nullptr && DamageSourceBone != NAME_None && PlayerMesh->DoesSocketExist(DamageSourceBone)
		? PlayerMesh->GetSocketLocation(DamageSourceBone)
		: Player->GetActorLocation() + FVector::UpVector * 80.0f;
	const FVector TraceEnd = TraceStart + Player->GetActorForwardVector() * 180.0f;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SocialSandboxAttack), false, Player);
	TArray<FHitResult> Hits;
	if (!GetWorld()->SweepMultiByChannel(Hits, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(45.0f), QueryParams)) return;
	AZLSocialSandboxNpc* Target = nullptr;
	for (const FHitResult& Hit : Hits)
	{
		Target = Cast<AZLSocialSandboxNpc>(Hit.GetActor());
		if (IsValid(Target)) break;
	}
	if (!IsValid(Target)) return;
	FZLSocialSandboxDamageResult DamageResult;
	const double NowSeconds = GetWorld()->GetTimeSeconds();
	if (!Target->ApplySandboxDamage(FZLSocialSandboxCombat::AttackDamage, NowSeconds, DamageResult))
	{
		Target->ShowDamageResult(DamageResult);
		return;
	}
	const FVector ImpulseDirection = (Target->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
	Target->ApplyDamage(DamageResult.AppliedDamage, Player, Target->GetActorLocation(), ImpulseDirection * 250.0f + FVector::UpVector * 300.0f);
	UZLSocialSandboxDecisionComponent* Decision = GameMode->GetDecisionComponent();
	Decision->ApplyNpcConflict(Target, EZLSocialSandboxConflictEvent::Attack);
	FZLSocialActionEvent HitEvent;
	HitEvent.EventId = FGuid::NewGuid();
	HitEvent.Action = EZLSocialActionType::Attack;
	HitEvent.Phase = EZLSocialActionPhase::Completed;
	HitEvent.ActorId = TEXT("player");
	HitEvent.TargetId = Target->GetStableId();
	HitEvent.Position = Player->GetActorLocation();
	HitEvent.Forward = Player->GetActorForwardVector();
	HitEvent.TimestampSeconds = NowSeconds;
	FZLSocialObserver TargetObserver;
	TargetObserver.AgentId = Target->GetStableId();
	TargetObserver.Position = Target->GetActorLocation();
	TargetObserver.Forward = Target->GetPlanarForwardVector();
	FZLSocialObservation HitObservation = FZLSocialObservationEvaluator(GameMode->GetObservationSettings()).ObserveAction(HitEvent, TargetObserver, NowSeconds);
	HitObservation.bSaw = true;
	HitObservation.VisualFilter = EZLSocialObservationFilterReason::None;
	HitObservation.TargetJudgment = EZLSocialTargetJudgment::ExplicitSelf;
	Target->RecordObservation(HitObservation);
	Decision->RecordNpcSocialFact(
		Target->GetStableId(), TEXT("received_harm"), TEXT("player"), Target->GetStableId(),
		TEXT("The player struck this NPC and caused confirmed damage."), 1.0f);
	Decision->QueueNpcDecision(Target, HitObservation, FString(), EZLSocialSandboxDecisionTriggerReason::Hit);
	Target->ShowDamageResult(DamageResult);
}
