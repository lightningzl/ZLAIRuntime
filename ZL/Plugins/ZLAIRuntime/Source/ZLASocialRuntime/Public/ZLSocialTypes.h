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
	Auditory = 1 << 2
};
ENUM_CLASS_FLAGS(EZLSocialPerceptionChannel);

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SourceId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TargetId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Position = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float Radius = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Severity = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float Noise = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum="/Script/ZLASocialRuntime.EZLSocialPerceptionChannel")) int32 Channels = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) double CreatedAtSeconds = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) double ExpiresAtSeconds = 0.0;

	bool IsValid(double NowSeconds) const;
	bool HasChannel(EZLSocialPerceptionChannel Channel) const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialAgentProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AgentId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Position = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag Faction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FZLSocialPersonalityTraits Personality;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanSee = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanHear = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanReport = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanAssist = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanConfront = true;

	bool IsValid() const { return !AgentId.IsNone(); }
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
