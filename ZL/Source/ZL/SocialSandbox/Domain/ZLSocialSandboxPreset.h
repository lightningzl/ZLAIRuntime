#pragma once

#include "CoreMinimal.h"

#include "SocialSandbox/Domain/ZLSocialSandboxNpcProfile.h"

struct FZLSocialSandboxPlayerPreset
{
	FName StableId;
	FText DisplayName;
	FLinearColor BodyColor = FLinearColor::White;
	FTransform SpawnTransform = FTransform::Identity;
	float InitialHealth = 100.0f;

	bool IsValid() const;
};

struct FZLSocialSandboxNpcPreset
{
	FZLSocialSandboxNpcProfile Profile;
	FTransform SpawnTransform = FTransform::Identity;
	float InitialHealth = 100.0f;

	bool IsValid() const;
};

struct FZLSocialSandboxPreset
{
	static constexpr int32 SchemaVersion = 1;

	FString PresetId;
	FZLSocialSandboxPlayerPreset Player;
	TArray<FZLSocialSandboxNpcPreset> Npcs;

	bool IsValid() const;
};

class FZLSocialSandboxPresetCodec
{
public:
	static bool LoadNamedPreset(const FString& PresetName, FZLSocialSandboxPreset& OutPreset, FText& OutError);
	static bool LoadFromFile(const FString& FilePath, FZLSocialSandboxPreset& OutPreset, FText& OutError);
	static bool ExportToSaved(const FZLSocialSandboxPreset& Preset, FString& OutFilePath, FText& OutError);
	static FString GetPresetDirectory();

private:
	static bool IsSafePresetName(const FString& PresetName);
};
