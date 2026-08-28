#pragma once

#include "CoreMinimal.h"
#include "ZLSocialTypes.h"

struct ZLASOCIALRUNTIME_API FZLSocialReportConfirmation
{
	FZLSocialEvent SourceEvent;
	FName ReporterId;
	TArray<FName> ReceiverIds;
	FGuid CausationId;
	double ConfirmedAtSeconds = 0.0;
	float ReporterConfidence = 1.0f;
	bool bAnchorDerivedEvents = false;
};

struct ZLASOCIALRUNTIME_API FZLSocialPropagationResult
{
	TArray<FZLSocialEvent> DerivedEvents;
	int32 RejectedReceivers = 0;
	int32 RemainingRootBudget = 0;
	bool bDuplicateReporter = false;
};

class ZLASOCIALRUNTIME_API FZLSocialPropagation
{
public:
	explicit FZLSocialPropagation(float InConfidenceDecay = 0.8f, float InMinimumImportance = 0.2f);

	bool ConfirmReport(const FZLSocialReportConfirmation& Confirmation, FZLSocialPropagationResult& OutResult);
	bool HasReporterReported(FGuid RootEventId, FName ReporterId) const;
	int32 GetTrackedRootCount() const { return RootExpiryByRoot.Num(); }
	void Reset();

private:
	float ConfidenceDecay;
	float MinimumImportance;
	TMap<FGuid, TSet<FName>> ReportersByRoot;
	TMap<FGuid, int32> RemainingBudgetByRoot;
	TMap<FGuid, double> RootExpiryByRoot;
};
