#include "SocialSandbox/UI/ZLSocialNameWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"

void UZLSocialNameWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	NameText = WidgetTree->ConstructWidget<UTextBlock>();
	NameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 18.0f));
	NameText->SetJustification(ETextJustify::Center);
	WidgetTree->RootWidget = NameText;
}

void UZLSocialNameWidget::SetName(const FText& Text, const FLinearColor& Color)
{
	if (NameText != nullptr)
	{
		NameText->SetText(Text);
		NameText->SetColorAndOpacity(FSlateColor(Color));
	}
}
