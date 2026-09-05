#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ZLSocialWorldContext.generated.h"

namespace ZLSocialWorldContextLimits
{
	inline constexpr int32 MaxStableIdLength = 64;
	inline constexpr int32 MaxDisplayNameLength = 96;
	inline constexpr int32 MaxSummaryLength = 512;
	inline constexpr int32 MaxItems = 16;
}

/** A static rule that applies to the scene. It is not a runtime fact or NPC memory. */
USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialWorldRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context")
	FName StableId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (MultiLine = true))
	FString Summary;

	bool IsValid() const;
};

/** Public background for a named faction at the start of a scene. */
USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialFactionBackground
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context")
	FName FactionId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (MultiLine = true))
	FString BackgroundSummary;

	bool IsValid() const;
};

/** Static common knowledge available as a scene starting condition, never a dynamic broadcast. */
USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPublicKnowledgeSeed
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context")
	FName StableId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (MultiLine = true))
	FString Summary;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialWorldContextData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName StableId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context")
	TArray<FZLSocialWorldRule> Rules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context")
	TArray<FZLSocialFactionBackground> Factions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context")
	TArray<FZLSocialPublicKnowledgeSeed> PublicKnowledge;

	bool IsValid(FString* OutError = nullptr) const;
};

/** Editable static scene background. Dynamic events and NPC knowledge are deliberately excluded. */
UCLASS(BlueprintType)
class ZLASOCIALRUNTIME_API UZLSocialWorldContextAsset final : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context")
	FZLSocialWorldContextData WorldContext;

	UPROPERTY(EditAnywhere, Category = "JSON")
	FFilePath ImportFile;

	UPROPERTY(EditAnywhere, Category = "JSON")
	FFilePath ExportFile;

	UPROPERTY(VisibleAnywhere, Transient, Category = "JSON")
	FString LastJsonOperationResult;

	UFUNCTION(CallInEditor, Category = "JSON")
	void ImportWorldContextJson();

	UFUNCTION(CallInEditor, Category = "JSON")
	void ExportWorldContextJson();

	bool ImportWorldContextJsonText(const FString& Json);
	bool IsValid(FString* OutError = nullptr) const { return WorldContext.IsValid(OutError); }
};
