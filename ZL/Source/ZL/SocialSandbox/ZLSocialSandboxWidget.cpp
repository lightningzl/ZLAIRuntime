#include "SocialSandbox/ZLSocialSandboxWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
const FString SpeechOption = TEXT("说话 Speech");
const FString ActionOption = TEXT("行为 Action");
const FString NoTargetOption = TEXT("无目标 None");
const FString WhisperOption = TEXT("小声 Whisper");
const FString TalkOption = TEXT("正常 Talk");
const FString ShoutOption = TEXT("大喊 Shout");
const FString InEarOption = TEXT("耳边 InEar");

UTextBlock* AddLabel(UWidgetTree* WidgetTree, UVerticalBox* Parent, const FString& Text, const float Size = 18.0f)
{
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
	Label->SetText(FText::FromString(Text));
	Label->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size)));
	Parent->AddChildToVerticalBox(Label)->SetPadding(FMargin(0.0f, 3.0f));
	return Label;
}
}

void UZLSocialSandboxWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Root;
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetPadding(FMargin(16.0f));
	Panel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.92f));
	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.0f, 1.0f));
	PanelSlot->SetAlignment(FVector2D(0.0f, 1.0f));
	PanelSlot->SetPosition(FVector2D(20.0f, -20.0f));
	PanelSlot->SetSize(FVector2D(500.0f, 430.0f));

	UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Layout);
	AddLabel(WidgetTree, Layout, TEXT("Social Sandbox / 社会交互舞台"), 24.0f);
	AddLabel(WidgetTree, Layout, TEXT("WASD 移动 · 鼠标转向 · 输入不会发送到 Python"), 14.0f);

	InputModeCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	InputModeCombo->AddOption(SpeechOption);
	InputModeCombo->AddOption(ActionOption);
	InputModeCombo->SetSelectedOption(SpeechOption);
	InputModeCombo->OnSelectionChanged.AddDynamic(this, &UZLSocialSandboxWidget::HandleInputModeChanged);
	Layout->AddChildToVerticalBox(InputModeCombo)->SetPadding(FMargin(0.0f, 5.0f));

	SpeechModeCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	SpeechModeCombo->AddOption(WhisperOption);
	SpeechModeCombo->AddOption(TalkOption);
	SpeechModeCombo->AddOption(ShoutOption);
	SpeechModeCombo->AddOption(InEarOption);
	SpeechModeCombo->SetSelectedOption(TalkOption);
	Layout->AddChildToVerticalBox(SpeechModeCombo)->SetPadding(FMargin(0.0f, 5.0f));

	TargetCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	TargetCombo->AddOption(NoTargetOption);
	TargetCombo->SetSelectedOption(NoTargetOption);
	Layout->AddChildToVerticalBox(TargetCombo)->SetPadding(FMargin(0.0f, 5.0f));

	InputBox = WidgetTree->ConstructWidget<UEditableTextBox>();
	InputBox->SetHintText(FText::FromString(TEXT("输入说话内容或受控行为（1–512 字符）")));
	InputBox->OnTextCommitted.AddDynamic(this, &UZLSocialSandboxWidget::HandleInputCommitted);
	Layout->AddChildToVerticalBox(InputBox)->SetPadding(FMargin(0.0f, 5.0f));

	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>();
	Layout->AddChildToVerticalBox(Buttons)->SetPadding(FMargin(0.0f, 5.0f));
	UButton* SubmitButton = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* SubmitLabel = WidgetTree->ConstructWidget<UTextBlock>();
	SubmitLabel->SetText(FText::FromString(TEXT("提交 Submit")));
	SubmitButton->SetContent(SubmitLabel);
	SubmitButton->OnClicked.AddDynamic(this, &UZLSocialSandboxWidget::HandleSubmitClicked);
	Buttons->AddChildToHorizontalBox(SubmitButton)->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));

	UButton* ResetButton = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* ResetLabel = WidgetTree->ConstructWidget<UTextBlock>();
	ResetLabel->SetText(FText::FromString(TEXT("重置舞台 Reset")));
	ResetButton->SetContent(ResetLabel);
	ResetButton->OnClicked.AddDynamic(this, &UZLSocialSandboxWidget::HandleResetClicked);
	Buttons->AddChildToHorizontalBox(ResetButton);

	StatusText = AddLabel(WidgetTree, Layout, TEXT("就绪 Ready"), 15.0f);
	SetStatus(FText::FromString(TEXT("就绪 Ready")), false);
}

void UZLSocialSandboxWidget::SetTargets(const TArray<FName>& StableIds, const TArray<FText>& DisplayNames)
{
	if (TargetCombo == nullptr)
	{
		return;
	}
	TargetIdsByOption.Reset();
	TargetCombo->ClearOptions();
	TargetCombo->AddOption(NoTargetOption);
	const int32 Count = FMath::Min(StableIds.Num(), DisplayNames.Num());
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FString Option = FString::Printf(TEXT("%s [%s]"), *DisplayNames[Index].ToString(), *StableIds[Index].ToString());
		TargetCombo->AddOption(Option);
		TargetIdsByOption.Add(Option, StableIds[Index]);
	}
	TargetCombo->SetSelectedOption(NoTargetOption);
}

void UZLSocialSandboxWidget::SetStatus(const FText& Status, const bool bIsError)
{
	if (StatusText != nullptr)
	{
		StatusText->SetText(Status);
		StatusText->SetColorAndOpacity(bIsError ? FSlateColor(FLinearColor(1.0f, 0.2f, 0.15f)) : FSlateColor(FLinearColor(0.2f, 1.0f, 0.45f)));
	}
}

void UZLSocialSandboxWidget::HandleSubmitClicked()
{
	const FString Input = InputBox == nullptr ? FString() : InputBox->GetText().ToString().TrimStartAndEnd();
	const int32 Length = CountUnicodeCodePoints(Input);
	if (Length < 1 || Length > 512)
	{
		SetStatus(FText::FromString(TEXT("拒绝：输入必须为 1–512 个 Unicode 字符")), true);
		return;
	}

	const EZLSocialSandboxInputMode InputMode = InputModeCombo != nullptr && InputModeCombo->GetSelectedOption() == ActionOption
		? EZLSocialSandboxInputMode::Action
		: EZLSocialSandboxInputMode::Speech;
	const FName SpeechMode = GetSelectedSpeechMode();
	const FName TargetId = GetSelectedTargetId();
	if (InputMode == EZLSocialSandboxInputMode::Speech && SpeechMode == TEXT("InEar") && TargetId.IsNone())
	{
		SetStatus(FText::FromString(TEXT("拒绝：耳边说话必须选择明确目标")), true);
		return;
	}
	if (!SubmitHandler)
	{
		SetStatus(FText::FromString(TEXT("拒绝：交互运行时尚未就绪")), true);
		return;
	}

	const FText Error = SubmitHandler(InputMode, SpeechMode, TargetId, Input);
	if (!Error.IsEmpty())
	{
		SetStatus(Error, true);
		return;
	}
	InputBox->SetText(FText::GetEmpty());
	SetStatus(FText::FromString(TEXT("已接受：UE 正在处理输入")), false);
}

void UZLSocialSandboxWidget::HandleResetClicked()
{
	if (ResetHandler)
	{
		ResetHandler();
		SetStatus(FText::FromString(TEXT("舞台已恢复确定初始状态")), false);
	}
}

void UZLSocialSandboxWidget::HandleInputCommitted(const FText&, const ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		HandleSubmitClicked();
	}
}

void UZLSocialSandboxWidget::HandleInputModeChanged(FString SelectedItem, ESelectInfo::Type)
{
	if (SpeechModeCombo != nullptr)
	{
		SpeechModeCombo->SetIsEnabled(SelectedItem != ActionOption);
	}
}

FName UZLSocialSandboxWidget::GetSelectedTargetId() const
{
	if (TargetCombo == nullptr)
	{
		return NAME_None;
	}
	const FName* Target = TargetIdsByOption.Find(TargetCombo->GetSelectedOption());
	return Target == nullptr ? NAME_None : *Target;
}

FName UZLSocialSandboxWidget::GetSelectedSpeechMode() const
{
	if (SpeechModeCombo == nullptr)
	{
		return TEXT("Talk");
	}
	const FString Selected = SpeechModeCombo->GetSelectedOption();
	if (Selected == WhisperOption) { return TEXT("Whisper"); }
	if (Selected == ShoutOption) { return TEXT("Shout"); }
	if (Selected == InEarOption) { return TEXT("InEar"); }
	return TEXT("Talk");
}

int32 UZLSocialSandboxWidget::CountUnicodeCodePoints(const FString& Text)
{
	int32 Count = 0;
	for (int32 Index = 0; Index < Text.Len(); ++Index)
	{
		const uint32 CodeUnit = static_cast<uint32>(Text[Index]);
		if (CodeUnit >= 0xD800 && CodeUnit <= 0xDBFF && Index + 1 < Text.Len())
		{
			const uint32 Next = static_cast<uint32>(Text[Index + 1]);
			if (Next >= 0xDC00 && Next <= 0xDFFF)
			{
				++Index;
			}
		}
		++Count;
	}
	return Count;
}
