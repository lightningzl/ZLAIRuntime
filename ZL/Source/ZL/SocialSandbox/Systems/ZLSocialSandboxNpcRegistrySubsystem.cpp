#include "SocialSandbox/Systems/ZLSocialSandboxNpcRegistrySubsystem.h"

#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"

bool UZLSocialSandboxNpcRegistrySubsystem::RegisterNpc(AZLSocialSandboxNpc* Npc)
{
	if (!IsValid(Npc) || Npc->GetStableId().IsNone() || FindNpc(Npc->GetStableId()) != nullptr)
	{
		return false;
	}
	Npcs.Add(Npc);
	return true;
}

AZLSocialSandboxNpc* UZLSocialSandboxNpcRegistrySubsystem::FindNpc(const FName StableId) const
{
	for (AZLSocialSandboxNpc* Npc : Npcs)
	{
		if (IsValid(Npc) && Npc->GetStableId() == StableId)
		{
			return Npc;
		}
	}
	return nullptr;
}

void UZLSocialSandboxNpcRegistrySubsystem::ResetNpcStates()
{
	for (AZLSocialSandboxNpc* Npc : Npcs)
	{
		if (IsValid(Npc))
		{
			Npc->ResetToSandboxStart();
		}
	}
}
