#include "ZLAIServiceProtocol.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	int32 CountUnicodeCodePoints(const FString& Value)
	{
		int32 Count = 0;
		for (int32 Index = 0; Index < Value.Len(); ++Index)
		{
			const uint32 CodeUnit = static_cast<uint32>(Value[Index]);
			if (CodeUnit >= 0xD800 && CodeUnit <= 0xDBFF && Index + 1 < Value.Len())
			{
				const uint32 NextCodeUnit = static_cast<uint32>(Value[Index + 1]);
				if (NextCodeUnit >= 0xDC00 && NextCodeUnit <= 0xDFFF)
				{
					++Index;
				}
			}
			++Count;
		}
		return Count;
	}

	bool ValidateContextText(
		const FString& Value,
		const int32 MaxLength,
		const TCHAR* FieldName,
		FString& OutError)
	{
		const FString TrimmedValue = Value.TrimStartAndEnd();
		if (TrimmedValue.IsEmpty())
		{
			OutError = FString::Printf(TEXT("%s must not be blank"), FieldName);
			return false;
		}
		if (CountUnicodeCodePoints(TrimmedValue) > MaxLength)
		{
			OutError = FString::Printf(
				TEXT("%s must contain at most %d Unicode code points"),
				FieldName,
				MaxLength);
			return false;
		}
		return true;
	}

	bool ValidateStringArray(
		const TArray<FString>& Values,
		const int32 MinItems,
		const int32 MaxItems,
		const int32 MaxItemLength,
		const TCHAR* FieldName,
		FString& OutError)
	{
		if (Values.Num() < MinItems || Values.Num() > MaxItems)
		{
			OutError = FString::Printf(
				TEXT("%s must contain between %d and %d items"),
				FieldName,
				MinItems,
				MaxItems);
			return false;
		}

		for (int32 Index = 0; Index < Values.Num(); ++Index)
		{
			const FString ItemFieldName = FString::Printf(TEXT("%s[%d]"), FieldName, Index);
			if (!ValidateContextText(Values[Index], MaxItemLength, *ItemFieldName, OutError))
			{
				return false;
			}
		}
		return true;
	}

	TArray<TSharedPtr<FJsonValue>> SerializeStringArray(const TArray<FString>& Values)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		JsonValues.Reserve(Values.Num());
		for (const FString& Value : Values)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		return JsonValues;
	}

	TSharedRef<FJsonObject> SerializeDialogueContext(const FZLDialogueContext& Context)
	{
		const TSharedRef<FJsonObject> NpcObject = MakeShared<FJsonObject>();
		NpcObject->SetStringField(TEXT("display_name"), Context.Npc.DisplayName);
		NpcObject->SetStringField(TEXT("role"), Context.Npc.Role);
		NpcObject->SetArrayField(TEXT("personality"), SerializeStringArray(Context.Npc.Personality));
		NpcObject->SetStringField(TEXT("speaking_style"), Context.Npc.SpeakingStyle);
		NpcObject->SetArrayField(TEXT("goals"), SerializeStringArray(Context.Npc.Goals));

		const TSharedRef<FJsonObject> WorldObject = MakeShared<FJsonObject>();
		WorldObject->SetStringField(TEXT("location"), Context.World.Location);
		WorldObject->SetStringField(TEXT("situation"), Context.World.Situation);
		WorldObject->SetArrayField(TEXT("facts"), SerializeStringArray(Context.World.Facts));

		TArray<TSharedPtr<FJsonValue>> HistoryValues;
		HistoryValues.Reserve(Context.DialogueHistory.Num());
		for (const FZLDialogueHistoryMessage& Message : Context.DialogueHistory)
		{
			const TSharedRef<FJsonObject> MessageObject = MakeShared<FJsonObject>();
			MessageObject->SetStringField(TEXT("role"), Message.Role);
			MessageObject->SetStringField(TEXT("content"), Message.Content);
			HistoryValues.Add(MakeShared<FJsonValueObject>(MessageObject));
		}

		const TSharedRef<FJsonObject> ContextObject = MakeShared<FJsonObject>();
		ContextObject->SetObjectField(TEXT("npc"), NpcObject);
		ContextObject->SetObjectField(TEXT("world"), WorldObject);
		ContextObject->SetArrayField(TEXT("dialogue_history"), MoveTemp(HistoryValues));
		return ContextObject;
	}

	TSharedRef<FJsonObject> SerializeDialogueMemory(const FZLDialogueMemory& Memory)
	{
		const TSharedRef<FJsonObject> MemoryObject = MakeShared<FJsonObject>();
		MemoryObject->SetStringField(TEXT("scope_id"), Memory.ScopeId);
		return MemoryObject;
	}

	bool TryDeserializeObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject)
	{
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
	}
}

bool ZLAIServiceProtocol::ValidateDialogueRequest(const FZLDialogueRequest& Request, FString& OutError)
{
	OutError.Reset();
	if (Request.bHasMemory)
	{
		if (!ValidateContextText(Request.Memory.ScopeId, 128, TEXT("memory.scope_id"), OutError))
		{
			return false;
		}
		if (Request.Memory.ScopeId != Request.Memory.ScopeId.TrimStartAndEnd())
		{
			OutError = TEXT("memory.scope_id must not contain leading or trailing whitespace");
			return false;
		}
	}

	if (!Request.bHasContext)
	{
		return true;
	}

	const FZLDialogueContext& Context = Request.Context;
	if (!ValidateContextText(Context.Npc.DisplayName, 64, TEXT("context.npc.display_name"), OutError)
		|| !ValidateContextText(Context.Npc.Role, 128, TEXT("context.npc.role"), OutError)
		|| !ValidateStringArray(
			Context.Npc.Personality,
			1,
			8,
			64,
			TEXT("context.npc.personality"),
			OutError)
		|| !ValidateContextText(
			Context.Npc.SpeakingStyle,
			256,
			TEXT("context.npc.speaking_style"),
			OutError)
		|| !ValidateStringArray(Context.Npc.Goals, 0, 8, 128, TEXT("context.npc.goals"), OutError)
		|| !ValidateContextText(Context.World.Location, 128, TEXT("context.world.location"), OutError)
		|| !ValidateContextText(Context.World.Situation, 512, TEXT("context.world.situation"), OutError)
		|| !ValidateStringArray(Context.World.Facts, 0, 16, 256, TEXT("context.world.facts"), OutError))
	{
		return false;
	}

	if (Context.DialogueHistory.Num() > 8)
	{
		OutError = TEXT("context.dialogue_history must contain at most 8 items");
		return false;
	}

	for (int32 Index = 0; Index < Context.DialogueHistory.Num(); ++Index)
	{
		const FZLDialogueHistoryMessage& Message = Context.DialogueHistory[Index];
		if (Message.Role != TEXT("player") && Message.Role != TEXT("npc"))
		{
			OutError = FString::Printf(
				TEXT("context.dialogue_history[%d].role must be player or npc"),
				Index);
			return false;
		}

		const FString ContentFieldName = FString::Printf(
			TEXT("context.dialogue_history[%d].content"),
			Index);
		if (!ValidateContextText(Message.Content, 512, *ContentFieldName, OutError))
		{
			return false;
		}
	}

	return true;
}

bool ZLAIServiceProtocol::SerializeDialogueRequest(const FZLDialogueRequest& Request, FString& OutJson)
{
	FString ValidationError;
	if (!ValidateDialogueRequest(Request, ValidationError))
	{
		OutJson.Reset();
		return false;
	}

	const TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("request_id"), Request.RequestId);
	RootObject->SetStringField(TEXT("npc_id"), Request.NpcId);
	RootObject->SetStringField(TEXT("player_input"), Request.PlayerInput);
	if (Request.bHasContext)
	{
		RootObject->SetObjectField(TEXT("context"), SerializeDialogueContext(Request.Context));
	}
	if (Request.bHasMemory)
	{
		RootObject->SetObjectField(TEXT("memory"), SerializeDialogueMemory(Request.Memory));
	}

	OutJson.Reset();
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	return FJsonSerializer::Serialize(RootObject, Writer);
}

bool ZLAIServiceProtocol::TryParseDialogueResponse(const FString& Json, FZLDialogueResponse& OutResponse)
{
	TSharedPtr<FJsonObject> RootObject;
	if (!TryDeserializeObject(Json, RootObject))
	{
		return false;
	}

	FZLDialogueResponse ParsedResponse;
	if (!RootObject->TryGetStringField(TEXT("request_id"), ParsedResponse.RequestId)
		|| !RootObject->TryGetStringField(TEXT("npc_id"), ParsedResponse.NpcId)
		|| !RootObject->TryGetStringField(TEXT("reply"), ParsedResponse.Reply)
		|| !RootObject->TryGetStringField(TEXT("provider"), ParsedResponse.Provider))
	{
		return false;
	}

	OutResponse = MoveTemp(ParsedResponse);
	return true;
}

bool ZLAIServiceProtocol::TryParseServiceError(const FString& Json, FZLServiceError& OutError)
{
	TSharedPtr<FJsonObject> RootObject;
	if (!TryDeserializeObject(Json, RootObject))
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* ErrorObject = nullptr;
	FZLServiceError ParsedError;
	if (!RootObject->TryGetStringField(TEXT("request_id"), ParsedError.RequestId)
		|| !RootObject->TryGetObjectField(TEXT("error"), ErrorObject)
		|| ErrorObject == nullptr
		|| !ErrorObject->IsValid()
		|| !(*ErrorObject)->TryGetStringField(TEXT("code"), ParsedError.Code)
		|| !(*ErrorObject)->TryGetStringField(TEXT("message"), ParsedError.Message))
	{
		return false;
	}

	OutError = MoveTemp(ParsedError);
	return true;
}
