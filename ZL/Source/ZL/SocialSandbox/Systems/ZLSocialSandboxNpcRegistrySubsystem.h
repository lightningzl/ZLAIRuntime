#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ZLSocialSandboxNpcRegistrySubsystem.generated.h"

class AZLSocialSandboxNpc;

/** 世界子系统：统一登记场景生成的 NPC，并按稳定标识查找或重置它们。 */
UCLASS()
class ZL_API UZLSocialSandboxNpcRegistrySubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool RegisterNpc(AZLSocialSandboxNpc* Npc);
	AZLSocialSandboxNpc* FindNpc(FName StableId) const;
	const TArray<TObjectPtr<AZLSocialSandboxNpc>>& GetNpcs() const { return Npcs; }
	void ResetNpcStates();

private:
	UPROPERTY()
	TArray<TObjectPtr<AZLSocialSandboxNpc>> Npcs;
};
