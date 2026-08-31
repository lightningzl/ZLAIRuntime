#pragma once

#include "CoreMinimal.h"

#include "ZLAIServiceTypes.h"

namespace ZLAIServiceProtocol
{
	ZLAIRUNTIME_API bool ValidateDialogueRequest(const FZLDialogueRequest& Request, FString& OutError);

	ZLAIRUNTIME_API bool SerializeDialogueRequest(const FZLDialogueRequest& Request, FString& OutJson);

	ZLAIRUNTIME_API bool TryParseDialogueResponse(const FString& Json, FZLDialogueResponse& OutResponse);

	ZLAIRUNTIME_API bool TryParseServiceError(const FString& Json, FZLServiceError& OutError);

	ZLAIRUNTIME_API bool ValidateDecisionRequest(const FZLDecisionRequest& Request, FString& OutError);

	ZLAIRUNTIME_API bool SerializeDecisionRequest(const FZLDecisionRequest& Request, FString& OutJson);

	ZLAIRUNTIME_API bool TryParseDecisionResponse(const FString& Json, FZLDecisionResponse& OutResponse);
}
