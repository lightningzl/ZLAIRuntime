#include "ZLSocialInputValidation.h"

int32 FZLSocialInputValidation::CountUnicodeCodePoints(const FString& Text)
{
	int32 Count = 0;
	for (int32 Index = 0; Index < Text.Len(); ++Index)
	{
		const uint32 CodeUnit = static_cast<uint32>(Text[Index]);
		if (CodeUnit >= 0xD800 && CodeUnit <= 0xDBFF && Index + 1 < Text.Len())
		{
			const uint32 Next = static_cast<uint32>(Text[Index + 1]);
			if (Next >= 0xDC00 && Next <= 0xDFFF) { ++Index; }
		}
		++Count;
	}
	return Count;
}

EZLSocialInputValidationResult FZLSocialInputValidation::ValidateText(const FString& Text)
{
	const FString Trimmed = Text.TrimStartAndEnd();
	if (Trimmed.IsEmpty()) { return EZLSocialInputValidationResult::Empty; }
	return CountUnicodeCodePoints(Trimmed) > 512 ? EZLSocialInputValidationResult::TooLong : EZLSocialInputValidationResult::Valid;
}

EZLSocialInputValidationResult FZLSocialInputValidation::ValidateSpeech(const FString& Text, const EZLSocialSpeechMode Mode, const FName ExplicitTargetId)
{
	const EZLSocialInputValidationResult TextResult = ValidateText(Text);
	if (TextResult != EZLSocialInputValidationResult::Valid) { return TextResult; }
	return Mode == EZLSocialSpeechMode::InEar && ExplicitTargetId.IsNone()
		? EZLSocialInputValidationResult::InEarRequiresTarget
		: EZLSocialInputValidationResult::Valid;
}
