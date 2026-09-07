#include "SocialSandbox/Systems/ZLSocialSandboxWorldKnowledgeSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPawn.h"
#include "SocialSandbox/World/ZLSocialSandboxGameMode.h"

void UZLSocialSandboxWorldKnowledgeSubsystem::Reset()
{
	KnowledgeStore = FZLSocialKnowledgeStore();
	LastFact = FZLSocialWorldFact();
}

void UZLSocialSandboxWorldKnowledgeSubsystem::Trigger(AZLSocialSandboxGameMode& Owner, const FName EventType)
{
	const double Now = GetWorld() == nullptr ? 0.0 : GetWorld()->GetTimeSeconds();
	FZLSocialWorldFact Fact;
	Fact.FactId = EventType == TEXT("curfew") ? TEXT("active_curfew") : EventType == TEXT("hazard") ? TEXT("market_hazard") : TEXT("faction_alert");
	Fact.Summary = EventType == TEXT("curfew") ? TEXT("The district is under a confirmed curfew.") : EventType == TEXT("hazard") ? TEXT("A confirmed hazard has been reported near the market.") : TEXT("A confirmed faction alert is active in the district.");
	Fact.CauseEventId = FGuid::NewGuid();
	Fact.ConfirmedAtSeconds = Now;
	Fact.ExpiresAtSeconds = Now + 90.0;
	if (!KnowledgeStore.RecordWorldFact(Fact, Now)) return;
	LastFact = Fact;
	AZLSocialSandboxPawn* Player = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(&Owner, 0));
	for (AZLSocialSandboxNpc* Npc : Owner.GetSandboxNpcs())
	{
		if (IsValid(Npc) && (!Player || FVector::Dist2D(Player->GetActorLocation(), Npc->GetActorLocation()) <= Owner.GetObservationSettings().ShoutRange))
		{
			Learn(Owner, Npc, Fact, EZLSocialKnowledgeSource::DirectPerception, 1.0f, TEXT("personally observed the controlled world event"));
			Npc->ShowDecisionSpeech(EventType == TEXT("curfew") ? TEXT("宵禁已经生效，我得调整自己的安排。") : TEXT("我知道这件事，但会谨慎判断。"), TEXT("RulePlaceholder"));
		}
	}
	Owner.AppendInteractionRecord(FText::FromString(TEXT("世界事件已触发；只有在感知范围内的 NPC 获得确认知识。")), FLinearColor(0.95f, 0.7f, 0.2f));
	Owner.RefreshInspector();
}

void UZLSocialSandboxWorldKnowledgeSubsystem::Report(AZLSocialSandboxGameMode& Owner, const FName ReporterId, const FName ReceiverId)
{
	const double Now = GetWorld() == nullptr ? 0.0 : GetWorld()->GetTimeSeconds();
	AZLSocialSandboxNpc* Receiver = Owner.FindSandboxNpc(ReceiverId);
	const FZLSocialKnowledgeItem* Source = KnowledgeStore.Find(ReporterId, LastFact.FactId);
	if (!Receiver || !Source || !LastFact.IsValid(Now)) return;
	Learn(Owner, Receiver, LastFact, EZLSocialKnowledgeSource::ConfirmedReport, FMath::Min(Source->Confidence, 0.8f), FString::Printf(TEXT("confirmed report from %s"), *ReporterId.ToString()));
	Receiver->ShowDecisionSpeech(TEXT("我收到了确认报告，但会保留来源和可信度。"), TEXT("RulePlaceholder"));
	Owner.RefreshInspector();
}

void UZLSocialSandboxWorldKnowledgeSubsystem::SpreadRumor(AZLSocialSandboxGameMode& Owner, const FName ReporterId, const FName ReceiverId)
{
	const double Now = GetWorld() == nullptr ? 0.0 : GetWorld()->GetTimeSeconds();
	AZLSocialSandboxNpc* Receiver = Owner.FindSandboxNpc(ReceiverId);
	const FZLSocialKnowledgeItem* Source = KnowledgeStore.Find(ReporterId, LastFact.FactId);
	if (!Receiver || !Source || Source->Source == EZLSocialKnowledgeSource::BoundedPropagation || !LastFact.IsValid(Now)) return;
	Learn(Owner, Receiver, LastFact, EZLSocialKnowledgeSource::BoundedPropagation, FMath::Min(Source->Confidence * 0.6f, 0.5f), FString::Printf(TEXT("single-hop rumor from %s"), *ReporterId.ToString()));
	Receiver->ShowDecisionSpeech(TEXT("我听到了传闻，但还不能把它当成确定事实。"), TEXT("RulePlaceholder"));
	Owner.RefreshInspector();
}

void UZLSocialSandboxWorldKnowledgeSubsystem::Learn(AZLSocialSandboxGameMode& Owner, AZLSocialSandboxNpc* Npc, const FZLSocialWorldFact& Fact, const EZLSocialKnowledgeSource Source, const float Confidence, const FString& Reason)
{
	if (!IsValid(Npc) || GetWorld() == nullptr) return;
	FZLSocialKnowledgeItem Item;
	Item.FactId = Fact.FactId;
	Item.Summary = Fact.Summary;
	Item.Source = Source;
	Item.SourceAgentId = TEXT("world_authority");
	Item.Confidence = Confidence;
	Item.LearnedAtSeconds = GetWorld()->GetTimeSeconds();
	Item.ExpiresAtSeconds = Fact.ExpiresAtSeconds;
	Item.UpdateReason = Reason;
	KnowledgeStore.Learn(Npc->GetStableId(), Item, Item.LearnedAtSeconds);
}
