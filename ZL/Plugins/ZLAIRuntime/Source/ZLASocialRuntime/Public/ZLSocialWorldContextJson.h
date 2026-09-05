#pragma once

#include "CoreMinimal.h"
#include "ZLSocialWorldContext.h"

class ZLASOCIALRUNTIME_API FZLSocialWorldContextJsonCodec
{
public:
	static bool Serialize(const FZLSocialWorldContextData& WorldContext, FString& OutJson, FString& OutError);
	static bool Deserialize(const FString& Json, FZLSocialWorldContextData& OutWorldContext, FString& OutError);
};
