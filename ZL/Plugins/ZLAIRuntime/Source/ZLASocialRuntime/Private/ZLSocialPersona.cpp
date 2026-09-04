#include "ZLSocialPersona.h"

namespace
{
	bool IsBoundedText(const FString& Value, const int32 MaxLength)
	{
		return !Value.TrimStartAndEnd().IsEmpty() && Value.Len() <= MaxLength;
	}

	bool AreBoundedItems(const TArray<FString>& Items)
	{
		if (Items.IsEmpty() || Items.Num() > ZLSocialPersonaLimits::MaxListItems)
		{
			return false;
		}

		return !Items.ContainsByPredicate([](const FString& Item)
		{
			return !IsBoundedText(Item, ZLSocialPersonaLimits::MaxListItemLength);
		});
	}

	bool IsUnitInterval(const float Value)
	{
		return FMath::IsFinite(Value) && FMath::IsWithinInclusive(Value, 0.0f, 1.0f);
	}
}

bool FZLSocialPersonaRelationship::IsValid() const
{
	return FMath::IsFinite(Trust) && FMath::IsWithinInclusive(Trust, -1.0f, 1.0f)
		&& FMath::IsFinite(Affinity) && FMath::IsWithinInclusive(Affinity, -1.0f, 1.0f)
		&& IsUnitInterval(Fear)
		&& IsUnitInterval(Familiarity);
}

bool FZLSocialPersonaInstantState::IsValid() const
{
	return IsUnitInterval(Fear) && IsUnitInterval(Anger) && IsUnitInterval(Curiosity) && IsUnitInterval(Alert);
}

bool FZLSocialPersonaData::IsValid(FString* OutError) const
{
	auto Fail = [OutError](const TCHAR* Message)
	{
		if (OutError)
		{
			*OutError = Message;
		}
		return false;
	};

	const FString StableIdString = StableId.ToString();
	if (!IsBoundedText(StableIdString, ZLSocialPersonaLimits::MaxStableIdLength))
	{
		return Fail(TEXT("stable_id must be non-empty and within its length limit"));
	}
	if (!IsBoundedText(DisplayName, ZLSocialPersonaLimits::MaxDisplayNameLength))
	{
		return Fail(TEXT("display_name must be non-empty and within its length limit"));
	}
	if (!IsBoundedText(BackgroundSummary, ZLSocialPersonaLimits::MaxBackgroundSummaryLength))
	{
		return Fail(TEXT("background_summary must be non-empty and within its length limit"));
	}
	if (!IsBoundedText(Role, ZLSocialPersonaLimits::MaxRoleLength))
	{
		return Fail(TEXT("role must be non-empty and within its length limit"));
	}
	if (!AreBoundedItems(Personality) || !IsBoundedText(SpeakingStyle, ZLSocialPersonaLimits::MaxSpeakingStyleLength) || !AreBoundedItems(Goals))
	{
		return Fail(TEXT("persona traits, speaking style, or goals are invalid"));
	}
	if (!InitialRelationship.IsValid() || !InitialInstantState.IsValid())
	{
		return Fail(TEXT("initial relationship or instant state is outside its allowed range"));
	}

	if (OutError)
	{
		OutError->Reset();
	}
	return true;
}
