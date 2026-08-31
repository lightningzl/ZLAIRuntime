#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZLSocialSandboxNpc.generated.h"

class UArrowComponent;
class UCapsuleComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class ZL_API AZLSocialSandboxNpc final : public AActor
{
	GENERATED_BODY()

public:
	AZLSocialSandboxNpc();

	void InitializeSandboxNpc(FName InStableId, const FText& InDisplayName, const FTransform& InStartTransform);
	void ResetToSandboxStart();

	FName GetStableId() const { return StableId; }
	const FText& GetDisplayName() const { return DisplayName; }
	FVector GetPlanarForwardVector() const;

private:
	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UArrowComponent> FacingArrow;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UTextRenderComponent> NameLabel;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	FName StableId;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	FText DisplayName;

	FTransform SandboxStartTransform;
};
