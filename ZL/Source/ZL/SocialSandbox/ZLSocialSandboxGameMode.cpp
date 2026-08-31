#include "SocialSandbox/ZLSocialSandboxGameMode.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "SocialSandbox/ZLSocialSandboxNpc.h"
#include "SocialSandbox/ZLSocialSandboxPawn.h"

AZLSocialSandboxGameMode::AZLSocialSandboxGameMode()
{
	DefaultPawnClass = AZLSocialSandboxPawn::StaticClass();
}

void AZLSocialSandboxGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnEnvironment();
	SpawnNpc(TEXT("npc_guard"), TEXT("守卫 Guard"), FVector(350.0f, 0.0f, 96.0f), FRotator(0.0f, 180.0f, 0.0f));
	SpawnNpc(TEXT("npc_merchant"), TEXT("商人 Merchant"), FVector(600.0f, 500.0f, 96.0f), FRotator(0.0f, 225.0f, 0.0f));
	SpawnNpc(TEXT("npc_scout"), TEXT("斥候 Scout"), FVector(-450.0f, 250.0f, 96.0f), FRotator(0.0f, 0.0f, 0.0f));
	SpawnNpc(TEXT("npc_civilian"), TEXT("居民 Civilian"), FVector(1250.0f, -500.0f, 96.0f), FRotator(0.0f, 135.0f, 0.0f));
}

void AZLSocialSandboxGameMode::ResetSocialSandbox()
{
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (IsValid(Npc))
		{
			Npc->ResetToSandboxStart();
		}
	}
	if (AZLSocialSandboxPawn* Pawn = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Pawn->ResetToSandboxStart();
	}
}

AZLSocialSandboxNpc* AZLSocialSandboxGameMode::FindSandboxNpc(const FName StableId) const
{
	for (AZLSocialSandboxNpc* Npc : SandboxNpcs)
	{
		if (IsValid(Npc) && Npc->GetStableId() == StableId)
		{
			return Npc;
		}
	}
	return nullptr;
}

void AZLSocialSandboxGameMode::SpawnEnvironment()
{
	if (GetWorld() == nullptr)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Name = TEXT("SocialSandboxFloor");
	AStaticMeshActor* Floor = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0.0f, 0.0f, -55.0f), FRotator::ZeroRotator, Params);
	if (Floor != nullptr)
	{
		Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
		Floor->SetActorScale3D(FVector(35.0f, 35.0f, 1.0f));
		Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
	}

	if (!GetWorld()->GetAuthGameMode()->FindPlayerStart(nullptr))
	{
		GetWorld()->SpawnActor<APlayerStart>(FVector::ZeroVector, FRotator::ZeroRotator);
	}

	ADirectionalLight* Light = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-50.0f, -35.0f, 0.0f));
	if (Light != nullptr)
	{
		Light->SetMobility(EComponentMobility::Movable);
	}
}

void AZLSocialSandboxGameMode::SpawnNpc(const FName StableId, const TCHAR* DisplayName, const FVector& Location, const FRotator& Rotation)
{
	FActorSpawnParameters Params;
	Params.Name = StableId;
	AZLSocialSandboxNpc* Npc = GetWorld()->SpawnActor<AZLSocialSandboxNpc>(AZLSocialSandboxNpc::StaticClass(), Location, Rotation, Params);
	if (Npc != nullptr)
	{
		Npc->InitializeSandboxNpc(StableId, FText::FromString(DisplayName), FTransform(Rotation, Location));
		SandboxNpcs.Add(Npc);
	}
}
