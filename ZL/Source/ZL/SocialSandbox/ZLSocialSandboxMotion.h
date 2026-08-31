#pragma once

#include "CoreMinimal.h"
#include "ZLSocialObservation.h"

struct FZLSocialSandboxMotionStep
{
	FVector Translation = FVector::ZeroVector;
	FVector Facing = FVector::ForwardVector;
	bool bComplete = false;
};

class FZLSocialSandboxMotion
{
public:
	static FZLSocialSandboxMotionStep Compute(EZLSocialActionType Action, const FVector& Current, const FVector& Target, float DeltaSeconds, float Speed);
};
