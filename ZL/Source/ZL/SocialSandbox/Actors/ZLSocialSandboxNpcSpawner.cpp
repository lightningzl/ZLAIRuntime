#include "SocialSandbox/Actors/ZLSocialSandboxNpcSpawner.h"

#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/Domain/ZLSocialSandboxPersonaAdapter.h"
#include "ZLSocialPersona.h"

AZLSocialSandboxNpcSpawner::AZLSocialSandboxNpcSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AZLSocialSandboxNpcSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (bSpawnOnBeginPlay)
	{
		SpawnNpc();
	}
}

bool AZLSocialSandboxNpcSpawner::SpawnNpc()
{
	FString Error;
	FZLSocialSandboxNpcProfile Profile;
	if (!GetWorld() || !NpcClass || !PersonaAsset || !FZLSocialSandboxPersonaAdapter::ToNpcProfile(PersonaAsset->Persona, BodyColor, Profile, Error))
	{
		LastSpawnResult = Error.IsEmpty() ? TEXT("Spawner requires a world, NPC class, and valid Persona Asset") : Error;
		return false;
	}

	AZLSocialSandboxNpc* Npc = GetWorld()->SpawnActorDeferred<AZLSocialSandboxNpc>(NpcClass, GetActorTransform(), this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Npc)
	{
		LastSpawnResult = TEXT("NPC spawn failed before initialization");
		return false;
	}
	Npc->InitializeSandboxNpc(Profile, GetActorTransform(), InitialHealth);
	Npc->FinishSpawning(GetActorTransform());
	LastSpawnResult = TEXT("NPC spawn succeeded");
	return true;
}
