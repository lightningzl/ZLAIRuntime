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

	bool IsDecisionKind(const FString& Value)
	{
		return Value == TEXT("speech") || Value == TEXT("action_result");
	}

	bool IsDecisionTool(const FString& Value)
	{
		return Value == TEXT("face_target")
			|| Value == TEXT("move_toward")
			|| Value == TEXT("move_away")
			|| Value == TEXT("stop");
	}

	bool IsDecisionIntent(const FString& Value)
	{
		return Value == TEXT("respond")
			|| Value == TEXT("engage")
			|| Value == TEXT("disengage")
			|| Value == TEXT("hold");
	}

	bool ValidateUnitValue(const float Value, const float Minimum, const TCHAR* FieldName, FString& OutError)
	{
		if (!FMath::IsFinite(Value) || Value < Minimum || Value > 1.0f)
		{
			OutError = FString::Printf(TEXT("%s is outside the confirmed range"), FieldName);
			return false;
		}
		return true;
	}

	bool ValidateDecisionNpc(const FZLDialogueNpcContext& Npc, FString& OutError)
	{
		return ValidateContextText(Npc.DisplayName, 64, TEXT("context.npc.display_name"), OutError)
			&& ValidateContextText(Npc.Role, 128, TEXT("context.npc.role"), OutError)
			&& ValidateStringArray(Npc.Personality, 1, 8, 64, TEXT("context.npc.personality"), OutError)
			&& ValidateContextText(Npc.SpeakingStyle, 256, TEXT("context.npc.speaking_style"), OutError)
			&& ValidateStringArray(Npc.Goals, 0, 8, 128, TEXT("context.npc.goals"), OutError);
	}

	TSharedRef<FJsonObject> SerializeDecisionNpc(const FZLDialogueNpcContext& Npc)
	{
		const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("display_name"), Npc.DisplayName);
		Object->SetStringField(TEXT("role"), Npc.Role);
		Object->SetArrayField(TEXT("personality"), SerializeStringArray(Npc.Personality));
		Object->SetStringField(TEXT("speaking_style"), Npc.SpeakingStyle);
		Object->SetArrayField(TEXT("goals"), SerializeStringArray(Npc.Goals));
		return Object;
	}

	bool TryGetProtocolInt64(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, int64& OutValue)
	{
		double Number = 0.0;
		if (!Object.IsValid() || !Object->TryGetNumberField(FieldName, Number)
			|| !FMath::IsFinite(Number) || Number < 0.0 || FMath::FloorToDouble(Number) != Number)
		{
			return false;
		}
		OutValue = static_cast<int64>(Number);
		return true;
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

bool ZLAIServiceProtocol::ValidateDecisionRequest(const FZLDecisionRequest& Request, FString& OutError)
{
	OutError.Reset();
	if (!ValidateContextText(Request.RequestId, 128, TEXT("request_id"), OutError)
		|| !ValidateContextText(Request.NpcId, 128, TEXT("npc_id"), OutError)
		|| Request.StateVersion < 0
		|| Request.TtlMs < 100 || Request.TtlMs > 60000)
	{
		if (OutError.IsEmpty()) { OutError = TEXT("state_version or ttl_ms is outside the confirmed range"); }
		return false;
	}

	const FZLDecisionTrigger& Trigger = Request.Trigger;
	if (!ValidateContextText(Trigger.EventId, 128, TEXT("trigger.event_id"), OutError)
		|| !IsDecisionKind(Trigger.Kind)
		|| !ValidateContextText(Trigger.SourceId, 128, TEXT("trigger.source_id"), OutError)
		|| (!Trigger.TargetId.IsEmpty() && !ValidateContextText(Trigger.TargetId, 128, TEXT("trigger.target_id"), OutError))
		|| !ValidateContextText(Trigger.Summary, 256, TEXT("trigger.summary"), OutError)
		|| Trigger.OccurredAtMs < 0)
	{
		if (OutError.IsEmpty()) { OutError = TEXT("trigger is invalid"); }
		return false;
	}
	if ((Trigger.Kind == TEXT("speech") && !ValidateContextText(Trigger.Content, 512, TEXT("trigger.content"), OutError))
		|| (Trigger.Kind == TEXT("action_result") && !Trigger.Content.IsEmpty()))
	{
		if (OutError.IsEmpty()) { OutError = TEXT("action_result trigger must not contain content"); }
		return false;
	}
	if (Trigger.Channels.Num() < 1 || Trigger.Channels.Num() > 3)
	{
		OutError = TEXT("trigger.channels must contain between 1 and 3 items");
		return false;
	}
	TSet<FString> ChannelNames;
	for (const FString& Channel : Trigger.Channels)
	{
		if ((Channel != TEXT("direct") && Channel != TEXT("visual") && Channel != TEXT("auditory"))
			|| ChannelNames.Contains(Channel))
		{
			OutError = TEXT("trigger.channels contains an unknown or duplicate channel");
			return false;
		}
		ChannelNames.Add(Channel);
	}

	const FZLDecisionContext& Context = Request.Context;
	if (!ValidateDecisionNpc(Context.Npc, OutError)
		|| !ValidateUnitValue(Context.Relationship.Trust, -1.0f, TEXT("context.relationship.trust"), OutError)
		|| !ValidateUnitValue(Context.Relationship.Affinity, -1.0f, TEXT("context.relationship.affinity"), OutError)
		|| !ValidateUnitValue(Context.Relationship.Fear, 0.0f, TEXT("context.relationship.fear"), OutError)
		|| !ValidateUnitValue(Context.Relationship.Familiarity, 0.0f, TEXT("context.relationship.familiarity"), OutError)
		|| !ValidateUnitValue(Context.InstantState.Fear, 0.0f, TEXT("context.instant_state.fear"), OutError)
		|| !ValidateUnitValue(Context.InstantState.Anger, 0.0f, TEXT("context.instant_state.anger"), OutError)
		|| !ValidateUnitValue(Context.InstantState.Curiosity, 0.0f, TEXT("context.instant_state.curiosity"), OutError)
		|| !ValidateUnitValue(Context.InstantState.Alert, 0.0f, TEXT("context.instant_state.alert"), OutError))
	{
		return false;
	}
	if (Context.RecentHistory.Num() > 8)
	{
		OutError = TEXT("context.recent_history must contain at most 8 items");
		return false;
	}
	for (int32 Index = 0; Index < Context.RecentHistory.Num(); ++Index)
	{
		const FZLDecisionHistoryItem& Item = Context.RecentHistory[Index];
		if (!IsDecisionKind(Item.Kind)
			|| !ValidateContextText(Item.SourceId, 128, TEXT("context.recent_history.source_id"), OutError)
			|| (!Item.TargetId.IsEmpty() && !ValidateContextText(Item.TargetId, 128, TEXT("context.recent_history.target_id"), OutError))
			|| !ValidateContextText(Item.Summary, 256, TEXT("context.recent_history.summary"), OutError)
			|| Item.OccurredAtMs < 0)
		{
			if (OutError.IsEmpty()) { OutError = TEXT("context.recent_history contains an invalid item"); }
			return false;
		}
	}

	if (Request.AllowedTools.Num() < 1 || Request.AllowedTools.Num() > 4)
	{
		OutError = TEXT("allowed_tools must contain between 1 and 4 items");
		return false;
	}
	TSet<FString> ToolNames;
	for (const FZLDecisionAllowedTool& Tool : Request.AllowedTools)
	{
		if (!IsDecisionTool(Tool.Name) || ToolNames.Contains(Tool.Name) || Tool.TargetIds.Num() > 4)
		{
			OutError = TEXT("allowed_tools contains an unknown, duplicate, or oversized Tool");
			return false;
		}
		if ((Tool.Name == TEXT("stop") && !Tool.TargetIds.IsEmpty())
			|| (Tool.Name != TEXT("stop") && Tool.TargetIds.IsEmpty()))
		{
			OutError = TEXT("allowed Tool target_ids do not match Tool semantics");
			return false;
		}
		TSet<FString> TargetIds;
		for (const FString& TargetId : Tool.TargetIds)
		{
			if (!ValidateContextText(TargetId, 128, TEXT("allowed_tools.target_ids"), OutError)
				|| TargetIds.Contains(TargetId))
			{
				if (OutError.IsEmpty()) { OutError = TEXT("allowed Tool contains a duplicate target_id"); }
				return false;
			}
			TargetIds.Add(TargetId);
		}
		ToolNames.Add(Tool.Name);
	}
	return true;
}

bool ZLAIServiceProtocol::SerializeDecisionRequest(const FZLDecisionRequest& Request, FString& OutJson)
{
	FString ValidationError;
	if (!ValidateDecisionRequest(Request, ValidationError))
	{
		OutJson.Reset();
		return false;
	}

	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("request_id"), Request.RequestId);
	Root->SetStringField(TEXT("npc_id"), Request.NpcId);
	Root->SetNumberField(TEXT("state_version"), static_cast<double>(Request.StateVersion));
	Root->SetNumberField(TEXT("ttl_ms"), Request.TtlMs);

	const TSharedRef<FJsonObject> Trigger = MakeShared<FJsonObject>();
	Trigger->SetStringField(TEXT("event_id"), Request.Trigger.EventId);
	Trigger->SetStringField(TEXT("kind"), Request.Trigger.Kind);
	Trigger->SetStringField(TEXT("source_id"), Request.Trigger.SourceId);
	if (!Request.Trigger.TargetId.IsEmpty()) { Trigger->SetStringField(TEXT("target_id"), Request.Trigger.TargetId); }
	Trigger->SetArrayField(TEXT("channels"), SerializeStringArray(Request.Trigger.Channels));
	if (Request.Trigger.Kind == TEXT("speech")) { Trigger->SetStringField(TEXT("content"), Request.Trigger.Content); }
	Trigger->SetStringField(TEXT("summary"), Request.Trigger.Summary);
	Trigger->SetNumberField(TEXT("occurred_at_ms"), static_cast<double>(Request.Trigger.OccurredAtMs));
	Root->SetObjectField(TEXT("trigger"), Trigger);

	const TSharedRef<FJsonObject> Context = MakeShared<FJsonObject>();
	Context->SetObjectField(TEXT("npc"), SerializeDecisionNpc(Request.Context.Npc));
	const TSharedRef<FJsonObject> Relationship = MakeShared<FJsonObject>();
	Relationship->SetNumberField(TEXT("trust"), Request.Context.Relationship.Trust);
	Relationship->SetNumberField(TEXT("affinity"), Request.Context.Relationship.Affinity);
	Relationship->SetNumberField(TEXT("fear"), Request.Context.Relationship.Fear);
	Relationship->SetNumberField(TEXT("familiarity"), Request.Context.Relationship.Familiarity);
	Context->SetObjectField(TEXT("relationship"), Relationship);
	const TSharedRef<FJsonObject> InstantState = MakeShared<FJsonObject>();
	InstantState->SetNumberField(TEXT("fear"), Request.Context.InstantState.Fear);
	InstantState->SetNumberField(TEXT("anger"), Request.Context.InstantState.Anger);
	InstantState->SetNumberField(TEXT("curiosity"), Request.Context.InstantState.Curiosity);
	InstantState->SetNumberField(TEXT("alert"), Request.Context.InstantState.Alert);
	Context->SetObjectField(TEXT("instant_state"), InstantState);
	TArray<TSharedPtr<FJsonValue>> History;
	for (const FZLDecisionHistoryItem& Item : Request.Context.RecentHistory)
	{
		const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("kind"), Item.Kind);
		Object->SetStringField(TEXT("source_id"), Item.SourceId);
		if (!Item.TargetId.IsEmpty()) { Object->SetStringField(TEXT("target_id"), Item.TargetId); }
		Object->SetStringField(TEXT("summary"), Item.Summary);
		Object->SetNumberField(TEXT("occurred_at_ms"), static_cast<double>(Item.OccurredAtMs));
		History.Add(MakeShared<FJsonValueObject>(Object));
	}
	Context->SetArrayField(TEXT("recent_history"), MoveTemp(History));
	Root->SetObjectField(TEXT("context"), Context);

	TArray<TSharedPtr<FJsonValue>> AllowedTools;
	for (const FZLDecisionAllowedTool& Tool : Request.AllowedTools)
	{
		const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("name"), Tool.Name);
		Object->SetArrayField(TEXT("target_ids"), SerializeStringArray(Tool.TargetIds));
		AllowedTools.Add(MakeShared<FJsonValueObject>(Object));
	}
	Root->SetArrayField(TEXT("allowed_tools"), MoveTemp(AllowedTools));

	OutJson.Reset();
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	return FJsonSerializer::Serialize(Root, Writer);
}

bool ZLAIServiceProtocol::TryParseDecisionResponse(const FString& Json, FZLDecisionResponse& OutResponse)
{
	TSharedPtr<FJsonObject> Root;
	if (!TryDeserializeObject(Json, Root)) { return false; }

	FZLDecisionResponse Parsed;
	double Confidence = 0.0;
	if (!Root->TryGetStringField(TEXT("request_id"), Parsed.RequestId)
		|| !Root->TryGetStringField(TEXT("npc_id"), Parsed.NpcId)
		|| !TryGetProtocolInt64(Root, TEXT("state_version"), Parsed.StateVersion)
		|| !Root->TryGetStringField(TEXT("decision_id"), Parsed.DecisionId)
		|| !Root->TryGetStringField(TEXT("intent"), Parsed.Intent)
		|| !Root->TryGetNumberField(TEXT("confidence"), Confidence)
		|| !Root->TryGetStringField(TEXT("provider"), Parsed.Provider)
		|| !FMath::IsFinite(Confidence) || Confidence < 0.0 || Confidence > 1.0
		|| !IsDecisionIntent(Parsed.Intent)
		|| (Parsed.Provider != TEXT("stub") && Parsed.Provider != TEXT("kimi")))
	{
		return false;
	}
	FString ValidationError;
	if (!ValidateContextText(Parsed.RequestId, 128, TEXT("request_id"), ValidationError)
		|| !ValidateContextText(Parsed.NpcId, 128, TEXT("npc_id"), ValidationError)
		|| !ValidateContextText(Parsed.DecisionId, 128, TEXT("decision_id"), ValidationError))
	{
		return false;
	}
	Parsed.Confidence = static_cast<float>(Confidence);

	const TSharedPtr<FJsonObject>* Speech = nullptr;
	if (Root->TryGetObjectField(TEXT("speech"), Speech))
	{
		if (Speech == nullptr || !Speech->IsValid()
			|| !(*Speech)->TryGetStringField(TEXT("text"), Parsed.Speech.Text)
			|| !ValidateContextText(Parsed.Speech.Text, 512, TEXT("speech.text"), ValidationError))
		{
			return false;
		}
		if ((*Speech)->HasField(TEXT("emotion"))
			&& (!(*Speech)->TryGetStringField(TEXT("emotion"), Parsed.Speech.Emotion)
				|| !ValidateContextText(Parsed.Speech.Emotion, 64, TEXT("speech.emotion"), ValidationError)))
		{
			return false;
		}
		Parsed.bHasSpeech = true;
	}

	const TSharedPtr<FJsonObject>* ToolCall = nullptr;
	if (Root->TryGetObjectField(TEXT("tool_call"), ToolCall))
	{
		if (ToolCall == nullptr || !ToolCall->IsValid()
			|| !(*ToolCall)->TryGetStringField(TEXT("call_id"), Parsed.ToolCall.CallId)
			|| !(*ToolCall)->TryGetStringField(TEXT("name"), Parsed.ToolCall.Name)
			|| !ValidateContextText(Parsed.ToolCall.CallId, 128, TEXT("tool_call.call_id"), ValidationError)
			|| !IsDecisionTool(Parsed.ToolCall.Name))
		{
			return false;
		}
		const bool bHasTarget = (*ToolCall)->TryGetStringField(TEXT("target_id"), Parsed.ToolCall.TargetId);
		if ((Parsed.ToolCall.Name == TEXT("stop") && bHasTarget)
			|| (Parsed.ToolCall.Name != TEXT("stop")
				&& (!bHasTarget || !ValidateContextText(Parsed.ToolCall.TargetId, 128, TEXT("tool_call.target_id"), ValidationError))))
		{
			return false;
		}
		Parsed.bHasToolCall = true;
	}

	if (!Parsed.bHasSpeech && !Parsed.bHasToolCall) { return false; }
	OutResponse = MoveTemp(Parsed);
	return true;
}
