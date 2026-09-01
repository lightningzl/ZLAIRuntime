#pragma once

#include "CoreMinimal.h"

#include "ZLSocialToolRegistry.generated.h"

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialToolDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName Name;
	UPROPERTY(BlueprintReadOnly) FName RequiredCapability;
	UPROPERTY(BlueprintReadOnly) bool bRequiresTarget = true;
	UPROPERTY(BlueprintReadOnly) bool bRequiresNavigation = false;
	UPROPERTY(BlueprintReadOnly) float MaxDistance = 3000.0f;
	UPROPERTY(BlueprintReadOnly) float CooldownSeconds = 0.5f;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialToolCall
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString CallId;
	UPROPERTY(BlueprintReadOnly) FName Name;
	UPROPERTY(BlueprintReadOnly) FName TargetId;
	UPROPERTY(BlueprintReadOnly) int64 StateVersion = 0;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialToolValidationContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int64 CurrentStateVersion = 0;
	UPROPERTY(BlueprintReadOnly) double NowSeconds = 0.0;
	UPROPERTY(BlueprintReadOnly) float DistanceToTarget = 0.0f;
	UPROPERTY(BlueprintReadOnly) bool bRequestFresh = true;
	UPROPERTY(BlueprintReadOnly) bool bTargetValid = true;
	UPROPERTY(BlueprintReadOnly) bool bNavigationReachable = true;
	UPROPERTY(BlueprintReadOnly) bool bExecutable = true;
	UPROPERTY(BlueprintReadOnly) int32 ExecutionsInWindow = 0;
	UPROPERTY(BlueprintReadOnly) TArray<FName> Capabilities;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialToolValidationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bAccepted = false;
	UPROPERTY(BlueprintReadOnly) FName ReasonCode;
};

class ZLASOCIALRUNTIME_API FZLSocialToolRegistry
{
public:
	static constexpr int32 MaxDefinitions = 16;
	static constexpr int32 MaxRememberedCalls = 128;
	static constexpr int32 MaxExecutionsPerWindow = 4;

	bool Register(const FZLSocialToolDefinition& Definition);
	void RegisterMilestone8Defaults();

	FZLSocialToolValidationResult ValidateAndCommit(
		const FZLSocialToolCall& Call,
		const FZLSocialToolValidationContext& Context);

	int32 GetDefinitionCount() const { return Definitions.Num(); }
	int32 GetRememberedCallCount() const { return CompletedCallIds.Num(); }

private:
	TMap<FName, FZLSocialToolDefinition> Definitions;
	TSet<FString> CompletedCallIds;
	TArray<FString> CompletedCallOrder;
	TMap<FName, double> LastExecutionSeconds;

	static FZLSocialToolValidationResult Reject(FName ReasonCode);
	void RememberCall(const FString& CallId);
};

namespace ZLSocialToolReason
{
	ZLASOCIALRUNTIME_API extern const FName Accepted;
	ZLASOCIALRUNTIME_API extern const FName UnknownTool;
	ZLASOCIALRUNTIME_API extern const FName InvalidCallId;
	ZLASOCIALRUNTIME_API extern const FName MissingCapability;
	ZLASOCIALRUNTIME_API extern const FName InvalidTarget;
	ZLASOCIALRUNTIME_API extern const FName StateVersionMismatch;
	ZLASOCIALRUNTIME_API extern const FName Expired;
	ZLASOCIALRUNTIME_API extern const FName DistanceExceeded;
	ZLASOCIALRUNTIME_API extern const FName NavigationUnavailable;
	ZLASOCIALRUNTIME_API extern const FName InvalidState;
	ZLASOCIALRUNTIME_API extern const FName Cooldown;
	ZLASOCIALRUNTIME_API extern const FName RateLimited;
	ZLASOCIALRUNTIME_API extern const FName DuplicateCall;
}
