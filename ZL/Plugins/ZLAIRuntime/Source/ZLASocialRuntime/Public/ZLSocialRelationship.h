#pragma once

#include "CoreMinimal.h"
#include "ZLSocialPerception.h"

struct ZLASOCIALRUNTIME_API FZLSocialRelationshipState
{
	float Trust = 0.0f;
	float Affinity = 0.0f;
	float Fear = 0.0f;
	float Familiarity = 0.0f;
	float Reputation = 0.0f;
	double LastUpdatedSeconds = 0.0;
	FGuid LastCausationEventId;

	void Clamp();
};

struct ZLASOCIALRUNTIME_API FZLSocialRelationshipDelta
{
	float Trust = 0.0f;
	float Affinity = 0.0f;
	float Fear = 0.0f;
	float Familiarity = 0.0f;
	float Reputation = 0.0f;
};

struct ZLASOCIALRUNTIME_API FZLSocialFactionStandingState
{
	float Standing = 0.0f;
	double LastUpdatedSeconds = 0.0;
	FGuid LastCausationEventId;
};

struct ZLASOCIALRUNTIME_API FZLSocialRelationshipKey
{
	FName ObserverId;
	FName SubjectId;

	bool operator==(const FZLSocialRelationshipKey& Other) const
	{
		return ObserverId == Other.ObserverId && SubjectId == Other.SubjectId;
	}
};

FORCEINLINE uint32 GetTypeHash(const FZLSocialRelationshipKey& Key)
{
	return HashCombine(GetTypeHash(Key.ObserverId), GetTypeHash(Key.SubjectId));
}

struct ZLASOCIALRUNTIME_API FZLSocialFactionStandingKey
{
	FName FactionId;
	FName SubjectId;

	bool operator==(const FZLSocialFactionStandingKey& Other) const
	{
		return FactionId == Other.FactionId && SubjectId == Other.SubjectId;
	}
};

FORCEINLINE uint32 GetTypeHash(const FZLSocialFactionStandingKey& Key)
{
	return HashCombine(GetTypeHash(Key.FactionId), GetTypeHash(Key.SubjectId));
}

class ZLASOCIALRUNTIME_API FZLSocialRelationshipStore
{
public:
	bool ApplyPersonalEvent(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Observer, const FZLSocialPerceptionResult& Perception, double NowSeconds, FZLSocialRelationshipDelta* OutDelta = nullptr);
	bool ConfirmFactionStanding(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Authority, const FZLSocialPerceptionResult& Perception, double NowSeconds);
	void DecayTowardsNeutral(float DeltaSeconds, float RatePerSecond = 0.002f);
	void Reset();

	const FZLSocialRelationshipState* FindRelationship(FName ObserverId, FName SubjectId) const;
	const FZLSocialFactionStandingState* FindFactionStanding(FName FactionId, FName SubjectId) const;
	int32 GetRelationshipEdgeCount() const { return Relationships.Num(); }
	int32 GetFactionStandingCount() const { return FactionStandings.Num(); }

private:
	static bool BuildDelta(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Observer, const FZLSocialPerceptionResult& Perception, FZLSocialRelationshipDelta& OutDelta);
	static float SourceWeight(EZLSocialPerceptionChannel Channel, float Confidence);

	TMap<FZLSocialRelationshipKey, FZLSocialRelationshipState> Relationships;
	TMap<FZLSocialFactionStandingKey, FZLSocialFactionStandingState> FactionStandings;
	TMap<FGuid, TSet<FName>> PersonalObserversByRoot;
	TMap<FGuid, TSet<FName>> UpdatedFactionsByRoot;
	TMap<FGuid, double> RootExpiryByRoot;
};
