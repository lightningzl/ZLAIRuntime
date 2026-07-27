#pragma once

#include "CoreMinimal.h"

#include "ZLAIServiceTypes.generated.h"

UENUM(BlueprintType)
enum class EZLServiceErrorCategory : uint8
{
	Client,
	Network,
	Timeout,
	Http,
	Parse
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDialogueNpcContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FString DisplayName;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FString Role;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	TArray<FString> Personality;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FString SpeakingStyle;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	TArray<FString> Goals;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDialogueWorldContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FString Location;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FString Situation;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	TArray<FString> Facts;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDialogueHistoryMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FString Role;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FString Content;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDialogueContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FZLDialogueNpcContext Npc;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FZLDialogueWorldContext World;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	TArray<FZLDialogueHistoryMessage> DialogueHistory;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDialogueRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString RequestId;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString NpcId;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString PlayerInput;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	bool bHasContext = false;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FZLDialogueContext Context;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDialogueResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString RequestId;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString NpcId;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString Reply;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString Provider;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLServiceError
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	EZLServiceErrorCategory Category = EZLServiceErrorCategory::Client;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString RequestId;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString Code;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI")
	int32 HttpStatusCode = 0;
};

DECLARE_DELEGATE_OneParam(FZLDialogueSuccessDelegate, const FZLDialogueResponse&);
DECLARE_DELEGATE_OneParam(FZLDialogueFailureDelegate, const FZLServiceError&);
