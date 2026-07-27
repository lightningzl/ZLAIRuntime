#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ZLAIServiceTypes.h"

#include "ZLAIServiceSubsystem.generated.h"

UCLASS()
class ZLAIRUNTIME_API UZLAIServiceSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Sends one dialogue request using UZLAIServiceSettings.
	 * Returns the generated request ID immediately; completion delegates run on the Game Thread.
	 */
	FString SendDialogueRequest(
		const FString& NpcId,
		const FString& PlayerInput,
		FZLDialogueSuccessDelegate OnSuccess,
		FZLDialogueFailureDelegate OnFailure);

	/**
	 * Sends one dialogue request with a complete transient context snapshot.
	 * The snapshot is validated before any HTTP request is created.
	 */
	FString SendDialogueRequest(
		const FString& NpcId,
		const FString& PlayerInput,
		const FZLDialogueContext& Context,
		FZLDialogueSuccessDelegate OnSuccess,
		FZLDialogueFailureDelegate OnFailure);

private:
	FString SendDialogueRequest(
		FZLDialogueRequest DialogueRequest,
		FZLDialogueSuccessDelegate OnSuccess,
		FZLDialogueFailureDelegate OnFailure);

	void CompleteRequest(
		const FString& ExpectedRequestId,
		const FString& ExpectedNpcId,
		bool bTransportSucceeded,
		bool bTimedOut,
		int32 HttpStatusCode,
		const FString& ResponseBody,
		FZLDialogueSuccessDelegate OnSuccess,
		FZLDialogueFailureDelegate OnFailure);

	friend class FZLAIServiceFailureHandlingTest;
};
