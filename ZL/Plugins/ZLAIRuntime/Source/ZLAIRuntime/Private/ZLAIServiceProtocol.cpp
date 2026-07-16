#include "ZLAIServiceProtocol.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	bool TryDeserializeObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject)
	{
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
	}
}

bool ZLAIServiceProtocol::SerializeDialogueRequest(const FZLDialogueRequest& Request, FString& OutJson)
{
	const TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("request_id"), Request.RequestId);
	RootObject->SetStringField(TEXT("npc_id"), Request.NpcId);
	RootObject->SetStringField(TEXT("player_input"), Request.PlayerInput);

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
