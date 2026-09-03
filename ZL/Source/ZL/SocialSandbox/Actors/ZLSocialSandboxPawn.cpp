#include "SocialSandbox/Actors/ZLSocialSandboxPawn.h"

#include "SocialSandbox/Domain/ZLSocialSandboxMotion.h"
#include "SocialSandbox/Domain/ZLSocialSandboxPreset.h"
#include "SocialSandbox/UI/ZLSocialBubbleWidget.h"
#include "SocialSandbox/UI/ZLSocialNameWidget.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPlayerController.h"
#include "SocialSandbox/World/ZLSocialSandboxGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

AZLSocialSandboxPawn::AZLSocialSandboxPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	OnAttackMontageEnded.BindUObject(this, &AZLSocialSandboxPawn::AttackMontageEnded);
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1800.0f;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
	SandboxDisplayName = Preset.DisplayName;
	SandboxStartTransform = Preset.SpawnTransform;
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
	if (GetMesh()->GetNumMaterials() > 0)
	{
		if (UMaterialInstanceDynamic* Material = GetMesh()->CreateDynamicMaterialInstance(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), Preset.BodyColor);
		}
	}
	NameWidget->InitWidget();
	if (UZLSocialNameWidget* Widget = Cast<UZLSocialNameWidget>(NameWidget->GetUserWidgetObject()))
	{
		Widget->SetName(SandboxDisplayName, Preset.BodyColor);
	}
}

void AZLSocialSandboxPawn::BeginPlay()
{
	Super::BeginPlay();
	SandboxStartTransform = GetActorTransform();
	if (GetMesh()->GetNumMaterials() > 0)
	{
		if (UMaterialInstanceDynamic* Material = GetMesh()->CreateDynamicMaterialInstance(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.03f, 0.32f, 0.85f));
		}
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
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveInputAction != nullptr) { EnhancedInput->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AZLSocialSandboxPawn::MoveFromInput); }
		if (LookInputAction != nullptr) { EnhancedInput->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AZLSocialSandboxPawn::LookFromInput); }
		if (AttackInputAction != nullptr) { EnhancedInput->BindAction(AttackInputAction, ETriggerEvent::Started, this, &AZLSocialSandboxPawn::ComboAttackPressed); }
		if (ChargedAttackInputAction != nullptr)
		{
			EnhancedInput->BindAction(ChargedAttackInputAction, ETriggerEvent::Started, this, &AZLSocialSandboxPawn::ChargedAttackPressed);
			EnhancedInput->BindAction(ChargedAttackInputAction, ETriggerEvent::Completed, this, &AZLSocialSandboxPawn::ChargedAttackReleased);
		}
	}
}

void AZLSocialSandboxPawn::ComboAttackPressed()
{
	if (bIsAttacking)
	{
		bQueuedCombo = true;
		return;
	}
	const AZLSocialSandboxPlayerController* SandboxController = Cast<AZLSocialSandboxPlayerController>(Controller);
	AZLSocialSandboxGameMode* GameMode = GetWorld() == nullptr ? nullptr : GetWorld()->GetAuthGameMode<AZLSocialSandboxGameMode>();
	if (SandboxController != nullptr && GameMode != nullptr && SandboxController->GetSandboxWidget() != nullptr)
	{
		GameMode->BeginPlayerAttack(this, SandboxController->GetSandboxWidget()->GetSelectedTargetId(), false);
	}
}

void AZLSocialSandboxPawn::ChargedAttackPressed()
{
	if (bIsAttacking)
	{
		bChargingAttack = true;
		return;
	}
	const AZLSocialSandboxPlayerController* SandboxController = Cast<AZLSocialSandboxPlayerController>(Controller);
	AZLSocialSandboxGameMode* GameMode = GetWorld() == nullptr ? nullptr : GetWorld()->GetAuthGameMode<AZLSocialSandboxGameMode>();
	if (SandboxController != nullptr && GameMode != nullptr && SandboxController->GetSandboxWidget() != nullptr)
	{
		GameMode->BeginPlayerAttack(this, SandboxController->GetSandboxWidget()->GetSelectedTargetId(), true);
	}
}

void AZLSocialSandboxPawn::ChargedAttackReleased()
{
	ReleaseSandboxChargedAttack();
}

bool AZLSocialSandboxPawn::PlayAttackMontage(UAnimMontage* Montage, const FName Section)
{
	if (Montage == nullptr) { return false; }
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->Montage_Play(Montage, MontagePlayRate) > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, Montage);
			if (Section != NAME_None) { AnimInstance->Montage_JumpToSection(Section, Montage); }
			return true;
		}
	}
	return false;
}

bool AZLSocialSandboxPawn::StartSandboxComboAttack(AZLSocialSandboxNpc* Target)
{
	if (bIsAttacking || !IsValid(Target)) { return false; }
	PendingAttackTarget = Target;
	ComboIndex = 0;
	bQueuedCombo = false;
	bPendingAttackHitConsumed = false;
	bIsAttacking = PlayAttackMontage(AttackMontage, ComboSections.IsValidIndex(0) ? ComboSections[0] : AttackMontageSection);
	return bIsAttacking;
}

bool AZLSocialSandboxPawn::StartSandboxChargedAttack(AZLSocialSandboxNpc* Target)
{
	if (bIsAttacking || !IsValid(Target)) { return false; }
	PendingAttackTarget = Target;
	bChargingAttack = true;
	bChargeLoopReached = false;
	bPendingAttackHitConsumed = false;
	bIsAttacking = PlayAttackMontage(ChargedAttackMontage, NAME_None);
	return bIsAttacking;
}

void AZLSocialSandboxPawn::ReleaseSandboxChargedAttack()
{
	bChargingAttack = false;
	if (bIsAttacking && bChargeLoopReached) { CheckChargedAttack(); }
}

bool AZLSocialSandboxPawn::ConsumePendingAttackHit()
{
	if (!bIsAttacking || bPendingAttackHitConsumed || !PendingAttackTarget.IsValid()) { return false; }
	bPendingAttackHitConsumed = true;
	return true;
}

void AZLSocialSandboxPawn::DoAttackTrace(FName)
{
	if (AZLSocialSandboxGameMode* GameMode = GetWorld() == nullptr ? nullptr : GetWorld()->GetAuthGameMode<AZLSocialSandboxGameMode>())
	{
		GameMode->ResolvePlayerAttackFromAnimNotify(this);
	}
}

void AZLSocialSandboxPawn::CheckCombo()
{
	if (!bIsAttacking || !bQueuedCombo) { return; }
	if (UAnimInstance* ExistingInstance = GetMesh()->GetAnimInstance())
	{
		if (ChargedAttackMontage != nullptr && ExistingInstance->Montage_IsPlaying(ChargedAttackMontage)) { return; }
	}
	bQueuedCombo = false;
	++ComboIndex;
	if (!ComboSections.IsValidIndex(ComboIndex)) { return; }
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		bPendingAttackHitConsumed = false;
		AnimInstance->Montage_JumpToSection(ComboSections[ComboIndex], AttackMontage);
	}
}

void AZLSocialSandboxPawn::CheckChargedAttack()
{
	if (!bIsAttacking || ChargedAttackMontage == nullptr) { return; }
	bChargeLoopReached = true;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (bChargingAttack) { AnimInstance->Montage_JumpToSection(ChargeLoopSection, ChargedAttackMontage); }
		else { bPendingAttackHitConsumed = false; AnimInstance->Montage_JumpToSection(ChargeAttackSection, ChargedAttackMontage); }
	}
}

void AZLSocialSandboxPawn::AttackMontageEnded(UAnimMontage*, bool)
{
	bIsAttacking = false;
	bQueuedCombo = false;
	bChargingAttack = false;
	bChargeLoopReached = false;
	bPendingAttackHitConsumed = false;
	PendingAttackTarget.Reset();
}

void AZLSocialSandboxPawn::PlaySandboxAttackPresentation_Implementation(AActor*)
{
	if (AttackMontage != nullptr)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (AnimInstance->Montage_Play(AttackMontage, MontagePlayRate) > 0.0f && AttackMontageSection != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(AttackMontageSection, AttackMontage);
			}
		}
	}
}

void AZLSocialSandboxPawn::PlaySandboxHitPresentation_Implementation(AActor*, float, bool)
{
	if (HitMontage != nullptr)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (AnimInstance->Montage_Play(HitMontage, MontagePlayRate) > 0.0f && HitMontageSection != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(HitMontageSection, HitMontage);
			}
		}
	}
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

void AZLSocialSandboxPawn::MoveFromInput(const FInputActionValue& Value)
{
	const FVector2D Movement = Value.Get<FVector2D>();
	MoveForward(Movement.Y);
	MoveRight(Movement.X);
}

void AZLSocialSandboxPawn::LookFromInput(const FInputActionValue& Value)
{
	const FVector2D Look = Value.Get<FVector2D>();
	Turn(Look.X);
	LookUp(Look.Y);
}
