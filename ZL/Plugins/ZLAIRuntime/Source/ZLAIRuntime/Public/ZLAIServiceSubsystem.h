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
	/** Sends one v2 personal social-plan request without changing the v1 path. */
	FString SendDecisionV2Request(
		FZLDecisionV2Request DecisionRequest,
		FZLDecisionV2SuccessDelegate OnSuccess,
		FZLDecisionV2FailureDelegate OnFailure);
	/** Sends one complete personal Decision request to the independent endpoint. */
	FString SendDecisionRequest(
		FZLDecisionRequest DecisionRequest,
		FZLDecisionSuccessDelegate OnSuccess,
		FZLDecisionFailureDelegate OnFailure);

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

	/**
	 * Sends one dialogue request with an explicit persistent-memory scope.
	 * The scope is validated before any HTTP request is created.
	 */
	FString SendDialogueRequest(
		const FString& NpcId,
		const FString& PlayerInput,
		const FZLDialogueMemory& Memory,
		FZLDialogueSuccessDelegate OnSuccess,
		FZLDialogueFailureDelegate OnFailure);

	/** Sends one dialogue request with both transient context and Memory. */
	FString SendDialogueRequest(
		const FString& NpcId,
		const FString& PlayerInput,
		const FZLDialogueContext& Context,
		const FZLDialogueMemory& Memory,
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

	void CompleteDecisionRequest(
		const FString& ExpectedRequestId,
		const FString& ExpectedNpcId,
		int64 ExpectedStateVersion,
		double SentAtSeconds,
		int32 TtlMs,
		bool bTransportSucceeded,
		bool bTimedOut,
		int32 HttpStatusCode,
		const FString& ResponseBody,
		FZLDecisionSuccessDelegate OnSuccess,
		FZLDecisionFailureDelegate OnFailure);

	void CompleteDecisionV2Request(
		const FString& ExpectedRequestId, const FString& ExpectedNpcId, int64 ExpectedStateVersion,
		double SentAtSeconds, int32 TtlMs, bool bTransportSucceeded, bool bTimedOut,
		int32 HttpStatusCode, const FString& ResponseBody,
		FZLDecisionV2SuccessDelegate OnSuccess, FZLDecisionV2FailureDelegate OnFailure);

	friend class FZLAIServiceFailureHandlingTest;
	friend class FZLDecisionClientCompletionTest;
};
