#pragma once

#include "CoreMinimal.h"

enum class EZLSocialSandboxConflictLevel : uint8
{
	Calm,
	Alert,
	Escalated,
	Recovering
};

enum class EZLSocialSandboxConflictEvent : uint8
{
	Attack,
	DistanceNear,
	DistanceFar,
	PlayerStop,
	PlannerEngage,
	PlannerDisengage,
	LocalFallback
};

struct FZLSocialSandboxConflictTransition
{
	EZLSocialSandboxConflictLevel Previous = EZLSocialSandboxConflictLevel::Calm;
	EZLSocialSandboxConflictLevel Current = EZLSocialSandboxConflictLevel::Calm;
	bool bChanged = false;
	bool bShouldDefend = false;
};

/** Bounded UE-authoritative public conflict stance for the one Guard. */
class FZLSocialSandboxConflictState
{
public:
	void Reset() { Level = EZLSocialSandboxConflictLevel::Calm; }
	EZLSocialSandboxConflictLevel GetLevel() const { return Level; }
	FZLSocialSandboxConflictTransition Apply(EZLSocialSandboxConflictEvent Event);
	static const TCHAR* LevelName(EZLSocialSandboxConflictLevel Value);

private:
	EZLSocialSandboxConflictLevel Level = EZLSocialSandboxConflictLevel::Calm;
};
