#pragma once

#include "CoreMinimal.h"

struct FZLSocialSandboxAttackValidationContext
{
	float Distance = 0.0f;
	double NowSeconds = 0.0;
	double LastAttackSeconds = -DBL_MAX;
	bool bTargetValid = false;
	bool bPlayerExecutable = false;
	bool bTargetIncapacitated = false;
};

struct FZLSocialSandboxAttackValidationResult
{
	FName ReasonCode;
	bool bAccepted = false;
};

class FZLSocialSandboxCombat
{
public:
	static constexpr float AttackRange = 220.0f;
	static constexpr float AttackDamage = 25.0f;
	static constexpr double AttackCooldownSeconds = 1.0;

	static FZLSocialSandboxAttackValidationResult ValidatePlayerAttack(
		const FZLSocialSandboxAttackValidationContext& Context);
};
