#include "ZLSocialWorldContext.h"

#include "ZLSocialWorldContextJson.h"

#include "Misc/FileHelper.h"

namespace
{
	bool IsBoundedText(const FString& Value, const int32 MaxLength)
	{
		return !Value.TrimStartAndEnd().IsEmpty() && Value.Len() <= MaxLength;
	}

	bool IsBoundedId(const FName Value)
	{
		return IsBoundedText(Value.ToString(), ZLSocialWorldContextLimits::MaxStableIdLength);
	}

	template <typename ItemType>
	bool AreUniqueAndValid(const TArray<ItemType>& Items, const TFunctionRef<FName(const ItemType&)>& GetId)
	{
		if (Items.Num() > ZLSocialWorldContextLimits::MaxItems)
		{
			return false;
		}
		TSet<FName> Ids;
		for (const ItemType& Item : Items)
		{
			const FName Id = GetId(Item);
			if (!Item.IsValid() || Ids.Contains(Id))
			{
				return false;
			}
			Ids.Add(Id);
		}
		return true;
	}
}

bool FZLSocialWorldRule::IsValid() const
{
	return IsBoundedId(StableId) && IsBoundedText(Summary, ZLSocialWorldContextLimits::MaxSummaryLength);
}

bool FZLSocialFactionBackground::IsValid() const
{
	return IsBoundedId(FactionId)
		&& IsBoundedText(DisplayName, ZLSocialWorldContextLimits::MaxDisplayNameLength)
		&& IsBoundedText(BackgroundSummary, ZLSocialWorldContextLimits::MaxSummaryLength);
}

bool FZLSocialPublicKnowledgeSeed::IsValid() const
{
	return IsBoundedId(StableId) && IsBoundedText(Summary, ZLSocialWorldContextLimits::MaxSummaryLength);
}

bool FZLSocialWorldContextData::IsValid(FString* OutError) const
{
	auto Fail = [OutError](const TCHAR* Message)
	{
		if (OutError) { *OutError = Message; }
		return false;
	};
	if (!IsBoundedId(StableId) || !IsBoundedText(DisplayName, ZLSocialWorldContextLimits::MaxDisplayNameLength))
	{
		return Fail(TEXT("world context identity is invalid"));
	}
	if (!AreUniqueAndValid<FZLSocialWorldRule>(Rules, [](const FZLSocialWorldRule& Rule) { return Rule.StableId; }))
	{
		return Fail(TEXT("world rules must be valid, unique, and within the item limit"));
	}
	if (!AreUniqueAndValid<FZLSocialFactionBackground>(Factions, [](const FZLSocialFactionBackground& Faction) { return Faction.FactionId; }))
	{
		return Fail(TEXT("factions must be valid, unique, and within the item limit"));
	}
	if (!AreUniqueAndValid<FZLSocialPublicKnowledgeSeed>(PublicKnowledge, [](const FZLSocialPublicKnowledgeSeed& Knowledge) { return Knowledge.StableId; }))
	{
		return Fail(TEXT("public knowledge must be valid, unique, and within the item limit"));
	}
	if (OutError) { OutError->Reset(); }
	return true;
}

void UZLSocialWorldContextAsset::ImportWorldContextJson()
{
	FString Json;
	if (ImportFile.FilePath.IsEmpty() || !FFileHelper::LoadFileToString(Json, *ImportFile.FilePath))
	{
		LastJsonOperationResult = TEXT("World Context JSON file could not be read");
		return;
	}
	ImportWorldContextJsonText(Json);
}

bool UZLSocialWorldContextAsset::ImportWorldContextJsonText(const FString& Json)
{
	FString Error;
	FZLSocialWorldContextData Candidate;
	if (!FZLSocialWorldContextJsonCodec::Deserialize(Json, Candidate, Error))
	{
		LastJsonOperationResult = Error.IsEmpty() ? TEXT("World Context JSON import failed") : Error;
		return false;
	}
	Modify();
	WorldContext = MoveTemp(Candidate);
	MarkPackageDirty();
	LastJsonOperationResult = TEXT("World Context JSON import succeeded");
	return true;
}

void UZLSocialWorldContextAsset::ExportWorldContextJson()
{
	FString Json;
	FString Error;
	if (ExportFile.FilePath.IsEmpty() || !FZLSocialWorldContextJsonCodec::Serialize(WorldContext, Json, Error) || !FFileHelper::SaveStringToFile(Json, *ExportFile.FilePath))
	{
		LastJsonOperationResult = Error.IsEmpty() ? TEXT("World Context JSON export failed") : Error;
		return;
	}
	LastJsonOperationResult = TEXT("World Context JSON export succeeded");
}
