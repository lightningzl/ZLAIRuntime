#include "SocialSandbox/ZLSocialSandboxNpc.h"

#include "SocialSandbox/ZLSocialBubbleWidget.h"
#include "SocialSandbox/ZLSocialSandboxMotion.h"

#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AZLSocialSandboxNpc::AZLSocialSandboxNpc()
{
	PrimaryActorTick.bCanEverTick = true;

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

	BubbleWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("BubbleWidget"));
	BubbleWidget->SetupAttachment(Capsule);
	BubbleWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 205.0f));
	BubbleWidget->SetWidgetSpace(EWidgetSpace::Screen);
	BubbleWidget->SetDrawSize(FVector2D(320.0f, 72.0f));
	BubbleWidget->SetPivot(FVector2D(0.5f, 1.0f));
	BubbleWidget->SetWidgetClass(UZLSocialBubbleWidget::StaticClass());
	BubbleWidget->SetVisibility(false);
}

void AZLSocialSandboxNpc::InitializeSandboxNpc(const FName InStableId, const FText& InDisplayName, const FTransform& InStartTransform)
{
	StableId = InStableId;
	DisplayName = InDisplayName;
	SandboxStartTransform = InStartTransform;
	StateVersion = 1;
	Health = MaxHealth;
	LastDamageSeconds = -DBL_MAX;
	bDefending = false;
	bIncapacitated = false;
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
	RefreshNameLabel();
	if (UMaterialInstanceDynamic* Material = BodyMesh->CreateDynamicMaterialInstance(0))
	{
		const uint32 Hash = GetTypeHash(StableId);
		const FLinearColor Palette[] = {
			FLinearColor(0.85f, 0.18f, 0.08f),
			FLinearColor(0.90f, 0.48f, 0.05f),
			FLinearColor(0.18f, 0.62f, 0.22f),
			FLinearColor(0.55f, 0.18f, 0.75f)
		};
		Material->SetVectorParameterValue(TEXT("Color"), Palette[Hash % UE_ARRAY_COUNT(Palette)]);
	}
	const TCHAR* MeshPaths[] = {
		TEXT("/Engine/BasicShapes/Cube.Cube"),
		TEXT("/Engine/BasicShapes/Cone.Cone"),
		TEXT("/Engine/BasicShapes/Sphere.Sphere"),
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder")
	};
	if (UStaticMesh* Shape = LoadObject<UStaticMesh>(nullptr, MeshPaths[GetTypeHash(StableId) % UE_ARRAY_COUNT(MeshPaths)]))
	{
		BodyMesh->SetStaticMesh(Shape);
		BodyMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.8f));
	}
}

void AZLSocialSandboxNpc::ResetToSandboxStart()
{
	StopDecisionAction();
	SetActorTransform(SandboxStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
	++StateVersion;
	ObservationBuffer.Reset();
	LastDecisionSpeech.Reset();
	Health = MaxHealth;
	LastDamageSeconds = -DBL_MAX;
	bDefending = false;
	bIncapacitated = false;
	RefreshNameLabel();
	ClearBubble();
}

bool AZLSocialSandboxNpc::ApplySandboxDamage(
	const float RawDamage,
	const double NowSeconds,
	FZLSocialSandboxDamageResult& OutResult)
{
	OutResult = FZLSocialSandboxDamageResult();
	OutResult.HealthBefore = Health;
	OutResult.HealthAfter = Health;
	if (!FMath::IsFinite(RawDamage) || RawDamage <= 0.0f || !FMath::IsFinite(NowSeconds) || NowSeconds < 0.0)
	{
		OutResult.ReasonCode = TEXT("InvalidDamage");
		return false;
	}
	if (bIncapacitated)
	{
		OutResult.ReasonCode = TEXT("TargetIncapacitated");
		return false;
	}
	if (NowSeconds - LastDamageSeconds < 0.35)
	{
		OutResult.ReasonCode = TEXT("DamageInvulnerable");
		return false;
	}

	const bool bWasDefending = bDefending;
	const float AppliedDamage = bWasDefending
		? FMath::Max(1.0f, FMath::CeilToFloat(RawDamage * 0.35f))
		: RawDamage;
	Health = FMath::Clamp(Health - AppliedDamage, 0.0f, MaxHealth);
	LastDamageSeconds = NowSeconds;
	bIncapacitated = Health <= 0.0f;
	if (bIncapacitated)
	{
		StopDecisionAction();
		bDefending = false;
	}
	++StateVersion;
	RefreshNameLabel();
	OutResult.ReasonCode = TEXT("Accepted");
	OutResult.HealthAfter = Health;
	OutResult.AppliedDamage = OutResult.HealthBefore - OutResult.HealthAfter;
	OutResult.bAccepted = true;
	OutResult.bDefended = bWasDefending;
	OutResult.bIncapacitated = bIncapacitated;
	return true;
}

void AZLSocialSandboxNpc::SetDefending(const bool bValue)
{
	const bool bNewValue = bValue && !bIncapacitated;
	if (bDefending == bNewValue)
	{
		return;
	}
	bDefending = bNewValue;
	++StateVersion;
	RefreshNameLabel();
}

FVector AZLSocialSandboxNpc::GetPlanarForwardVector() const
{
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.0f;
	return Forward.GetSafeNormal();
}

void AZLSocialSandboxNpc::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FaceLabelsToCamera();
	AdvanceDecisionAction(DeltaSeconds);
}

void AZLSocialSandboxNpc::AdvanceDecisionAction(const float DeltaSeconds)
{
	if (!bDecisionActionActive || !DecisionTarget.IsValid())
	{
		return;
	}
	const FZLSocialSandboxMotionStep Motion = FZLSocialSandboxMotion::Compute(
		DecisionAction,
		GetActorLocation(),
		DecisionTarget->GetActorLocation(),
		DeltaSeconds,
		DecisionSpeed);
	if (Motion.bComplete)
	{
		bDecisionActionActive = false;
		DecisionTarget.Reset();
		++StateVersion;
		TFunction<void()> Completion = MoveTemp(DecisionCompletion);
		DecisionCompletion = nullptr;
		if (Completion) { Completion(); }
		return;
	}
	SetActorRotation(Motion.Facing.Rotation());
	AddActorWorldOffset(Motion.Translation, true);
}

bool AZLSocialSandboxNpc::StartDecisionAction(
	const EZLSocialActionType Action,
	AActor* Target,
	TFunction<void()> OnCompleted)
{
	if (bIncapacitated)
	{
		return false;
	}
	if (Action == EZLSocialActionType::Attack)
	{
		return false;
	}
	const bool bNeedsTarget = Action != EZLSocialActionType::Stop;
	if (bNeedsTarget && !IsValid(Target))
	{
		return false;
	}
	FVector FaceDirection = FVector::ZeroVector;
	if (Action == EZLSocialActionType::Face)
	{
		FaceDirection = Target->GetActorLocation() - GetActorLocation();
		FaceDirection.Z = 0.0f;
		if (FaceDirection.IsNearlyZero())
		{
			return false;
		}
	}
	StopDecisionAction();
	DecisionAction = Action;
	DecisionTarget = Target;
	DecisionCompletion = MoveTemp(OnCompleted);
	++StateVersion;

	if (Action == EZLSocialActionType::Face)
	{
		SetActorRotation(FaceDirection.Rotation());
		TFunction<void()> Completion = MoveTemp(DecisionCompletion);
		DecisionTarget.Reset();
		if (Completion) { Completion(); }
		return true;
	}
	if (Action == EZLSocialActionType::Stop)
	{
		TFunction<void()> Completion = MoveTemp(DecisionCompletion);
		DecisionTarget.Reset();
		if (Completion) { Completion(); }
		return true;
	}
	bDecisionActionActive = true;
	return true;
}

void AZLSocialSandboxNpc::StopDecisionAction()
{
	if (bDecisionActionActive)
	{
		++StateVersion;
	}
	bDecisionActionActive = false;
	DecisionTarget.Reset();
	DecisionCompletion = nullptr;
}

void AZLSocialSandboxNpc::ShowRuleSpeech(const FZLSocialObservation& Observation)
{
	if (!Observation.bHeard) { return; }
	const TCHAR* Response = TEXT("[RulePlaceholder] 我听见了，但对象不明。");
	if (Observation.TargetJudgment == EZLSocialTargetJudgment::ExplicitSelf)
	{
		Response = Observation.bHeardClearly ? TEXT("[RulePlaceholder] 我听清了。") : TEXT("[RulePlaceholder] 我听到了。");
	}
	else if (Observation.TargetJudgment == EZLSocialTargetJudgment::Candidate)
	{
		Response = TEXT("[RulePlaceholder] 你是在对我说吗？");
	}
	else if (Observation.TargetJudgment == EZLSocialTargetJudgment::ExplicitOther)
	{
		Response = TEXT("[RulePlaceholder]（旁听）");
	}
	ShowBubble(FText::FromString(Response), FColor(255, 210, 80));
}

void AZLSocialSandboxNpc::ShowActionObservation(const FZLSocialObservation& Observation)
{
	if (!Observation.bSaw) { return; }
	const TCHAR* ActionText = TEXT("停止");
	switch (Observation.Action)
	{
	case EZLSocialActionType::Face: ActionText = TEXT("面向"); break;
	case EZLSocialActionType::Approach: ActionText = TEXT("靠近"); break;
	case EZLSocialActionType::MoveAway: ActionText = TEXT("远离"); break;
	case EZLSocialActionType::Attack: ActionText = TEXT("攻击"); break;
	default: break;
	}
	const TCHAR* PhaseText = Observation.ActionPhase == EZLSocialActionPhase::Started ? TEXT("开始") : TEXT("完成");
	ShowBubble(FText::FromString(FString::Printf(TEXT("[观察] 玩家%s%s"), PhaseText, ActionText)), FColor(255, 170, 70), 3.0f);
}

void AZLSocialSandboxNpc::ShowDecisionSpeech(const FString& Text, const FString& Provider)
{
	const FString Bounded = Text.Len() > 96 ? Text.Left(96) + TEXT("…") : Text;
	const FString Source = Provider.Equals(TEXT("kimi"), ESearchCase::IgnoreCase) ? TEXT("Kimi") : TEXT("Stub");
	LastDecisionSpeech = FString::Printf(TEXT("[%s] %s"), *Source, *Bounded);
	ShowBubble(FText::FromString(LastDecisionSpeech), FColor(90, 235, 170), 6.0f);
}

void AZLSocialSandboxNpc::ShowDecisionAction(const EZLSocialActionType Action, const EZLSocialActionPhase Phase)
{
	const TCHAR* ActionText = TEXT("停止 Stop");
	switch (Action)
	{
	case EZLSocialActionType::Face: ActionText = TEXT("面向 Face"); break;
	case EZLSocialActionType::Approach: ActionText = TEXT("靠近 Approach"); break;
	case EZLSocialActionType::MoveAway: ActionText = TEXT("远离 MoveAway"); break;
	case EZLSocialActionType::Attack: ActionText = TEXT("攻击 Attack"); break;
	default: break;
	}
	const TCHAR* PhaseText = Phase == EZLSocialActionPhase::Started ? TEXT("执行") : TEXT("完成");
	const FString SpeechPrefix = LastDecisionSpeech.IsEmpty() ? FString() : LastDecisionSpeech + TEXT("\n");
	ShowBubble(FText::FromString(FString::Printf(TEXT("%s[Decision %s] %s"), *SpeechPrefix, PhaseText, ActionText)), FColor(90, 235, 170), 5.0f);
}

void AZLSocialSandboxNpc::ShowDecisionRejection(const FName ReasonCode)
{
	const FString SpeechPrefix = LastDecisionSpeech.IsEmpty() ? FString() : LastDecisionSpeech + TEXT("\n");
	ShowBubble(FText::FromString(FString::Printf(TEXT("%s[Tool rejected] %s"), *SpeechPrefix, *ReasonCode.ToString())), FColor(255, 115, 95), 5.0f);
}

void AZLSocialSandboxNpc::ShowDecisionFallback()
{
	LastDecisionSpeech.Reset();
	ShowBubble(FText::FromString(TEXT("[LocalFallback] 我听见了，先保持警戒。")), FColor(255, 210, 80), 5.0f);
}

void AZLSocialSandboxNpc::ShowBubble(const FText& Text, const FColor& Color, const float DurationSeconds)
{
	if (BubbleWidget == nullptr || GetWorld() == nullptr) { return; }
	GetWorld()->GetTimerManager().ClearTimer(BubbleTimer);
	BubbleWidget->InitWidget();
	if (UZLSocialBubbleWidget* Widget = Cast<UZLSocialBubbleWidget>(BubbleWidget->GetUserWidgetObject()))
	{
		Widget->SetBubble(Text, FLinearColor(Color));
	}
	BubbleWidget->SetVisibility(true);
	GetWorld()->GetTimerManager().SetTimer(BubbleTimer, this, &AZLSocialSandboxNpc::ClearBubble, FMath::Clamp(DurationSeconds, 0.5f, 8.0f), false);
}

void AZLSocialSandboxNpc::ClearBubble()
{
	if (GetWorld() != nullptr) { GetWorld()->GetTimerManager().ClearTimer(BubbleTimer); }
	if (BubbleWidget != nullptr) { BubbleWidget->SetVisibility(false); }
}

void AZLSocialSandboxNpc::FaceLabelsToCamera() const
{
	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (Camera == nullptr) { return; }
	if (NameLabel != nullptr)
	{
		NameLabel->SetWorldRotation((Camera->GetCameraLocation() - NameLabel->GetComponentLocation()).Rotation());
	}
}

void AZLSocialSandboxNpc::ShowDamageResult(const FZLSocialSandboxDamageResult& Result)
{
	if (!Result.bAccepted)
	{
		ShowBubble(
			FText::FromString(FString::Printf(TEXT("[Attack rejected] %s"), *Result.ReasonCode.ToString())),
			FColor(255, 170, 70),
			3.0f);
		return;
	}
	const FString Defense = Result.bDefended ? TEXT(" · DEFENDED") : FString();
	const FString State = Result.bIncapacitated ? TEXT(" · INCAPACITATED") : FString();
	ShowBubble(
		FText::FromString(FString::Printf(
			TEXT("[Hit] -%.0f HP · %.0f remaining%s%s"),
			Result.AppliedDamage,
			Result.HealthAfter,
			*Defense,
			*State)),
		FColor(255, 90, 80),
		5.0f);
}

void AZLSocialSandboxNpc::RefreshNameLabel()
{
	if (NameLabel == nullptr)
	{
		return;
	}
	FString ShortName = DisplayName.ToString();
	int32 Separator = INDEX_NONE;
	if (ShortName.FindLastChar(TEXT(' '), Separator))
	{
		ShortName.RightChopInline(Separator + 1);
	}
	const FString State = bIncapacitated
		? TEXT("INCAPACITATED")
		: (bDefending ? TEXT("DEFEND") : TEXT("READY"));
	NameLabel->SetText(FText::FromString(FString::Printf(
		TEXT("%s  HP %.0f/%.0f  %s"),
		*ShortName,
		Health,
		MaxHealth,
		*State)));
}
