#pragma once

#include "CoreMinimal.h"
#include "ZLSocialTypes.h"

struct ZLASOCIALRUNTIME_API FZLSocialPerceptionSettings
{
	float VisualThreshold = 0.15f;
	float AuditoryThreshold = 0.1f;
	float VisualOcclusionFactor = 0.0f;
};

struct ZLASOCIALRUNTIME_API FZLSocialPerceptionResult
{
	bool bPerceived = false;
	EZLSocialPerceptionChannel Channel = EZLSocialPerceptionChannel::None;
	float EffectiveIntensity = 0.0f;
};

class ZLASOCIALRUNTIME_API FZLSocialPerceptionFilter
{
public:
	explicit FZLSocialPerceptionFilter(const FZLSocialPerceptionSettings& InSettings = FZLSocialPerceptionSettings());

	FZLSocialPerceptionResult Evaluate(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Agent, double NowSeconds, const TFunctionRef<bool(const FVector&, const FVector&)> HasLineOfSight) const;

private:
	static float DistanceFalloff(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Agent);

	FZLSocialPerceptionSettings Settings;
};
