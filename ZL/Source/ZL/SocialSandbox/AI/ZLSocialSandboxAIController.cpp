#include "SocialSandbox/AI/ZLSocialSandboxAIController.h"

#include "Components/StateTreeAIComponent.h"

AZLSocialSandboxAIController::AZLSocialSandboxAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	bStartAILogicOnPossess = false;
	StateTreeAI->SetStartLogicAutomatically(false);
	bAttachToPawn = true;
}

void AZLSocialSandboxAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	StateTreeAI->StartLogic();
}
