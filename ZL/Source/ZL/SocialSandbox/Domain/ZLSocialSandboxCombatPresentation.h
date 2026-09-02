#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZLSocialSandboxCombatPresentation.generated.h"

UINTERFACE(BlueprintType)
class ZL_API UZLSocialSandboxCombatPresentation : public UInterface
{
	GENERATED_BODY()
};

class ZL_API IZLSocialSandboxCombatPresentation
{
	GENERATED_BODY()

public:
	// Optional presentation hook. Gameplay has already accepted the attack when this fires.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Social Sandbox|Combat Presentation")
	void PlaySandboxAttackPresentation(AActor* Target);

	// Optional presentation hook. Damage has already changed authoritative health when this fires.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Social Sandbox|Combat Presentation")
	void PlaySandboxHitPresentation(AActor* Instigator, float AppliedDamage, bool bIncapacitated);
};
