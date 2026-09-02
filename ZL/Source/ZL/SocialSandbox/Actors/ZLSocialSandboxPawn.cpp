#include "SocialSandbox/Actors/ZLSocialSandboxPawn.h"

#include "SocialSandbox/Domain/ZLSocialSandboxMotion.h"
#include "SocialSandbox/Domain/ZLSocialSandboxPreset.h"
#include "SocialSandbox/UI/ZLSocialBubbleWidget.h"
#include "SocialSandbox/UI/ZLSocialNameWidget.h"

#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
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

	NameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	NameWidget->SetupAttachment(GetCapsuleComponent());
	NameWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 145.0f));
	NameWidget->SetWidgetSpace(EWidgetSpace::Screen);
	NameWidget->SetDrawSize(FVector2D(260.0f, 44.0f));
	NameWidget->SetPivot(FVector2D(0.5f, 0.5f));
	NameWidget->SetWidgetClass(UZLSocialNameWidget::StaticClass());

	BubbleWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("BubbleWidget"));
	BubbleWidget->SetupAttachment(GetCapsuleComponent());
	BubbleWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 210.0f));
	BubbleWidget->SetWidgetSpace(EWidgetSpace::Screen);
	BubbleWidget->SetDrawSize(FVector2D(360.0f, 80.0f));
	BubbleWidget->SetPivot(FVector2D(0.5f, 1.0f));
	BubbleWidget->SetWidgetClass(UZLSocialBubbleWidget::StaticClass());
	BubbleWidget->SetVisibility(false);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = 650.0f;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AZLSocialSandboxPawn::InitializeSandboxPlayer(const FZLSocialSandboxPlayerPreset& Preset)
{
	if (!Preset.IsValid()) { return; }
	SandboxStableId = Preset.StableId;
	SandboxStartTransform = Preset.SpawnTransform;
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
	if (UMaterialInstanceDynamic* Material = BodyMesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Preset.BodyColor);
	}
	NameWidget->InitWidget();
	if (UZLSocialNameWidget* Widget = Cast<UZLSocialNameWidget>(NameWidget->GetUserWidgetObject()))
	{
		Widget->SetName(Preset.DisplayName, Preset.BodyColor);
	}
}

void AZLSocialSandboxPawn::BeginPlay()
{
	Super::BeginPlay();
	SandboxStartTransform = GetActorTransform();
	if (UMaterialInstanceDynamic* Material = BodyMesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.03f, 0.32f, 0.85f));
	}
	NameWidget->InitWidget();
	if (UZLSocialNameWidget* Widget = Cast<UZLSocialNameWidget>(NameWidget->GetUserWidgetObject()))
	{
		Widget->SetName(FText::FromString(TEXT("玩家")), FLinearColor(0.16f, 0.86f, 1.0f));
	}
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
	ClearBubble();
	GetCharacterMovement()->StopMovementImmediately();
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
	if (Controller != nullptr)
	{
		Controller->SetControlRotation(SandboxStartTransform.Rotator());
	}
}

bool AZLSocialSandboxPawn::StartScriptedAction(const EZLSocialActionType Action, AActor* Target, TFunction<void()> OnCompleted)
{
	if (Action == EZLSocialActionType::Attack)
	{
		return false;
	}
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
	const FZLSocialSandboxMotionStep Motion = FZLSocialSandboxMotion::Compute(ScriptedAction, GetActorLocation(), ScriptedTarget->GetActorLocation(), DeltaSeconds, ScriptedSpeed);
	if (Motion.bComplete)
	{
		bScriptedActionActive = false;
		ScriptedTarget.Reset();
		TFunction<void()> Completion = MoveTemp(ScriptedCompletion);
		ScriptedCompletion = nullptr;
		if (Completion) { Completion(); }
		return;
	}

	SetActorRotation(Motion.Facing.Rotation());
	if (Controller != nullptr) { Controller->SetControlRotation(Motion.Facing.Rotation()); }
	AddActorWorldOffset(Motion.Translation, true);
}

void AZLSocialSandboxPawn::ShowSpeechBubble(const FString& SpokenText)
{
	const FString Bounded = SpokenText.Len() > 96 ? SpokenText.Left(96) + TEXT("…") : SpokenText;
	ShowBubble(FText::FromString(Bounded), FColor(40, 220, 255));
}

void AZLSocialSandboxPawn::ShowActionBubble(const EZLSocialActionType Action, const EZLSocialActionPhase Phase, const FText& TargetName)
{
	const TCHAR* ActionText = TEXT("停止");
	switch (Action)
	{
	case EZLSocialActionType::Face: ActionText = TEXT("面向"); break;
	case EZLSocialActionType::Approach: ActionText = TEXT("靠近"); break;
	case EZLSocialActionType::MoveAway: ActionText = TEXT("远离"); break;
	case EZLSocialActionType::Attack: ActionText = TEXT("攻击"); break;
	default: break;
	}
	const TCHAR* PhaseText = Phase == EZLSocialActionPhase::Started ? TEXT("开始") : TEXT("完成");
	const FString TargetSuffix = TargetName.IsEmpty() ? FString() : FString::Printf(TEXT(" → %s"), *TargetName.ToString());
	ShowBubble(FText::FromString(FString::Printf(TEXT("[%s] %s%s"), PhaseText, ActionText, *TargetSuffix)), FColor(120, 220, 255));
}

void AZLSocialSandboxPawn::ShowBubble(const FText& Text, const FColor& Color, const float DurationSeconds)
{
	if (BubbleWidget == nullptr || GetWorld() == nullptr) { return; }
	GetWorld()->GetTimerManager().ClearTimer(BubbleTimer);
	BubbleWidget->InitWidget();
	if (UZLSocialBubbleWidget* Widget = Cast<UZLSocialBubbleWidget>(BubbleWidget->GetUserWidgetObject()))
	{
		Widget->SetBubble(Text, FLinearColor(Color));
	}
	BubbleWidget->SetVisibility(true);
	GetWorld()->GetTimerManager().SetTimer(BubbleTimer, this, &AZLSocialSandboxPawn::ClearBubble, FMath::Clamp(DurationSeconds, 0.5f, 8.0f), false);
}

void AZLSocialSandboxPawn::ClearBubble()
{
	if (GetWorld() != nullptr) { GetWorld()->GetTimerManager().ClearTimer(BubbleTimer); }
	if (BubbleWidget != nullptr) { BubbleWidget->SetVisibility(false); }
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
