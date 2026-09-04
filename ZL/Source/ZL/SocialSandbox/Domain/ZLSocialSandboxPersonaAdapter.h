#pragma once

#include "CoreMinimal.h"
#include "ZLSocialPersona.h"
#include "SocialSandbox/Domain/ZLSocialSandboxNpcProfile.h"

class FZLSocialSandboxPersonaAdapter
{
public:
	static bool ToNpcProfile(const FZLSocialPersonaData& Persona, const FLinearColor& BodyColor, FZLSocialSandboxNpcProfile& OutProfile, FString& OutError);
};
