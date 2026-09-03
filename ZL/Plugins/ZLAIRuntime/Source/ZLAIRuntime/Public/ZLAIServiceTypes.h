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
	Parse,
	Stale
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
struct ZLAIRUNTIME_API FZLDialogueMemory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FString ScopeId;
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

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	bool bHasMemory = false;

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI")
	FZLDialogueMemory Memory;
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
struct ZLAIRUNTIME_API FZLDecisionTrigger
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString EventId;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString Kind;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString SourceId;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString TargetId;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") TArray<FString> Channels;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString Content;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString Summary;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") int64 OccurredAtMs = 0;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionRelationship
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") float Trust = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") float Affinity = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") float Fear = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") float Familiarity = 0.0f;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionInstantState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") float Fear = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") float Anger = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") float Curiosity = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") float Alert = 0.0f;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionHistoryItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString Kind;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString SourceId;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString TargetId;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString Summary;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") int64 OccurredAtMs = 0;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FZLDialogueNpcContext Npc;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FZLDecisionRelationship Relationship;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FZLDecisionInstantState InstantState;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") TArray<FZLDecisionHistoryItem> RecentHistory;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionAllowedTool
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FString Name;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") TArray<FString> TargetIds;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString RequestId;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString NpcId;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") int64 StateVersion = 0;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") int32 TtlMs = 30000;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FZLDecisionTrigger Trigger;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") FZLDecisionContext Context;
	UPROPERTY(BlueprintReadWrite, Category = "ZL|AI|Decision") TArray<FZLDecisionAllowedTool> AllowedTools;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionSpeech
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString Text;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString Emotion;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionToolCall
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString CallId;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString Name;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString TargetId;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString RequestId;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString NpcId;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") int64 StateVersion = 0;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString DecisionId;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString Intent;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") bool bHasSpeech = false;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FZLDecisionSpeech Speech;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") bool bHasToolCall = false;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FZLDecisionToolCall ToolCall;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") float Confidence = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "ZL|AI|Decision") FString Provider;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionV2SocialFact
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") FString Kind;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") FString SubjectId;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") FString TargetId;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") FString Summary;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") int64 OccurredAtMs = 0;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") float Salience = 0.0f;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionV2Capability
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") FString CapabilityId;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") FString Kind;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") TArray<FString> TargetIds;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionV2Request
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString RequestId;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString NpcId;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") int64 StateVersion = 0;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") int32 TtlMs = 30000;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") FZLDecisionTrigger Trigger;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") FZLDecisionContext Context;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") TArray<FZLDecisionV2SocialFact> SocialSituation;
	UPROPERTY(BlueprintReadWrite, Category="ZL|AI|DecisionV2") TArray<FZLDecisionV2Capability> AvailableCapabilities;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionV2PlanStep
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString StepId;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString CapabilityId;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString TargetId;
};

USTRUCT(BlueprintType)
struct ZLAIRUNTIME_API FZLDecisionV2Response
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString RequestId;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString NpcId;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") int64 StateVersion = 0;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString DecisionId;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString Objective;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString PublicReason;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString AttentionTargetId;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") TArray<FZLDecisionV2PlanStep> Steps;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") bool bHasSpeech = false;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FZLDecisionSpeech Speech;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") float Confidence = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category="ZL|AI|DecisionV2") FString Provider;
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
DECLARE_DELEGATE_OneParam(FZLDecisionSuccessDelegate, const FZLDecisionResponse&);
DECLARE_DELEGATE_OneParam(FZLDecisionFailureDelegate, const FZLServiceError&);
DECLARE_DELEGATE_OneParam(FZLDecisionV2SuccessDelegate, const FZLDecisionV2Response&);
DECLARE_DELEGATE_OneParam(FZLDecisionV2FailureDelegate, const FZLServiceError&);
