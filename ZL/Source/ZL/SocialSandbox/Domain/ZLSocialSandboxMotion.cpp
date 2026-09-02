#include "SocialSandbox/Domain/ZLSocialSandboxMotion.h"

FZLSocialSandboxMotionStep FZLSocialSandboxMotion::Compute(const EZLSocialActionType Action, const FVector& Current, const FVector& Target, const float DeltaSeconds, const float Speed)
{
	FZLSocialSandboxMotionStep Result;
	FVector DeltaToTarget = Target - Current;
	DeltaToTarget.Z = 0.0f;
	const float Distance = DeltaToTarget.Size();
	const float DesiredDistance = Action == EZLSocialActionType::Approach ? 180.0f : 650.0f;
	Result.bComplete = Action == EZLSocialActionType::Approach ? Distance <= DesiredDistance : Distance >= DesiredDistance;
	if (Result.bComplete || DeltaToTarget.IsNearlyZero())
	{
		return Result;
	}
	Result.Facing = DeltaToTarget.GetSafeNormal();
	if (Action == EZLSocialActionType::MoveAway) { Result.Facing *= -1.0f; }
	const float MaxStep = FMath::Max(0.0f, DeltaSeconds) * FMath::Max(0.0f, Speed);
	Result.Translation = Result.Facing * FMath::Min(MaxStep, FMath::Abs(Distance - DesiredDistance));
	return Result;
}
