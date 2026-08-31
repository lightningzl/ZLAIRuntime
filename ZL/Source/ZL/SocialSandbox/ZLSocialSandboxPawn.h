#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZLSocialSandboxPawn.generated.h"

class UArrowComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class ZL_API AZLSocialSandboxPawn final : public ACharacter
{
	GENERATED_BODY()

public:
	AZLSocialSandboxPawn();

	void ResetToSandboxStart();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UArrowComponent> FacingArrow;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category="Sandbox")
	TObjectPtr<UTextRenderComponent> NameLabel;

	FTransform SandboxStartTransform;
};
