#include "SocialSandbox/UI/ZLSocialBubbleWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"

void UZLSocialBubbleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	Background->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.035f, 0.92f));
	Background->SetPadding(FMargin(12.0f, 7.0f));
	WidgetTree->RootWidget = Background;

	BubbleText = WidgetTree->ConstructWidget<UTextBlock>();
	BubbleText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 16.0f)));
	BubbleText->SetJustification(ETextJustify::Center);
	BubbleText->SetAutoWrapText(true);
	Background->SetContent(BubbleText);
}

void UZLSocialBubbleWidget::SetBubble(const FText& Text, const FLinearColor& Color)
{
	if (BubbleText != nullptr)
	{
		BubbleText->SetText(Text);
		BubbleText->SetColorAndOpacity(FSlateColor(Color));
	}
}
