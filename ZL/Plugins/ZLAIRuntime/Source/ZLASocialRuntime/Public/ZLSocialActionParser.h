#pragma once

#include "CoreMinimal.h"
#include "ZLSocialObservation.h"

struct ZLASOCIALRUNTIME_API FZLSocialActionParseResult
{
	bool bMatched = false;
	EZLSocialActionType Action = EZLSocialActionType::Stop;
	FName AliasId;
};

class ZLASOCIALRUNTIME_API FZLSocialActionParser
{
public:
	static FZLSocialActionParseResult Parse(const FString& Input);
};
