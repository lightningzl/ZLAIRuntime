#include "SocialSandbox/World/ZLSocialSandboxGameMode.h"
#include "SocialSandbox/Systems/ZLSocialSandboxDecisionComponent.h"
#include "SocialSandbox/Systems/ZLSocialSandboxInteractionComponent.h"

#include "ZL.h"
#include "Kismet/GameplayStatics.h"
#include "SocialSandbox/Actors/ZLSocialSandboxNpc.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPawn.h"
#include "SocialSandbox/Actors/ZLSocialSandboxPlayerController.h"
#include "SocialSandbox/UI/ZLSocialSandboxInspectorPresenter.h"
#include "SocialSandbox/Systems/ZLSocialSandboxNpcRegistrySubsystem.h"
#include "SocialSandbox/Systems/ZLSocialSandboxWorldKnowledgeSubsystem.h"
#include "TimerManager.h"

AZLSocialSandboxGameMode::AZLSocialSandboxGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	SandboxPlayerClass = AZLSocialSandboxPawn::StaticClass();
	DefaultPawnClass = SandboxPlayerClass;
	PlayerControllerClass = AZLSocialSandboxPlayerController::StaticClass();
	DecisionComponent = CreateDefaultSubobject<UZLSocialSandboxDecisionComponent>(TEXT("DecisionComponent"));
	InteractionComponent = CreateDefaultSubobject<UZLSocialSandboxInteractionComponent>(TEXT("InteractionComponent"));
}

UClass* AZLSocialSandboxGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	return SandboxPlayerClass != nullptr ? SandboxPlayerClass.Get() : Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AZLSocialSandboxGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (GetSandboxNpcs().IsEmpty())
	{
		UE_LOG(LogZL, Warning, TEXT("Social Sandbox has no registered NPCs. Place NPC Spawner actors in the level."));
	}
	InteractionComponent->UpdateNpcDistanceBands();
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->RefreshSandboxTargets();
	}
}

void AZLSocialSandboxGameMode::ResetSocialSandbox()
{
	GetWorldTimerManager().ClearTimer(DecisionComponent->CooldownTimer);
	DecisionComponent->ResetState();
	GetWorld()->GetSubsystem<UZLSocialSandboxWorldKnowledgeSubsystem>()->Reset();
	GetWorld()->GetSubsystem<UZLSocialSandboxNpcRegistrySubsystem>()->ResetNpcStates();
	if (AZLSocialSandboxPawn* Pawn = Cast<AZLSocialSandboxPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Pawn->ResetToSandboxStart();
	}
	InteractionComponent->UpdateNpcDistanceBands();
	RefreshInspector();
}

void AZLSocialSandboxGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	InteractionComponent->UpdateNpcDistanceBands();
}

AZLSocialSandboxNpc* AZLSocialSandboxGameMode::FindSandboxNpc(const FName StableId) const
{
	return GetWorld()->GetSubsystem<UZLSocialSandboxNpcRegistrySubsystem>()->FindNpc(StableId);
}

const FZLSocialObservationSettings& AZLSocialSandboxGameMode::GetObservationSettings() const
{
	return ObservationSettings;
}

UZLSocialSandboxDecisionComponent* AZLSocialSandboxGameMode::GetDecisionComponent() const
{
	return DecisionComponent;
}

UZLSocialSandboxInteractionComponent* AZLSocialSandboxGameMode::GetInteractionComponent() const
{
	return InteractionComponent;
}

const TArray<TObjectPtr<AZLSocialSandboxNpc>>& AZLSocialSandboxGameMode::GetSandboxNpcs() const
{
	return GetWorld()->GetSubsystem<UZLSocialSandboxNpcRegistrySubsystem>()->GetNpcs();
}

bool AZLSocialSandboxGameMode::RegisterSandboxNpc(AZLSocialSandboxNpc* Npc)
{
	UZLSocialSandboxNpcRegistrySubsystem* Registry = GetWorld()->GetSubsystem<UZLSocialSandboxNpcRegistrySubsystem>();
	if (!IsValid(Npc) || Registry == nullptr || Registry->FindNpc(Npc->GetStableId()) != nullptr
		|| !DecisionComponent->MultiNpcDecision.RegisterNpc(Npc->GetStableId())
		|| !Registry->RegisterNpc(Npc))
	{
		return false;
	}
	InteractionComponent->UpdateNpcDistanceBands();
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->RefreshSandboxTargets();
	}
	return true;
}

void AZLSocialSandboxGameMode::TriggerWorldEvent(const FName EventType)
{
	GetWorld()->GetSubsystem<UZLSocialSandboxWorldKnowledgeSubsystem>()->Trigger(*this, EventType);
}

void AZLSocialSandboxGameMode::ReportWorldEvent(const FName ReporterId, const FName ReceiverId)
{
	GetWorld()->GetSubsystem<UZLSocialSandboxWorldKnowledgeSubsystem>()->Report(*this, ReporterId, ReceiverId);
}

void AZLSocialSandboxGameMode::SpreadWorldRumor(const FName ReporterId, const FName ReceiverId)
{
	GetWorld()->GetSubsystem<UZLSocialSandboxWorldKnowledgeSubsystem>()->SpreadRumor(*this, ReporterId, ReceiverId);
}

FText AZLSocialSandboxGameMode::BuildInspectorText(const FName NpcId) const
{
	return FZLSocialSandboxInspectorPresenter::Build(
		FindSandboxNpc(NpcId),
		DecisionComponent->NpcDecisionDebug.Find(NpcId),
		DecisionComponent->NpcSocialFacts.FindRef(NpcId),
		GetWorld()->GetSubsystem<UZLSocialSandboxWorldKnowledgeSubsystem>()->GetStore().GetForAgent(NpcId));
}

void AZLSocialSandboxGameMode::RefreshInspector() const
{
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->RefreshObservationInspector();
	}
}

void AZLSocialSandboxGameMode::AppendInteractionRecord(const FText& Text, const FLinearColor& Color) const
{
	if (AZLSocialSandboxPlayerController* Controller = Cast<AZLSocialSandboxPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->AppendInteractionRecord(Text, Color);
	}
}

FText AZLSocialSandboxGameMode::SubmitSpeech(const FName SpeechMode, const FName TargetId, const FString& Text)
{
	return InteractionComponent->SubmitSpeech(SpeechMode, TargetId, Text);
}

FText AZLSocialSandboxGameMode::SubmitAction(const FName TargetId, const FString& Text)
{
	return InteractionComponent->SubmitAction(TargetId, Text);
}

void AZLSocialSandboxGameMode::ResolvePlayerAttackFromAnimNotify(AZLSocialSandboxPawn* Player, const FName DamageSourceBone)
{
	InteractionComponent->ResolvePlayerAttackFromAnimNotify(Player, DamageSourceBone);
}
