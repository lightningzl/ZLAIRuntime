#include "ZLSocialTypes.h"

void FZLSocialPersonalityTraits::Clamp()
{
	Brave = FMath::Clamp(Brave, 0.0f, 1.0f);
	FearSensitivity = FMath::Clamp(FearSensitivity, 0.0f, 1.0f);
	Curiosity = FMath::Clamp(Curiosity, 0.0f, 1.0f);
	Justice = FMath::Clamp(Justice, 0.0f, 1.0f);
	Aggression = FMath::Clamp(Aggression, 0.0f, 1.0f);
	Social = FMath::Clamp(Social, 0.0f, 1.0f);
}

bool FZLSocialEvent::IsValid(const double NowSeconds) const
{
	return EventId.IsValid()
		&& Type.IsValid()
		&& Radius >= 0.0f
		&& Severity >= 0.0f && Severity <= 1.0f
		&& Noise >= 0.0f && Noise <= 1.0f
		&& Channels != 0
		&& ExpiresAtSeconds > CreatedAtSeconds
		&& NowSeconds <= ExpiresAtSeconds;
}

bool FZLSocialEvent::HasChannel(const EZLSocialPerceptionChannel Channel) const
{
	return (Channels & static_cast<int32>(Channel)) != 0;
}
