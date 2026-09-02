#include "SocialSandbox/Domain/ZLSocialSandboxConflictState.h"

FZLSocialSandboxConflictTransition FZLSocialSandboxConflictState::Apply(const EZLSocialSandboxConflictEvent Event)
{
	FZLSocialSandboxConflictTransition Result;
	Result.Previous = Level;
	switch (Event)
	{
	case EZLSocialSandboxConflictEvent::Attack:
	case EZLSocialSandboxConflictEvent::PlannerEngage:
	case EZLSocialSandboxConflictEvent::LocalFallback:
		Level = EZLSocialSandboxConflictLevel::Escalated;
		break;
	case EZLSocialSandboxConflictEvent::DistanceNear:
		if (Level == EZLSocialSandboxConflictLevel::Calm || Level == EZLSocialSandboxConflictLevel::Recovering)
		{
			Level = EZLSocialSandboxConflictLevel::Alert;
		}
		break;
	case EZLSocialSandboxConflictEvent::DistanceFar:
	case EZLSocialSandboxConflictEvent::PlayerStop:
	case EZLSocialSandboxConflictEvent::PlannerDisengage:
		if (Level == EZLSocialSandboxConflictLevel::Escalated || Level == EZLSocialSandboxConflictLevel::Alert)
		{
			Level = EZLSocialSandboxConflictLevel::Recovering;
		}
		else if (Level == EZLSocialSandboxConflictLevel::Recovering)
		{
			Level = EZLSocialSandboxConflictLevel::Calm;
		}
		break;
	default:
		break;
	}
	Result.Current = Level;
	Result.bChanged = Result.Previous != Result.Current;
	Result.bShouldDefend = Level == EZLSocialSandboxConflictLevel::Escalated;
	return Result;
}

const TCHAR* FZLSocialSandboxConflictState::LevelName(const EZLSocialSandboxConflictLevel Value)
{
	switch (Value)
	{
	case EZLSocialSandboxConflictLevel::Alert: return TEXT("警戒");
	case EZLSocialSandboxConflictLevel::Escalated: return TEXT("冲突升级");
	case EZLSocialSandboxConflictLevel::Recovering: return TEXT("缓和中");
	default: return TEXT("平静");
	}
}
