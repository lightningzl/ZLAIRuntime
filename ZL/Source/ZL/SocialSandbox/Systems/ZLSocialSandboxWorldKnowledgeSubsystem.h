#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ZLSocialKnowledge.h"
#include "ZLSocialSandboxWorldKnowledgeSubsystem.generated.h"

/** 世界子系统：保存权威确认的世界事实，以及每个 NPC 已学习的知识。 */
UCLASS()
class ZL_API UZLSocialSandboxWorldKnowledgeSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	FZLSocialKnowledgeStore& GetStore() { return KnowledgeStore; }
	const FZLSocialKnowledgeStore& GetStore() const { return KnowledgeStore; }
	FZLSocialWorldFact& GetLastFact() { return LastFact; }
	const FZLSocialWorldFact& GetLastFact() const { return LastFact; }
	void Reset();
	void Trigger(class AZLSocialSandboxGameMode& Owner, FName EventType);
	void Report(class AZLSocialSandboxGameMode& Owner, FName ReporterId, FName ReceiverId);
	void SpreadRumor(class AZLSocialSandboxGameMode& Owner, FName ReporterId, FName ReceiverId);

private:
	void Learn(class AZLSocialSandboxGameMode& Owner, class AZLSocialSandboxNpc* Npc, const FZLSocialWorldFact& Fact, EZLSocialKnowledgeSource Source, float Confidence, const FString& Reason);
	FZLSocialKnowledgeStore KnowledgeStore;
	FZLSocialWorldFact LastFact;
};
