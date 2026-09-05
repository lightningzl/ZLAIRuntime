#include "ZLSocialPersonaJson.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	constexpr int32 PersonaSchemaVersion = 1;

	bool Fail(FString& OutError, const TCHAR* Message)
	{
		OutError = Message;
		return false;
	}

	bool HasOnlyFields(const TSharedPtr<FJsonObject>& Object, const TSet<FString>& AllowedFields, FString& OutError)
	{
		if (!Object.IsValid())
		{
			return Fail(OutError, TEXT("JSON object is missing"));
		}
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
		{
			if (!AllowedFields.Contains(Pair.Key))
			{
				return Fail(OutError, TEXT("JSON contains an unknown field"));
			}
		}
		return true;
	}

	bool ReadStringArray(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, TArray<FString>& OutValues, FString& OutError)
	{
		const TArray<TSharedPtr<FJsonValue>>* JsonValues = nullptr;
		if (!Object->TryGetArrayField(FieldName, JsonValues) || !JsonValues)
		{
			return Fail(OutError, TEXT("JSON array field is missing or invalid"));
		}
		OutValues.Reset(JsonValues->Num());
		for (const TSharedPtr<FJsonValue>& JsonValue : *JsonValues)
		{
			FString Value;
			if (!JsonValue.IsValid() || !JsonValue->TryGetString(Value))
			{
				return Fail(OutError, TEXT("JSON array values must be strings"));
			}
			OutValues.Add(MoveTemp(Value));
		}
		return true;
	}

	TArray<TSharedPtr<FJsonValue>> ToJsonStringArray(const TArray<FString>& Values)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		JsonValues.Reserve(Values.Num());
		for (const FString& Value : Values)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		return JsonValues;
	}

	TSharedPtr<FJsonObject> ToJsonRelationship(const FZLSocialPersonaRelationship& Relationship)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("trust"), Relationship.Trust);
		Object->SetNumberField(TEXT("affinity"), Relationship.Affinity);
		Object->SetNumberField(TEXT("fear"), Relationship.Fear);
		Object->SetNumberField(TEXT("familiarity"), Relationship.Familiarity);
		return Object;
	}

	TSharedPtr<FJsonObject> ToJsonInstantState(const FZLSocialPersonaInstantState& State)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("fear"), State.Fear);
		Object->SetNumberField(TEXT("anger"), State.Anger);
		Object->SetNumberField(TEXT("curiosity"), State.Curiosity);
		Object->SetNumberField(TEXT("alert"), State.Alert);
		return Object;
	}

	bool ReadRelationship(const TSharedPtr<FJsonObject>& Object, FZLSocialPersonaRelationship& OutRelationship, FString& OutError)
	{
		static const TSet<FString> Fields = {TEXT("trust"), TEXT("affinity"), TEXT("fear"), TEXT("familiarity")};
		return HasOnlyFields(Object, Fields, OutError)
			&& Object->TryGetNumberField(TEXT("trust"), OutRelationship.Trust)
			&& Object->TryGetNumberField(TEXT("affinity"), OutRelationship.Affinity)
			&& Object->TryGetNumberField(TEXT("fear"), OutRelationship.Fear)
			&& Object->TryGetNumberField(TEXT("familiarity"), OutRelationship.Familiarity)
			? true : Fail(OutError, TEXT("initial_relationship is invalid"));
	}

	bool ReadInstantState(const TSharedPtr<FJsonObject>& Object, FZLSocialPersonaInstantState& OutState, FString& OutError)
	{
		static const TSet<FString> Fields = {TEXT("fear"), TEXT("anger"), TEXT("curiosity"), TEXT("alert")};
		return HasOnlyFields(Object, Fields, OutError)
			&& Object->TryGetNumberField(TEXT("fear"), OutState.Fear)
			&& Object->TryGetNumberField(TEXT("anger"), OutState.Anger)
			&& Object->TryGetNumberField(TEXT("curiosity"), OutState.Curiosity)
			&& Object->TryGetNumberField(TEXT("alert"), OutState.Alert)
			? true : Fail(OutError, TEXT("initial_instant_state is invalid"));
	}
}

bool FZLSocialPersonaJsonCodec::Serialize(const FZLSocialPersonaData& Persona, FString& OutJson, FString& OutError)
{
	if (!Persona.IsValid(&OutError))
	{
		return false;
	}

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("schema_version"), PersonaSchemaVersion);
	Root->SetStringField(TEXT("stable_id"), Persona.StableId.ToString());
	Root->SetStringField(TEXT("display_name"), Persona.DisplayName);
	Root->SetStringField(TEXT("background_summary"), Persona.BackgroundSummary);
	Root->SetStringField(TEXT("role"), Persona.Role);
	Root->SetArrayField(TEXT("personality"), ToJsonStringArray(Persona.Personality));
	Root->SetStringField(TEXT("speaking_style"), Persona.SpeakingStyle);
	Root->SetArrayField(TEXT("goals"), ToJsonStringArray(Persona.Goals));
	Root->SetObjectField(TEXT("initial_relationship"), ToJsonRelationship(Persona.InitialRelationship));
	Root->SetObjectField(TEXT("initial_instant_state"), ToJsonInstantState(Persona.InitialInstantState));

	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		return Fail(OutError, TEXT("failed to serialize Persona JSON"));
	}
	OutError.Reset();
	return true;
}

bool FZLSocialPersonaJsonCodec::Deserialize(const FString& Json, FZLSocialPersonaData& OutPersona, FString& OutError)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return Fail(OutError, TEXT("JSON is not a valid Persona object"));
	}

	static const TSet<FString> RootFields = {TEXT("schema_version"), TEXT("stable_id"), TEXT("display_name"), TEXT("background_summary"), TEXT("role"), TEXT("personality"), TEXT("speaking_style"), TEXT("goals"), TEXT("initial_relationship"), TEXT("initial_instant_state")};
	if (!HasOnlyFields(Root, RootFields, OutError))
	{
		return false;
	}

	int32 SchemaVersion = 0;
	FString StableId;
	FZLSocialPersonaData Candidate;
	const TSharedPtr<FJsonObject>* Relationship = nullptr;
	const TSharedPtr<FJsonObject>* InstantState = nullptr;
	if (!Root->TryGetNumberField(TEXT("schema_version"), SchemaVersion) || SchemaVersion != PersonaSchemaVersion
		|| !Root->TryGetStringField(TEXT("stable_id"), StableId)
		|| !Root->TryGetStringField(TEXT("display_name"), Candidate.DisplayName)
		|| !Root->TryGetStringField(TEXT("background_summary"), Candidate.BackgroundSummary)
		|| !Root->TryGetStringField(TEXT("role"), Candidate.Role)
		|| !Root->TryGetStringField(TEXT("speaking_style"), Candidate.SpeakingStyle)
		|| !Root->TryGetObjectField(TEXT("initial_relationship"), Relationship)
		|| !Root->TryGetObjectField(TEXT("initial_instant_state"), InstantState))
	{
		return Fail(OutError, TEXT("Persona JSON has a missing or invalid required field"));
	}
	Candidate.StableId = FName(*StableId);
	if (!ReadStringArray(Root, TEXT("personality"), Candidate.Personality, OutError)
		|| !ReadStringArray(Root, TEXT("goals"), Candidate.Goals, OutError)
		|| !ReadRelationship(*Relationship, Candidate.InitialRelationship, OutError)
		|| !ReadInstantState(*InstantState, Candidate.InitialInstantState, OutError)
		|| !Candidate.IsValid(&OutError))
	{
		return false;
	}

	OutPersona = MoveTemp(Candidate);
	OutError.Reset();
	return true;
}

bool FZLSocialPersonaJsonCodec::SerializeBatch(const TArray<FZLSocialPersonaData>& Personas, FString& OutJson, FString& OutError)
{
	TArray<TSharedPtr<FJsonValue>> JsonPersonas;
	TSet<FName> StableIds;
	for (const FZLSocialPersonaData& Persona : Personas)
	{
		FString PersonaJson;
		if (!Serialize(Persona, PersonaJson, OutError) || StableIds.Contains(Persona.StableId))
		{
			return Fail(OutError, StableIds.Contains(Persona.StableId) ? TEXT("batch contains a duplicate stable_id") : *OutError);
		}
		StableIds.Add(Persona.StableId);
		TSharedPtr<FJsonObject> PersonaObject;
		if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(PersonaJson), PersonaObject))
		{
			return Fail(OutError, TEXT("failed to create Persona batch JSON"));
		}
		JsonPersonas.Add(MakeShared<FJsonValueObject>(PersonaObject));
	}
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("schema_version"), PersonaSchemaVersion);
	Root->SetArrayField(TEXT("personas"), JsonPersonas);
	return FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&OutJson)) || Fail(OutError, TEXT("failed to serialize Persona batch JSON"));
}

bool FZLSocialPersonaJsonCodec::DeserializeBatch(const FString& Json, TArray<FZLSocialPersonaData>& OutPersonas, FString& OutError)
{
	TSharedPtr<FJsonObject> Root;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
	{
		return Fail(OutError, TEXT("batch JSON is not valid"));
	}
	static const TSet<FString> Fields = {TEXT("schema_version"), TEXT("personas")};
	int32 SchemaVersion = 0;
	const TArray<TSharedPtr<FJsonValue>>* JsonPersonas = nullptr;
	if (!HasOnlyFields(Root, Fields, OutError) || !Root->TryGetNumberField(TEXT("schema_version"), SchemaVersion) || SchemaVersion != PersonaSchemaVersion || !Root->TryGetArrayField(TEXT("personas"), JsonPersonas) || !JsonPersonas)
	{
		return Fail(OutError, TEXT("batch JSON schema is invalid"));
	}
	TArray<FZLSocialPersonaData> Candidates;
	TSet<FName> StableIds;
	for (const TSharedPtr<FJsonValue>& Value : *JsonPersonas)
	{
		const TSharedPtr<FJsonObject>* Object = nullptr;
		if (!Value.IsValid() || !Value->TryGetObject(Object) || !Object || !Object->IsValid())
		{
			return Fail(OutError, TEXT("batch Persona item is invalid"));
		}
		FString ItemJson;
		if (!FJsonSerializer::Serialize(Object->ToSharedRef(), TJsonWriterFactory<>::Create(&ItemJson)))
		{
			return Fail(OutError, TEXT("failed to read batch Persona item"));
		}
		FZLSocialPersonaData Candidate;
		if (!Deserialize(ItemJson, Candidate, OutError) || StableIds.Contains(Candidate.StableId))
		{
			return Fail(OutError, StableIds.Contains(Candidate.StableId) ? TEXT("batch contains a duplicate stable_id") : *OutError);
		}
		StableIds.Add(Candidate.StableId);
		Candidates.Add(MoveTemp(Candidate));
	}
	OutPersonas = MoveTemp(Candidates);
	OutError.Reset();
	return true;
}

void UZLSocialPersonaAsset::ImportPersonaJson()
{
	FString Json;
	if (ImportFile.FilePath.IsEmpty() || !FFileHelper::LoadFileToString(Json, *ImportFile.FilePath))
	{
		LastJsonOperationResult = TEXT("Persona JSON file could not be read");
		return;
	}
	ImportPersonaJsonText(Json);
}

bool UZLSocialPersonaAsset::ImportPersonaJsonText(const FString& Json)
{
	FString Error;
	FZLSocialPersonaData Candidate;
	if (!FZLSocialPersonaJsonCodec::Deserialize(Json, Candidate, Error))
	{
		LastJsonOperationResult = Error.IsEmpty() ? TEXT("Persona JSON import failed") : Error;
		return false;
	}

	Modify();
	Persona = MoveTemp(Candidate);
	MarkPackageDirty();
	LastJsonOperationResult = TEXT("Persona JSON import succeeded");
	return true;
}

void UZLSocialPersonaAsset::ExportPersonaJson()
{
	FString Json;
	FString Error;
	if (ExportFile.FilePath.IsEmpty() || !FZLSocialPersonaJsonCodec::Serialize(Persona, Json, Error) || !FFileHelper::SaveStringToFile(Json, *ExportFile.FilePath))
	{
		LastJsonOperationResult = Error.IsEmpty() ? TEXT("Persona JSON export failed") : Error;
		return;
	}

	LastJsonOperationResult = TEXT("Persona JSON export succeeded");
}
