#include "ZLSocialPropagation.h"

FZLSocialPropagation::FZLSocialPropagation(const float InConfidenceDecay, const float InMinimumImportance)
	: ConfidenceDecay(FMath::Clamp(InConfidenceDecay, 0.0f, 1.0f))
	, MinimumImportance(FMath::Clamp(InMinimumImportance, 0.0f, 1.0f))
{
}

bool FZLSocialPropagation::ConfirmReport(const FZLSocialReportConfirmation& Confirmation, FZLSocialPropagationResult& OutResult)
{
	OutResult = FZLSocialPropagationResult();
	const FZLSocialEvent& Source = Confirmation.SourceEvent;
	if (!Source.IsValid(Confirmation.ConfirmedAtSeconds)
		|| Confirmation.ReporterId.IsNone()
		|| !Confirmation.CausationId.IsValid()
		|| Confirmation.ReceiverIds.IsEmpty()
		|| Source.ChainDepth >= ZLSocialEventLimits::MaxChainDepth
		|| Source.ChainBudget <= 0
		|| Source.Severity * Source.Confidence < MinimumImportance)
	{
		return false;
	}

	TSet<FName>& Reporters = ReportersByRoot.FindOrAdd(Source.RootEventId);
	if (Reporters.Contains(Confirmation.ReporterId))
	{
		OutResult.bDuplicateReporter = true;
		return false;
	}

	if (!RemainingBudgetByRoot.Contains(Source.RootEventId))
	{
		RemainingBudgetByRoot.Add(Source.RootEventId, Source.ChainBudget);
	}
	int32& RemainingBudget = RemainingBudgetByRoot.FindChecked(Source.RootEventId);
	RemainingBudget = FMath::Min(RemainingBudget, Source.ChainBudget);
	if (RemainingBudget <= 0)
	{
		return false;
	}

	TSet<FName> UniqueReceivers;
	for (const FName ReceiverId : Confirmation.ReceiverIds)
	{
		if (ReceiverId.IsNone() || ReceiverId == Confirmation.ReporterId || UniqueReceivers.Contains(ReceiverId))
		{
			++OutResult.RejectedReceivers;
			continue;
		}
		if (UniqueReceivers.Num() >= ZLSocialEventLimits::MaxFanOut || RemainingBudget <= 0)
		{
			++OutResult.RejectedReceivers;
			continue;
		}

		UniqueReceivers.Add(ReceiverId);
		--RemainingBudget;
		FZLSocialEvent& Derived = OutResult.DerivedEvents.AddDefaulted_GetRef();
		Derived.EventId = FGuid::NewGuid();
		Derived.RootEventId = Source.RootEventId;
		Derived.ParentEventId = Source.EventId;
		Derived.CausationId = Confirmation.CausationId;
		Derived.Type = Source.Type;
		Derived.SourceId = Source.SourceId;
		Derived.TargetId = Source.TargetId;
		Derived.ReporterId = Confirmation.ReporterId;
		Derived.SocialReceiverId = ReceiverId;
		Derived.Position = Source.Position;
		Derived.Radius = 0.0f;
		Derived.Severity = Source.Severity;
		Derived.Noise = 0.0f;
		Derived.Channels = static_cast<int32>(EZLSocialPerceptionChannel::Social);
		Derived.ChainDepth = Source.ChainDepth + 1;
		Derived.ChainBudget = RemainingBudget;
		Derived.Confidence = FMath::Clamp(Source.Confidence * Confirmation.ReporterConfidence * ConfidenceDecay, 0.0f, 1.0f);
		Derived.bAnchored = Confirmation.bAnchorDerivedEvents;
		Derived.CreatedAtSeconds = Confirmation.ConfirmedAtSeconds;
		Derived.ExpiresAtSeconds = Source.ExpiresAtSeconds;
	}

	OutResult.RemainingRootBudget = RemainingBudget;
	if (OutResult.DerivedEvents.IsEmpty())
	{
		return false;
	}
	Reporters.Add(Confirmation.ReporterId);
	return true;
}

void FZLSocialPropagation::Reset()
{
	ReportersByRoot.Reset();
	RemainingBudgetByRoot.Reset();
}

bool FZLSocialPropagation::HasReporterReported(const FGuid RootEventId, const FName ReporterId) const
{
	const TSet<FName>* Reporters = ReportersByRoot.Find(RootEventId);
	return Reporters != nullptr && Reporters->Contains(ReporterId);
}
