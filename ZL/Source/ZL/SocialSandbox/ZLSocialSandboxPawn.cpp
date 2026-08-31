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
	PrimaryActorTick.bCanEverTick = true;
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
	StopScriptedAction();
	GetCharacterMovement()->StopMovementImmediately();
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
	if (Controller != nullptr)
	{
		Controller->SetControlRotation(SandboxStartTransform.Rotator());
	}
}

bool AZLSocialSandboxPawn::StartScriptedAction(const EZLSocialActionType Action, AActor* Target, TFunction<void()> OnCompleted)
{
	if ((Action == EZLSocialActionType::Approach || Action == EZLSocialActionType::MoveAway) && !IsValid(Target))
	{
		return false;
	}
	StopScriptedAction();
	ScriptedAction = Action;
	ScriptedTarget = Target;
	ScriptedCompletion = MoveTemp(OnCompleted);
	bScriptedActionActive = Action == EZLSocialActionType::Approach || Action == EZLSocialActionType::MoveAway;
	return true;
}

void AZLSocialSandboxPawn::StopScriptedAction()
{
	bScriptedActionActive = false;
	ScriptedTarget.Reset();
	ScriptedCompletion = nullptr;
	GetCharacterMovement()->StopMovementImmediately();
}

void AZLSocialSandboxPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bScriptedActionActive || !ScriptedTarget.IsValid())
	{
		return;
	}
	FVector DeltaToTarget = ScriptedTarget->GetActorLocation() - GetActorLocation();
	DeltaToTarget.Z = 0.0f;
	const float Distance = DeltaToTarget.Size();
	const float DesiredDistance = ScriptedAction == EZLSocialActionType::Approach ? 180.0f : 650.0f;
	const bool bComplete = ScriptedAction == EZLSocialActionType::Approach ? Distance <= DesiredDistance : Distance >= DesiredDistance;
	if (bComplete)
	{
		bScriptedActionActive = false;
		ScriptedTarget.Reset();
		TFunction<void()> Completion = MoveTemp(ScriptedCompletion);
		ScriptedCompletion = nullptr;
		if (Completion) { Completion(); }
		return;
	}

	FVector Direction = DeltaToTarget.GetSafeNormal();
	if (ScriptedAction == EZLSocialActionType::MoveAway)
	{
		Direction *= -1.0f;
	}
	const float MaxStep = FMath::Max(0.0f, DeltaSeconds) * ScriptedSpeed;
	const float Remaining = FMath::Abs(Distance - DesiredDistance);
	const FVector Step = Direction * FMath::Min(MaxStep, Remaining);
	SetActorRotation(Direction.Rotation());
	if (Controller != nullptr) { Controller->SetControlRotation(Direction.Rotation()); }
	AddActorWorldOffset(Step, true);
}

void AZLSocialSandboxPawn::MoveForward(const float Value)
{
	if (!bScriptedActionActive && !FMath::IsNearlyZero(Value))
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AZLSocialSandboxPawn::MoveRight(const float Value)
{
	if (!bScriptedActionActive && !FMath::IsNearlyZero(Value))
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
