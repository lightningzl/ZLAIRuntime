#include "ZLSocialWorldContextJson.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace ZLSocialWorldContextJsonPrivate
{
	constexpr int32 SchemaVersion = 1;

	bool Fail(FString& OutError, const TCHAR* Message) { OutError = Message; return false; }

	bool HasOnlyFields(const TSharedPtr<FJsonObject>& Object, const TSet<FString>& AllowedFields, FString& OutError)
	{
		if (!Object.IsValid()) { return Fail(OutError, TEXT("JSON object is missing")); }
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
		{
			if (!AllowedFields.Contains(Pair.Key)) { return Fail(OutError, TEXT("JSON contains an unknown field")); }
		}
		return true;
	}

	bool ReadObjectArray(const TSharedPtr<FJsonObject>& Root, const TCHAR* Field, const TArray<TSharedPtr<FJsonValue>>*& OutValues, FString& OutError)
	{
		if (!Root->TryGetArrayField(Field, OutValues) || !OutValues) { return Fail(OutError, TEXT("JSON array field is missing or invalid")); }
		return true;
	}

	TSharedPtr<FJsonObject> ToJson(const FZLSocialWorldRule& Rule)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("stable_id"), Rule.StableId.ToString());
		Object->SetStringField(TEXT("summary"), Rule.Summary);
		return Object;
	}

	TSharedPtr<FJsonObject> ToJson(const FZLSocialFactionBackground& Faction)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("faction_id"), Faction.FactionId.ToString());
		Object->SetStringField(TEXT("display_name"), Faction.DisplayName);
		Object->SetStringField(TEXT("background_summary"), Faction.BackgroundSummary);
		return Object;
	}

	TSharedPtr<FJsonObject> ToJson(const FZLSocialPublicKnowledgeSeed& Knowledge)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("stable_id"), Knowledge.StableId.ToString());
		Object->SetStringField(TEXT("summary"), Knowledge.Summary);
		return Object;
	}

	template <typename ItemType>
	TArray<TSharedPtr<FJsonValue>> ToJsonArray(const TArray<ItemType>& Items)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		Values.Reserve(Items.Num());
		for (const ItemType& Item : Items) { Values.Add(MakeShared<FJsonValueObject>(ToJson(Item))); }
		return Values;
	}

	bool ReadRule(const TSharedPtr<FJsonObject>& Object, FZLSocialWorldRule& OutRule, FString& OutError)
	{
		static const TSet<FString> Fields = {TEXT("stable_id"), TEXT("summary")};
		FString StableId;
		if (!HasOnlyFields(Object, Fields, OutError) || !Object->TryGetStringField(TEXT("stable_id"), StableId) || !Object->TryGetStringField(TEXT("summary"), OutRule.Summary)) { return Fail(OutError, TEXT("world rule is invalid")); }
		OutRule.StableId = FName(*StableId);
		return OutRule.IsValid() || Fail(OutError, TEXT("world rule is outside allowed bounds"));
	}

	bool ReadFaction(const TSharedPtr<FJsonObject>& Object, FZLSocialFactionBackground& OutFaction, FString& OutError)
	{
		static const TSet<FString> Fields = {TEXT("faction_id"), TEXT("display_name"), TEXT("background_summary")};
		FString FactionId;
		if (!HasOnlyFields(Object, Fields, OutError) || !Object->TryGetStringField(TEXT("faction_id"), FactionId) || !Object->TryGetStringField(TEXT("display_name"), OutFaction.DisplayName) || !Object->TryGetStringField(TEXT("background_summary"), OutFaction.BackgroundSummary)) { return Fail(OutError, TEXT("faction background is invalid")); }
		OutFaction.FactionId = FName(*FactionId);
		return OutFaction.IsValid() || Fail(OutError, TEXT("faction background is outside allowed bounds"));
	}

	bool ReadKnowledge(const TSharedPtr<FJsonObject>& Object, FZLSocialPublicKnowledgeSeed& OutKnowledge, FString& OutError)
	{
		static const TSet<FString> Fields = {TEXT("stable_id"), TEXT("summary")};
		FString StableId;
		if (!HasOnlyFields(Object, Fields, OutError) || !Object->TryGetStringField(TEXT("stable_id"), StableId) || !Object->TryGetStringField(TEXT("summary"), OutKnowledge.Summary)) { return Fail(OutError, TEXT("public knowledge is invalid")); }
		OutKnowledge.StableId = FName(*StableId);
		return OutKnowledge.IsValid() || Fail(OutError, TEXT("public knowledge is outside allowed bounds"));
	}

	template <typename ItemType>
	bool ReadArray(const TArray<TSharedPtr<FJsonValue>>* Values, TArray<ItemType>& OutItems, const TFunctionRef<bool(const TSharedPtr<FJsonObject>&, ItemType&, FString&)>& ReadItem, FString& OutError)
	{
		OutItems.Reset(Values->Num());
		for (const TSharedPtr<FJsonValue>& Value : *Values)
		{
			const TSharedPtr<FJsonObject>* Object = nullptr;
			ItemType Item;
			if (!Value.IsValid() || !Value->TryGetObject(Object) || !Object || !Object->IsValid() || !ReadItem(*Object, Item, OutError)) { return false; }
			OutItems.Add(MoveTemp(Item));
		}
		return true;
	}
}

using namespace ZLSocialWorldContextJsonPrivate;

bool FZLSocialWorldContextJsonCodec::Serialize(const FZLSocialWorldContextData& WorldContext, FString& OutJson, FString& OutError)
{
	if (!WorldContext.IsValid(&OutError)) { return false; }
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("schema_version"), SchemaVersion);
	Root->SetStringField(TEXT("stable_id"), WorldContext.StableId.ToString());
	Root->SetStringField(TEXT("display_name"), WorldContext.DisplayName);
	Root->SetArrayField(TEXT("rules"), ToJsonArray(WorldContext.Rules));
	Root->SetArrayField(TEXT("factions"), ToJsonArray(WorldContext.Factions));
	Root->SetArrayField(TEXT("public_knowledge"), ToJsonArray(WorldContext.PublicKnowledge));
	return FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&OutJson)) || Fail(OutError, TEXT("failed to serialize World Context JSON"));
}

bool FZLSocialWorldContextJsonCodec::Deserialize(const FString& Json, FZLSocialWorldContextData& OutWorldContext, FString& OutError)
{
	TSharedPtr<FJsonObject> Root;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid()) { return Fail(OutError, TEXT("JSON is not a valid World Context object")); }
	static const TSet<FString> Fields = {TEXT("schema_version"), TEXT("stable_id"), TEXT("display_name"), TEXT("rules"), TEXT("factions"), TEXT("public_knowledge")};
	int32 Version = 0;
	FString StableId;
	const TArray<TSharedPtr<FJsonValue>>* Rules = nullptr;
	const TArray<TSharedPtr<FJsonValue>>* Factions = nullptr;
	const TArray<TSharedPtr<FJsonValue>>* Knowledge = nullptr;
	FZLSocialWorldContextData Candidate;
	if (!HasOnlyFields(Root, Fields, OutError) || !Root->TryGetNumberField(TEXT("schema_version"), Version) || Version != SchemaVersion || !Root->TryGetStringField(TEXT("stable_id"), StableId) || !Root->TryGetStringField(TEXT("display_name"), Candidate.DisplayName) || !ReadObjectArray(Root, TEXT("rules"), Rules, OutError) || !ReadObjectArray(Root, TEXT("factions"), Factions, OutError) || !ReadObjectArray(Root, TEXT("public_knowledge"), Knowledge, OutError)) { return Fail(OutError, TEXT("World Context JSON has a missing or invalid required field")); }
	Candidate.StableId = FName(*StableId);
	if (!ReadArray<FZLSocialWorldRule>(Rules, Candidate.Rules, ReadRule, OutError) || !ReadArray<FZLSocialFactionBackground>(Factions, Candidate.Factions, ReadFaction, OutError) || !ReadArray<FZLSocialPublicKnowledgeSeed>(Knowledge, Candidate.PublicKnowledge, ReadKnowledge, OutError) || !Candidate.IsValid(&OutError)) { return false; }
	OutWorldContext = MoveTemp(Candidate);
	OutError.Reset();
	return true;
}
