#include "SocialSandbox/Actors/ZLSocialSandboxNpcSpawner.h"

#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/Domain/ZLSocialSandboxPersonaAdapter.h"
#include "ZLSocialPersona.h"
#include "ZLSocialPersonaSettings.h"

#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

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
	FZLSocialPersonaData Persona;
	if (!GetWorld() || !NpcClass || (PersonaAsset == nullptr && PersonaId.IsNone()) || (PersonaAsset != nullptr && !PersonaId.IsNone()))
	{
		LastSpawnResult = TEXT("Spawner requires exactly one valid Persona Asset or Persona ID and an NPC class");
		return false;
	}
	if (PersonaAsset) { Persona = PersonaAsset->Persona; }
	else if (!GetDefault<UZLSocialPersonaSettings>()->ResolveConfiguredPersona(PersonaId, Persona, Error)) { LastSpawnResult = Error; return false; }
	if (!FZLSocialSandboxPersonaAdapter::ToNpcProfile(Persona, BodyColor, Profile, Error)) { LastSpawnResult = Error; return false; }

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

TArray<FString> AZLSocialSandboxNpcSpawner::GetConfiguredPersonaIdOptions() const
{
	TArray<FName> PersonaIds;
	GetDefault<UZLSocialPersonaSettings>()->GetConfiguredPersonaIds(PersonaIds);

	TArray<FString> Options;
	Options.Reserve(PersonaIds.Num());
	for (const FName PersonaIdOption : PersonaIds)
	{
		Options.Add(PersonaIdOption.ToString());
	}
	return Options;
}

#if WITH_EDITOR
void AZLSocialSandboxNpcSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName ChangedPropertyName = PropertyChangedEvent.GetPropertyName();
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AZLSocialSandboxNpcSpawner, PersonaAsset) && PersonaAsset)
	{
		PersonaId = NAME_None;
	}
	else if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AZLSocialSandboxNpcSpawner, PersonaId) && !PersonaId.IsNone())
	{
		PersonaAsset = nullptr;
	}
}
#endif
