#pragma once

#include "CoreMinimal.h"
#include "ZLSocialPersona.h"

class ZLASOCIALRUNTIME_API FZLSocialPersonaJsonCodec
{
public:
	static bool Serialize(const FZLSocialPersonaData& Persona, FString& OutJson, FString& OutError);
	static bool Deserialize(const FString& Json, FZLSocialPersonaData& OutPersona, FString& OutError);
};
