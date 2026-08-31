#include "SocialSandbox/ZLSocialSandboxNpc.h"

#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
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

	BubbleText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("BubbleText"));
	BubbleText->SetupAttachment(Capsule);
	BubbleText->SetRelativeLocation(FVector(0.0f, 0.0f, 190.0f));
	BubbleText->SetHorizontalAlignment(EHTA_Center);
	BubbleText->SetWorldSize(25.0f);
	BubbleText->SetTextRenderColor(FColor(255, 210, 80));
	BubbleText->SetHiddenInGame(true);
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
	ObservationBuffer.Reset();
	ClearBubble();
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
	default: break;
	}
	const TCHAR* PhaseText = Observation.ActionPhase == EZLSocialActionPhase::Started ? TEXT("开始") : TEXT("完成");
	ShowBubble(FText::FromString(FString::Printf(TEXT("[观察] 玩家%s%s"), PhaseText, ActionText)), FColor(255, 170, 70), 3.0f);
}

void AZLSocialSandboxNpc::ShowBubble(const FText& Text, const FColor& Color, const float DurationSeconds)
{
	if (BubbleText == nullptr || GetWorld() == nullptr) { return; }
	GetWorld()->GetTimerManager().ClearTimer(BubbleTimer);
	BubbleText->SetText(Text);
	BubbleText->SetTextRenderColor(Color);
	BubbleText->SetHiddenInGame(false);
	GetWorld()->GetTimerManager().SetTimer(BubbleTimer, this, &AZLSocialSandboxNpc::ClearBubble, FMath::Clamp(DurationSeconds, 0.5f, 8.0f), false);
}

void AZLSocialSandboxNpc::ClearBubble()
{
	if (GetWorld() != nullptr) { GetWorld()->GetTimerManager().ClearTimer(BubbleTimer); }
	if (BubbleText != nullptr) { BubbleText->SetHiddenInGame(true); }
}

void AZLSocialSandboxNpc::FaceLabelsToCamera() const
{
	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (Camera == nullptr) { return; }
	for (UTextRenderComponent* Label : { NameLabel.Get(), BubbleText.Get() })
	{
		if (Label != nullptr)
		{
			Label->SetWorldRotation((Camera->GetCameraLocation() - Label->GetComponentLocation()).Rotation());
		}
	}
}
