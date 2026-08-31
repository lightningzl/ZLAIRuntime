#include "SocialSandbox/ZLSocialSandboxPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"

AZLSocialSandboxPawn::AZLSocialSandboxPawn()
{
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1800.0f;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.65f, 0.65f, 1.9f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
	}

	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	FacingArrow->SetupAttachment(GetCapsuleComponent());
	FacingArrow->SetRelativeLocation(FVector(65.0f, 0.0f, 0.0f));
	FacingArrow->ArrowColor = FColor(40, 220, 255);
	FacingArrow->ArrowSize = 2.0f;
	FacingArrow->SetHiddenInGame(false);

	NameLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameLabel"));
	NameLabel->SetupAttachment(GetCapsuleComponent());
	NameLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 135.0f));
	NameLabel->SetHorizontalAlignment(EHTA_Center);
	NameLabel->SetWorldSize(34.0f);
	NameLabel->SetTextRenderColor(FColor(40, 220, 255));
	NameLabel->SetText(FText::FromString(TEXT("玩家 Player")));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = 650.0f;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AZLSocialSandboxPawn::BeginPlay()
{
	Super::BeginPlay();
	SandboxStartTransform = GetActorTransform();
}

void AZLSocialSandboxPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("SandboxMoveForward"), this, &AZLSocialSandboxPawn::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("SandboxMoveRight"), this, &AZLSocialSandboxPawn::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("SandboxTurn"), this, &AZLSocialSandboxPawn::Turn);
	PlayerInputComponent->BindAxis(TEXT("SandboxLookUp"), this, &AZLSocialSandboxPawn::LookUp);
}

void AZLSocialSandboxPawn::ResetToSandboxStart()
{
	GetCharacterMovement()->StopMovementImmediately();
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
	if (Controller != nullptr)
	{
		Controller->SetControlRotation(SandboxStartTransform.Rotator());
	}
}

void AZLSocialSandboxPawn::MoveForward(const float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AZLSocialSandboxPawn::MoveRight(const float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AZLSocialSandboxPawn::Turn(const float Value)
{
	AddControllerYawInput(Value);
}

void AZLSocialSandboxPawn::LookUp(const float Value)
{
	AddControllerPitchInput(Value);
}
