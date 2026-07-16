#pragma once

#include "CoreMinimal.h"

#include "ZLAIServiceTypes.h"

namespace ZLAIServiceProtocol
{
	ZLAIRUNTIME_API bool SerializeDialogueRequest(const FZLDialogueRequest& Request, FString& OutJson);

	ZLAIRUNTIME_API bool TryParseDialogueResponse(const FString& Json, FZLDialogueResponse& OutResponse);

	ZLAIRUNTIME_API bool TryParseServiceError(const FString& Json, FZLServiceError& OutError);
}
