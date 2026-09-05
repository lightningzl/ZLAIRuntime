#pragma once

#include "CoreMinimal.h"

#include "ZLSocialKnowledge.generated.h"

UENUM(BlueprintType)
enum class EZLSocialKnowledgeSource : uint8 { DirectPerception, ConfirmedReport, BoundedPropagation, Rebuttal };

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialWorldFact
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FName FactId;
	UPROPERTY(BlueprintReadOnly) FString Summary;
	UPROPERTY(BlueprintReadOnly) FGuid CauseEventId;
	UPROPERTY(BlueprintReadOnly) double ConfirmedAtSeconds = 0.0;
	UPROPERTY(BlueprintReadOnly) double ExpiresAtSeconds = 0.0;
	bool IsValid(double NowSeconds) const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialKnowledgeItem
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FName FactId;
	UPROPERTY(BlueprintReadOnly) FString Summary;
	UPROPERTY(BlueprintReadOnly) EZLSocialKnowledgeSource Source = EZLSocialKnowledgeSource::DirectPerception;
	UPROPERTY(BlueprintReadOnly) FName SourceAgentId;
	UPROPERTY(BlueprintReadOnly) float Confidence = 0.0f;
	UPROPERTY(BlueprintReadOnly) double LearnedAtSeconds = 0.0;
	UPROPERTY(BlueprintReadOnly) double ExpiresAtSeconds = 0.0;
	UPROPERTY(BlueprintReadOnly) FString UpdateReason;
	bool IsValid(double NowSeconds) const;
};

class ZLASOCIALRUNTIME_API FZLSocialKnowledgeStore
{
public:
	explicit FZLSocialKnowledgeStore(int32 InCapacityPerAgent = 16);
	bool RecordWorldFact(const FZLSocialWorldFact& Fact, double NowSeconds);
	bool Learn(FName AgentId, const FZLSocialKnowledgeItem& Item, double NowSeconds);
	bool Rebut(FName AgentId, FName FactId, FName SourceAgentId, const FString& Reason, double NowSeconds);
	void ForgetExpired(double NowSeconds);
	const FZLSocialKnowledgeItem* Find(FName AgentId, FName FactId) const;
	const TArray<FZLSocialKnowledgeItem>& GetForAgent(FName AgentId) const;
	const FZLSocialWorldFact* FindWorldFact(FName FactId) const;

private:
	int32 CapacityPerAgent;
	TMap<FName, FZLSocialWorldFact> WorldFacts;
	TMap<FName, TArray<FZLSocialKnowledgeItem>> KnowledgeByAgent;
};
