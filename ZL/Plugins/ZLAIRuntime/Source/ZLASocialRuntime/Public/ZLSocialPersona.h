#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"

#include "ZLSocialPersona.generated.h"

namespace ZLSocialPersonaLimits
{
	inline constexpr int32 MaxStableIdLength = 64;
	inline constexpr int32 MaxDisplayNameLength = 64;
	inline constexpr int32 MaxBackgroundSummaryLength = 512;
	inline constexpr int32 MaxRoleLength = 128;
	inline constexpr int32 MaxListItems = 8;
	inline constexpr int32 MaxListItemLength = 128;
	inline constexpr int32 MaxSpeakingStyleLength = 256;
}

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonaRelationship
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Trust = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Affinity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Fear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Familiarity = 0.0f;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonaInstantState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Fear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Anger = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Curiosity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Alert = 0.0f;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonaData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName StableId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FString BackgroundSummary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Role;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality")
	TArray<FString> Personality;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality", meta = (MultiLine = true))
	FString SpeakingStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality")
	TArray<FString> Goals;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initial State")
	FZLSocialPersonaRelationship InitialRelationship;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initial State")
	FZLSocialPersonaInstantState InitialInstantState;

	bool IsValid(FString* OutError = nullptr) const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonaRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Persona")
	FZLSocialPersonaData Persona;

	bool IsValid(FString* OutError = nullptr) const { return Persona.IsValid(OutError); }
};

UCLASS(BlueprintType)
class ZLASOCIALRUNTIME_API UZLSocialPersonaAsset final : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Persona")
	FZLSocialPersonaData Persona;

	UPROPERTY(EditAnywhere, Category = "JSON")
	FFilePath ImportFile;

	UPROPERTY(EditAnywhere, Category = "JSON")
	FFilePath ExportFile;

	UPROPERTY(VisibleAnywhere, Transient, Category = "JSON")
	FString LastJsonOperationResult;

	UFUNCTION(CallInEditor, Category = "JSON")
	void ImportPersonaJson();

	UFUNCTION(CallInEditor, Category = "JSON")
	void ExportPersonaJson();

	/** Validates and applies a pasted single-Persona JSON document. */
	bool ImportPersonaJsonText(const FString& Json);

	bool IsValid(FString* OutError = nullptr) const { return Persona.IsValid(OutError); }
};
