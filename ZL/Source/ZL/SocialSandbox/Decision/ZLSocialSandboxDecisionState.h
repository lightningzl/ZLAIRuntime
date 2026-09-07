#pragma once

#include "CoreMinimal.h"

struct FZLSocialSandboxDecisionDebug
{
	FString RequestId;
	FString Provider;
	FString Intent;
	FString ToolName;
	FName ToolResult;
	int64 StateVersion = 0;
	int32 LatencyMs = 0;
	bool bSpeechAccepted = false;
	bool bInFlight = false;
	bool bPending = false;
	FName TriggerReason;
	int32 CoalescedTriggers = 0;
	int32 AutomaticReplans = 0;
	FString ConflictLevel;
	bool bLocalFallback = false;
};
