#include "ZLSocialToolRegistry.h"

const FName ZLSocialToolReason::Accepted(TEXT("Accepted"));
const FName ZLSocialToolReason::UnknownTool(TEXT("UnknownTool"));
const FName ZLSocialToolReason::InvalidCallId(TEXT("InvalidCallId"));
const FName ZLSocialToolReason::MissingCapability(TEXT("MissingCapability"));
const FName ZLSocialToolReason::InvalidTarget(TEXT("InvalidTarget"));
const FName ZLSocialToolReason::StateVersionMismatch(TEXT("StateVersionMismatch"));
const FName ZLSocialToolReason::Expired(TEXT("Expired"));
const FName ZLSocialToolReason::DistanceExceeded(TEXT("DistanceExceeded"));
const FName ZLSocialToolReason::NavigationUnavailable(TEXT("NavigationUnavailable"));
const FName ZLSocialToolReason::InvalidState(TEXT("InvalidState"));
const FName ZLSocialToolReason::Cooldown(TEXT("Cooldown"));
const FName ZLSocialToolReason::RateLimited(TEXT("RateLimited"));
const FName ZLSocialToolReason::DuplicateCall(TEXT("DuplicateCall"));

bool FZLSocialToolRegistry::Register(const FZLSocialToolDefinition& Definition)
{
	if (Definition.Name.IsNone() || Definition.RequiredCapability.IsNone()
		|| !FMath::IsFinite(Definition.MaxDistance) || Definition.MaxDistance < 0.0f
		|| !FMath::IsFinite(Definition.CooldownSeconds) || Definition.CooldownSeconds < 0.0f
		|| (!Definitions.Contains(Definition.Name) && Definitions.Num() >= MaxDefinitions))
	{
		return false;
	}
	Definitions.Add(Definition.Name, Definition);
	return true;
}

void FZLSocialToolRegistry::RegisterMilestone8Defaults()
{
	auto Add = [this](const TCHAR* Name, const TCHAR* Capability, const bool bTarget, const bool bNavigation)
	{
		FZLSocialToolDefinition Definition;
		Definition.Name = Name;
		Definition.RequiredCapability = Capability;
		Definition.bRequiresTarget = bTarget;
		Definition.bRequiresNavigation = bNavigation;
		Definition.MaxDistance = bTarget ? 3000.0f : 0.0f;
		Definition.CooldownSeconds = 0.5f;
		Register(Definition);
	};
	Add(TEXT("face_target"), TEXT("Tool.FaceTarget"), true, false);
	Add(TEXT("move_toward"), TEXT("Tool.MoveToward"), true, true);
	Add(TEXT("move_away"), TEXT("Tool.MoveAway"), true, true);
	Add(TEXT("stop"), TEXT("Tool.Stop"), false, false);
}

FZLSocialToolValidationResult FZLSocialToolRegistry::ValidateAndCommit(
	const FZLSocialToolCall& Call,
	const FZLSocialToolValidationContext& Context)
{
	const FZLSocialToolDefinition* Definition = Definitions.Find(Call.Name);
	if (Definition == nullptr) { return Reject(ZLSocialToolReason::UnknownTool); }
	if (Call.CallId.TrimStartAndEnd().IsEmpty() || Call.CallId.Len() > 128)
	{
		return Reject(ZLSocialToolReason::InvalidCallId);
	}
	if (CompletedCallIds.Contains(Call.CallId)) { return Reject(ZLSocialToolReason::DuplicateCall); }
	if (!Context.Capabilities.Contains(Definition->RequiredCapability))
	{
		return Reject(ZLSocialToolReason::MissingCapability);
	}
	if ((Definition->bRequiresTarget && (Call.TargetId.IsNone() || !Context.bTargetValid))
		|| (!Definition->bRequiresTarget && !Call.TargetId.IsNone()))
	{
		return Reject(ZLSocialToolReason::InvalidTarget);
	}
	if (Call.StateVersion != Context.CurrentStateVersion)
	{
		return Reject(ZLSocialToolReason::StateVersionMismatch);
	}
	if (!Context.bRequestFresh) { return Reject(ZLSocialToolReason::Expired); }
	if (Definition->bRequiresTarget
		&& (!FMath::IsFinite(Context.DistanceToTarget)
			|| Context.DistanceToTarget < 0.0f
			|| Context.DistanceToTarget > Definition->MaxDistance))
	{
		return Reject(ZLSocialToolReason::DistanceExceeded);
	}
	if (Definition->bRequiresNavigation && !Context.bNavigationReachable)
	{
		return Reject(ZLSocialToolReason::NavigationUnavailable);
	}
	if (!Context.bExecutable) { return Reject(ZLSocialToolReason::InvalidState); }
	if (Context.ExecutionsInWindow >= MaxExecutionsPerWindow)
	{
		return Reject(ZLSocialToolReason::RateLimited);
	}
	if (const double* LastExecution = LastExecutionSeconds.Find(Call.Name))
	{
		if (Context.NowSeconds - *LastExecution < Definition->CooldownSeconds)
		{
			return Reject(ZLSocialToolReason::Cooldown);
		}
	}

	RememberCall(Call.CallId);
	LastExecutionSeconds.Add(Call.Name, Context.NowSeconds);
	FZLSocialToolValidationResult Result;
	Result.bAccepted = true;
	Result.ReasonCode = ZLSocialToolReason::Accepted;
	return Result;
}

FZLSocialToolValidationResult FZLSocialToolRegistry::Reject(const FName ReasonCode)
{
	FZLSocialToolValidationResult Result;
	Result.ReasonCode = ReasonCode;
	return Result;
}

void FZLSocialToolRegistry::RememberCall(const FString& CallId)
{
	if (CompletedCallOrder.Num() >= MaxRememberedCalls)
	{
		CompletedCallIds.Remove(CompletedCallOrder[0]);
		CompletedCallOrder.RemoveAt(0, 1, EAllowShrinking::No);
	}
	CompletedCallIds.Add(CallId);
	CompletedCallOrder.Add(CallId);
}
