#pragma once

#include "CoreMinimal.h"

#include "ZLAIServiceTypes.generated.h"

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
