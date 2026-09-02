#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZLSocialNameWidget.generated.h"

class UTextBlock;

UCLASS()
class ZL_API UZLSocialNameWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetName(const FText& Text, const FLinearColor& Color);

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText;
};
