#pragma once

#include "CoreMinimal.h"
#include "ZLSocialObservation.h"

enum class EZLSocialInputValidationResult : uint8
{
	Valid,
	Empty,
	TooLong,
	InEarRequiresTarget
};

class ZLASOCIALRUNTIME_API FZLSocialInputValidation
{
public:
	static int32 CountUnicodeCodePoints(const FString& Text);
	static EZLSocialInputValidationResult ValidateText(const FString& Text);
	static EZLSocialInputValidationResult ValidateSpeech(const FString& Text, EZLSocialSpeechMode Mode, FName ExplicitTargetId);
};
