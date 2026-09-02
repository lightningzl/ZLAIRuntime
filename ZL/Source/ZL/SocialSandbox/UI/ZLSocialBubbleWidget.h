#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZLSocialBubbleWidget.generated.h"

class UTextBlock;

UCLASS()
class ZL_API UZLSocialBubbleWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetBubble(const FText& Text, const FLinearColor& Color);

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BubbleText;
};
