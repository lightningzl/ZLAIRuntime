#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZLSocialSandboxAIController.generated.h"

class UStateTreeAIComponent;

UCLASS()
class ZL_API AZLSocialSandboxAIController final : public AAIController
{
	GENERATED_BODY()

public:
	AZLSocialSandboxAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sandbox|AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;
};
