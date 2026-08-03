#include "ZLSocialPerception.h"

FZLSocialPerceptionFilter::FZLSocialPerceptionFilter(const FZLSocialPerceptionSettings& InSettings)
	: Settings(InSettings)
{
	Settings.VisualThreshold = FMath::Clamp(Settings.VisualThreshold, 0.0f, 1.0f);
	Settings.AuditoryThreshold = FMath::Clamp(Settings.AuditoryThreshold, 0.0f, 1.0f);
	Settings.VisualOcclusionFactor = FMath::Clamp(Settings.VisualOcclusionFactor, 0.0f, 1.0f);
}

FZLSocialPerceptionResult FZLSocialPerceptionFilter::Evaluate(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Agent, const double NowSeconds, const TFunctionRef<bool(const FVector&, const FVector&)> HasLineOfSight) const
{
	FZLSocialPerceptionResult Result;
	if (!Event.IsValid(NowSeconds) || !Agent.IsValid())
	{
		return Result;
	}

	if (Event.HasChannel(EZLSocialPerceptionChannel::Direct) && Event.TargetId == Agent.AgentId)
	{
		Result.bPerceived = true;
		Result.Channel = EZLSocialPerceptionChannel::Direct;
		Result.EffectiveIntensity = Event.Severity;
		return Result;
	}

	const float Falloff = DistanceFalloff(Event, Agent);
	if (Falloff <= 0.0f)
	{
		return Result;
	}

	if (Event.HasChannel(EZLSocialPerceptionChannel::Visual) && Agent.bCanSee)
	{
		const bool bVisible = HasLineOfSight(Agent.Position, Event.Position);
		const float Occlusion = bVisible ? 1.0f : Settings.VisualOcclusionFactor;
		const float Intensity = Event.Severity * Falloff * Occlusion;
		if (Intensity >= Settings.VisualThreshold)
		{
			Result.bPerceived = true;
			Result.Channel = EZLSocialPerceptionChannel::Visual;
			Result.EffectiveIntensity = Intensity;
		}
	}

	if (Event.HasChannel(EZLSocialPerceptionChannel::Auditory) && Agent.bCanHear)
	{
		const float Intensity = Event.Noise * Falloff;
		if (Intensity >= Settings.AuditoryThreshold && Intensity > Result.EffectiveIntensity)
		{
			Result.bPerceived = true;
			Result.Channel = EZLSocialPerceptionChannel::Auditory;
			Result.EffectiveIntensity = Intensity;
		}
	}
	return Result;
}

float FZLSocialPerceptionFilter::DistanceFalloff(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Agent)
{
	if (Event.Radius <= 0.0f)
	{
		return FVector::DistSquared2D(Event.Position, Agent.Position) <= UE_KINDA_SMALL_NUMBER ? 1.0f : 0.0f;
	}
	const float NormalizedDistance = FVector::Dist2D(Event.Position, Agent.Position) / Event.Radius;
	return FMath::Clamp(1.0f - NormalizedDistance, 0.0f, 1.0f);
}
