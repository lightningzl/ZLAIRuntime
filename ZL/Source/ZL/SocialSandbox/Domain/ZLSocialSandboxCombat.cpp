#include "SocialSandbox/Domain/ZLSocialSandboxCombat.h"

FZLSocialSandboxAttackValidationResult FZLSocialSandboxCombat::ValidatePlayerAttack(
	const FZLSocialSandboxAttackValidationContext& Context)
{
	FZLSocialSandboxAttackValidationResult Result;
	auto Reject = [&Result](const FName Reason)
	{
		Result.ReasonCode = Reason;
		return Result;
	};
	if (!Context.bTargetValid)
	{
		return Reject(TEXT("InvalidTarget"));
	}
	if (!Context.bPlayerExecutable)
	{
		return Reject(TEXT("PlayerBusy"));
	}
	if (Context.bTargetIncapacitated)
	{
		return Reject(TEXT("TargetIncapacitated"));
	}
	if (!FMath::IsFinite(Context.Distance) || Context.Distance < 0.0f || Context.Distance > AttackRange)
	{
		return Reject(TEXT("OutOfAttackRange"));
	}
	if (!FMath::IsFinite(Context.NowSeconds) || Context.NowSeconds < 0.0
		|| Context.NowSeconds - Context.LastAttackSeconds < AttackCooldownSeconds)
	{
		return Reject(TEXT("AttackCooldown"));
	}
	Result.bAccepted = true;
	Result.ReasonCode = TEXT("Accepted");
	return Result;
}
