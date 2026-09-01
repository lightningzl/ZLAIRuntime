#pragma once

#include "CoreMinimal.h"

struct FZLSocialSandboxNpcProfile
{
	FName StableId;
	FText DisplayName;
	FString Role;
	TArray<FString> Personality;
	FString SpeakingStyle;
	TArray<FString> Goals;
	float Trust = 0.0f;
	float Affinity = 0.0f;
	float RelationshipFear = 0.0f;
	float Familiarity = 0.0f;
	float Fear = 0.0f;
	float Anger = 0.0f;
	float Curiosity = 0.0f;
	float Alert = 0.0f;
	FLinearColor BodyColor = FLinearColor::White;

	bool IsValid() const;
	static FZLSocialSandboxNpcProfile Create(FName StableId);
};
