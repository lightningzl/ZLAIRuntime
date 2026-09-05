#include "ZLSocialKnowledge.h"

namespace { const TArray<FZLSocialKnowledgeItem> EmptyKnowledge; }

bool FZLSocialWorldFact::IsValid(const double NowSeconds) const { return !FactId.IsNone() && !Summary.TrimStartAndEnd().IsEmpty() && CauseEventId.IsValid() && FMath::IsFinite(ConfirmedAtSeconds) && ExpiresAtSeconds > NowSeconds; }
bool FZLSocialKnowledgeItem::IsValid(const double NowSeconds) const { return !FactId.IsNone() && !Summary.TrimStartAndEnd().IsEmpty() && !SourceAgentId.IsNone() && FMath::IsFinite(Confidence) && FMath::IsWithinInclusive(Confidence, 0.0f, 1.0f) && ExpiresAtSeconds > NowSeconds; }

FZLSocialKnowledgeStore::FZLSocialKnowledgeStore(const int32 InCapacityPerAgent) : CapacityPerAgent(FMath::Clamp(InCapacityPerAgent, 1, 32)) {}

bool FZLSocialKnowledgeStore::RecordWorldFact(const FZLSocialWorldFact& Fact, const double NowSeconds)
{
	if (!Fact.IsValid(NowSeconds)) { return false; }
	WorldFacts.Add(Fact.FactId, Fact); return true;
}

bool FZLSocialKnowledgeStore::Learn(const FName AgentId, const FZLSocialKnowledgeItem& Item, const double NowSeconds)
{
	if (AgentId.IsNone() || !Item.IsValid(NowSeconds) || !WorldFacts.Contains(Item.FactId)) { return false; }
	TArray<FZLSocialKnowledgeItem>& Items = KnowledgeByAgent.FindOrAdd(AgentId);
	if (FZLSocialKnowledgeItem* Existing = Items.FindByPredicate([&Item](const FZLSocialKnowledgeItem& Value) { return Value.FactId == Item.FactId; })) { *Existing = Item; return true; }
	if (Items.Num() >= CapacityPerAgent) { Items.RemoveAt(0); }
	Items.Add(Item); return true;
}

bool FZLSocialKnowledgeStore::Rebut(const FName AgentId, const FName FactId, const FName SourceAgentId, const FString& Reason, const double NowSeconds)
{
	FZLSocialKnowledgeItem* Item = KnowledgeByAgent.FindOrAdd(AgentId).FindByPredicate([FactId](const FZLSocialKnowledgeItem& Value) { return Value.FactId == FactId; });
	if (!Item || SourceAgentId.IsNone() || Reason.TrimStartAndEnd().IsEmpty() || !Item->IsValid(NowSeconds)) { return false; }
	Item->Source = EZLSocialKnowledgeSource::Rebuttal; Item->SourceAgentId = SourceAgentId; Item->Confidence = FMath::Min(Item->Confidence, 0.25f); Item->UpdateReason = Reason; return true;
}

void FZLSocialKnowledgeStore::ForgetExpired(const double NowSeconds)
{
	for (auto It = WorldFacts.CreateIterator(); It; ++It) if (!It.Value().IsValid(NowSeconds)) It.RemoveCurrent();
	for (auto It = KnowledgeByAgent.CreateIterator(); It; ++It) { It.Value().RemoveAll([NowSeconds](const FZLSocialKnowledgeItem& Item) { return !Item.IsValid(NowSeconds); }); if (It.Value().IsEmpty()) It.RemoveCurrent(); }
}

const FZLSocialKnowledgeItem* FZLSocialKnowledgeStore::Find(const FName AgentId, const FName FactId) const { const TArray<FZLSocialKnowledgeItem>* Items = KnowledgeByAgent.Find(AgentId); return Items ? Items->FindByPredicate([FactId](const FZLSocialKnowledgeItem& Item) { return Item.FactId == FactId; }) : nullptr; }
const TArray<FZLSocialKnowledgeItem>& FZLSocialKnowledgeStore::GetForAgent(const FName AgentId) const { const TArray<FZLSocialKnowledgeItem>* Items = KnowledgeByAgent.Find(AgentId); return Items ? *Items : EmptyKnowledge; }
const FZLSocialWorldFact* FZLSocialKnowledgeStore::FindWorldFact(const FName FactId) const { return WorldFacts.Find(FactId); }
