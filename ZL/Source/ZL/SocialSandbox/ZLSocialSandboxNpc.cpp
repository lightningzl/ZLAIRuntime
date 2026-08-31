#include "SocialSandbox/ZLSocialSandboxNpc.h"

#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"

AZLSocialSandboxNpc::AZLSocialSandboxNpc()
{
	PrimaryActorTick.bCanEverTick = false;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(44.0f, 96.0f);
	Capsule->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	SetRootComponent(Capsule);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Capsule);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.65f, 0.65f, 1.9f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
	}

	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	FacingArrow->SetupAttachment(Capsule);
	FacingArrow->SetRelativeLocation(FVector(72.0f, 0.0f, 25.0f));
	FacingArrow->ArrowColor = FColor(255, 185, 45);
	FacingArrow->ArrowSize = 2.2f;
	FacingArrow->SetHiddenInGame(false);

	NameLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameLabel"));
	NameLabel->SetupAttachment(Capsule);
	NameLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 135.0f));
	NameLabel->SetHorizontalAlignment(EHTA_Center);
	NameLabel->SetWorldSize(34.0f);
	NameLabel->SetTextRenderColor(FColor::White);
}

void AZLSocialSandboxNpc::InitializeSandboxNpc(const FName InStableId, const FText& InDisplayName, const FTransform& InStartTransform)
{
	StableId = InStableId;
	DisplayName = InDisplayName;
	SandboxStartTransform = InStartTransform;
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
	NameLabel->SetText(DisplayName);
}

void AZLSocialSandboxNpc::ResetToSandboxStart()
{
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
}

FVector AZLSocialSandboxNpc::GetPlanarForwardVector() const
{
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.0f;
	return Forward.GetSafeNormal();
}
