#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ZLSocialTypes.generated.h"

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EZLSocialPerceptionChannel : uint8
{
	None = 0,
	Direct = 1 << 0,
	Visual = 1 << 1,
	Auditory = 1 << 2,
	Social = 1 << 3
};
ENUM_CLASS_FLAGS(EZLSocialPerceptionChannel);

UENUM(BlueprintType)
enum class EZLSocialAgentLevel : uint8
{
	Level1,
	Important
};

namespace ZLSocialEventLimits
{
	inline constexpr int32 MaxChainDepth = 2;
	inline constexpr int32 MaxChainBudget = 32;
	inline constexpr int32 DefaultChainBudget = 12;
	inline constexpr int32 MaxFanOut = 6;
}

namespace ZLSocialRuntimeLimits
{
	inline constexpr int32 MaxRegisteredAgents = 10000;
	inline constexpr int32 MaxTrackedRoots = 1024;
	inline constexpr int32 MaxRelationshipEdges = 4096;
	inline constexpr int32 MaxFactionStandings = 1024;
}

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonalityTraits
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Brave = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float FearSensitivity = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Curiosity = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Justice = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Aggression = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Social = 0.5f;

	void Clamp();
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid EventId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid RootEventId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid ParentEventId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid CausationId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SourceId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TargetId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName ReporterId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName SocialReceiverId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Position = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float Radius = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Severity = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Noise = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum="/Script/ZLASocialRuntime.EZLSocialPerceptionChannel")) int32 Channels = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 ChainDepth = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 ChainBudget = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0")) float Confidence = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAnchored = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) double CreatedAtSeconds = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) double ExpiresAtSeconds = 0.0;

	bool IsValid(double NowSeconds) const;
	bool HasChannel(EZLSocialPerceptionChannel Channel) const;
	bool IsRootEvent() const { return EventId.IsValid() && RootEventId == EventId && !ParentEventId.IsValid() && ChainDepth == 0; }
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialAgentProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AgentId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Position = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag Faction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName FactionId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName OccupationId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EZLSocialAgentLevel AgentLevel = EZLSocialAgentLevel::Level1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FZLSocialPersonalityTraits Personality;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanSee = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanHear = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanReport = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanAssist = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanConfront = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanReceiveReports = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasFactionAuthority = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", ClampMax="32")) int32 ShortMemoryCapacity = 6;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="24")) int32 LongMemoryCapacity = 0;

	bool IsValid() const { return !AgentId.IsNone(); }
	bool IsImportant() const { return AgentLevel == EZLSocialAgentLevel::Important; }
};

UCLASS(BlueprintType)
class ZLASOCIALRUNTIME_API UZLSocialEventArchetype : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag EventType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0")) float Radius = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0")) float Severity = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0")) float Noise = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum="/Script/ZLASocialRuntime.EZLSocialPerceptionChannel")) int32 Channels = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.01")) float LifetimeSeconds = 2.0f;
};

UCLASS(BlueprintType)
class ZLASOCIALRUNTIME_API UZLSocialPersonalityArchetype : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FZLSocialPersonalityTraits Traits;
};
