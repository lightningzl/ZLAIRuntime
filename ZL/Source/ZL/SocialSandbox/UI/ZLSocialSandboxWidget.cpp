#include "SocialSandbox/UI/ZLSocialSandboxWidget.h"

#include "ZLSocialInputValidation.h"
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
#include "Components/Widget.h"
#include "Components/ScrollBox.h"
#include "Styling/SlateTypes.h"

namespace
{
const FString SpeechOption = TEXT("说话");
const FString ActionOption = TEXT("行为");
const FString NoTargetOption = TEXT("无目标");
const FString WhisperOption = TEXT("小声说话");
const FString TalkOption = TEXT("正常说话");
const FString ShoutOption = TEXT("大声呼喊");
const FString InEarOption = TEXT("耳边说话");
const FString FaceOption = TEXT("面向目标");
const FString ApproachOption = TEXT("靠近目标");
const FString MoveAwayOption = TEXT("远离目标");
const FString AttackOption = TEXT("攻击目标");
const FString StopOption = TEXT("停止当前行为");

void ConfigureComboBoxAppearance(UComboBoxString* ComboBox)
{
	const FSlateColor TextColor(FLinearColor(0.95f, 0.97f, 1.0f));
	FComboBoxStyle ComboBoxStyle = ComboBox->GetWidgetStyle();
	FComboButtonStyle ComboButtonStyle = ComboBoxStyle.ComboButtonStyle;
	FButtonStyle ButtonStyle = ComboButtonStyle.ButtonStyle;
	ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(0.06f, 0.08f, 0.12f));
	ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.10f, 0.16f, 0.24f));
	ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.04f, 0.12f, 0.20f));
	ButtonStyle.Disabled.TintColor = FSlateColor(FLinearColor(0.04f, 0.05f, 0.07f));
	ButtonStyle.SetNormalForeground(TextColor);
	ButtonStyle.SetHoveredForeground(TextColor);
	ButtonStyle.SetPressedForeground(TextColor);
	ButtonStyle.SetDisabledForeground(FSlateColor(FLinearColor(0.55f, 0.60f, 0.68f)));
	ComboButtonStyle.SetButtonStyle(ButtonStyle);
	ComboButtonStyle.DownArrowImage.TintColor = TextColor;
	ComboButtonStyle.MenuBorderBrush.TintColor = FSlateColor(FLinearColor(0.03f, 0.04f, 0.06f));
	ComboBoxStyle.SetComboButtonStyle(ComboButtonStyle);
	ComboBox->SetWidgetStyle(ComboBoxStyle);

	FTableRowStyle ItemStyle = ComboBox->GetItemStyle();
	ItemStyle.SetTextColor(TextColor);
	ItemStyle.SetSelectedTextColor(FSlateColor(FLinearColor(0.3f, 0.9f, 1.0f)));
	ComboBox->SetItemStyle(ItemStyle);
}

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
	PanelSlot->SetSize(FVector2D(520.0f, 650.0f));

	UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Layout);
	AddLabel(WidgetTree, Layout, TEXT("社会交互舞台"), 24.0f);
	AddLabel(WidgetTree, Layout, TEXT("WASD 移动 · 鼠标转向 · 输入不会发送到服务端"), 14.0f);

	InputModeCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	ConfigureComboBoxAppearance(InputModeCombo);
	InputModeCombo->OnGenerateWidgetEvent.BindDynamic(this, &UZLSocialSandboxWidget::GenerateComboOption);
	InputModeCombo->AddOption(SpeechOption);
	InputModeCombo->AddOption(ActionOption);
	InputModeCombo->SetSelectedOption(SpeechOption);
	InputModeCombo->OnSelectionChanged.AddDynamic(this, &UZLSocialSandboxWidget::HandleInputModeChanged);
	Layout->AddChildToVerticalBox(InputModeCombo)->SetPadding(FMargin(0.0f, 5.0f));

	SpeechModeCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	ConfigureComboBoxAppearance(SpeechModeCombo);
	SpeechModeCombo->OnGenerateWidgetEvent.BindDynamic(this, &UZLSocialSandboxWidget::GenerateComboOption);
	SpeechModeCombo->AddOption(WhisperOption);
	SpeechModeCombo->AddOption(TalkOption);
	SpeechModeCombo->AddOption(ShoutOption);
	SpeechModeCombo->AddOption(InEarOption);
	SpeechModeCombo->SetSelectedOption(TalkOption);
	Layout->AddChildToVerticalBox(SpeechModeCombo)->SetPadding(FMargin(0.0f, 5.0f));

	ActionCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	ConfigureComboBoxAppearance(ActionCombo);
	ActionCombo->OnGenerateWidgetEvent.BindDynamic(this, &UZLSocialSandboxWidget::GenerateComboOption);
	ActionCombo->AddOption(FaceOption);
	ActionCombo->AddOption(ApproachOption);
	ActionCombo->AddOption(MoveAwayOption);
	ActionCombo->AddOption(AttackOption);
	ActionCombo->AddOption(StopOption);
	ActionCombo->SetSelectedOption(FaceOption);
	ActionCombo->SetVisibility(ESlateVisibility::Collapsed);
	Layout->AddChildToVerticalBox(ActionCombo)->SetPadding(FMargin(0.0f, 5.0f));

	TargetCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	ConfigureComboBoxAppearance(TargetCombo);
	TargetCombo->OnGenerateWidgetEvent.BindDynamic(this, &UZLSocialSandboxWidget::GenerateComboOption);
	TargetCombo->AddOption(NoTargetOption);
	TargetCombo->SetSelectedOption(NoTargetOption);
	TargetCombo->OnSelectionChanged.AddDynamic(this, &UZLSocialSandboxWidget::HandleTargetChanged);
	Layout->AddChildToVerticalBox(TargetCombo)->SetPadding(FMargin(0.0f, 5.0f));

	InputBox = WidgetTree->ConstructWidget<UEditableTextBox>();
	InputBox->SetHintText(FText::FromString(TEXT("输入说话内容（1–512 个字符）")));
	InputBox->OnTextCommitted.AddDynamic(this, &UZLSocialSandboxWidget::HandleInputCommitted);
	Layout->AddChildToVerticalBox(InputBox)->SetPadding(FMargin(0.0f, 5.0f));

	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>();
	Layout->AddChildToVerticalBox(Buttons)->SetPadding(FMargin(0.0f, 5.0f));
	UButton* SubmitButton = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* SubmitLabel = WidgetTree->ConstructWidget<UTextBlock>();
	SubmitLabel->SetText(FText::FromString(TEXT("提交")));
	SubmitButton->SetContent(SubmitLabel);
	SubmitButton->OnClicked.AddDynamic(this, &UZLSocialSandboxWidget::HandleSubmitClicked);
	Buttons->AddChildToHorizontalBox(SubmitButton)->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));

	UButton* ResetButton = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* ResetLabel = WidgetTree->ConstructWidget<UTextBlock>();
	ResetLabel->SetText(FText::FromString(TEXT("重置舞台")));
	ResetButton->SetContent(ResetLabel);
	ResetButton->OnClicked.AddDynamic(this, &UZLSocialSandboxWidget::HandleResetClicked);
	Buttons->AddChildToHorizontalBox(ResetButton);

	StatusText = AddLabel(WidgetTree, Layout, TEXT("就绪"), 15.0f);
	SetStatus(FText::FromString(TEXT("就绪")), false);
	AddLabel(WidgetTree, Layout, TEXT("NPC 个人感知面板"), 18.0f);
	InspectorText = AddLabel(WidgetTree, Layout, TEXT("选择一个 NPC 查看个人感知。"), 14.0f);
	InspectorText->SetAutoWrapText(true);

	UButton* HistoryToggleButton = WidgetTree->ConstructWidget<UButton>();
	InteractionHistoryToggleText = WidgetTree->ConstructWidget<UTextBlock>();
	InteractionHistoryToggleText->SetText(FText::FromString(TEXT("行动记录")));
	InteractionHistoryToggleText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 16.0f)));
	InteractionHistoryToggleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.93f, 1.0f)));
	HistoryToggleButton->SetContent(InteractionHistoryToggleText);
	HistoryToggleButton->OnClicked.AddDynamic(this, &UZLSocialSandboxWidget::HandleInteractionHistoryToggled);
	UCanvasPanelSlot* HistoryButtonSlot = Root->AddChildToCanvas(HistoryToggleButton);
	HistoryButtonSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	HistoryButtonSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	HistoryButtonSlot->SetPosition(FVector2D(-20.0f, 20.0f));
	HistoryButtonSlot->SetSize(FVector2D(120.0f, 40.0f));

	InteractionHistoryPanel = WidgetTree->ConstructWidget<UBorder>();
	InteractionHistoryPanel->SetPadding(FMargin(12.0f));
	InteractionHistoryPanel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.96f));
	InteractionHistoryPanel->SetVisibility(ESlateVisibility::Collapsed);
	UCanvasPanelSlot* HistoryPanelSlot = Root->AddChildToCanvas(InteractionHistoryPanel);
	HistoryPanelSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	HistoryPanelSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	HistoryPanelSlot->SetPosition(FVector2D(-20.0f, 72.0f));
	HistoryPanelSlot->SetSize(FVector2D(460.0f, 420.0f));
	UVerticalBox* HistoryLayout = WidgetTree->ConstructWidget<UVerticalBox>();
	InteractionHistoryPanel->SetContent(HistoryLayout);
	AddLabel(WidgetTree, HistoryLayout, TEXT("行动与对话记录"), 20.0f);
	AddLabel(WidgetTree, HistoryLayout, TEXT("玩家输入、NPC 对话与已接受行动（最近 12 条）"), 13.0f);
	InteractionHistoryList = WidgetTree->ConstructWidget<UScrollBox>();
	HistoryLayout->AddChildToVerticalBox(InteractionHistoryList)->SetSize(ESlateSizeRule::Fill);
}

void UZLSocialSandboxWidget::SetInspectorText(const FText& Text)
{
	if (InspectorText != nullptr) { InspectorText->SetText(Text); }
}

void UZLSocialSandboxWidget::SelectTarget(const FName TargetId)
{
	if (TargetCombo == nullptr) { return; }
	for (const TPair<FString, FName>& Entry : TargetIdsByOption)
	{
		if (Entry.Value == TargetId)
		{
			TargetCombo->SetSelectedOption(Entry.Key);
			return;
		}
	}
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
		const FString Option = DisplayNames[Index].ToString();
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
	const EZLSocialSandboxInputMode InputMode = InputModeCombo != nullptr && InputModeCombo->GetSelectedOption() == ActionOption
		? EZLSocialSandboxInputMode::Action
		: EZLSocialSandboxInputMode::Speech;
	const FString Input = InputMode == EZLSocialSandboxInputMode::Action
		? GetSelectedActionInput()
		: (InputBox == nullptr ? FString() : InputBox->GetText().ToString().TrimStartAndEnd());
	if (InputMode == EZLSocialSandboxInputMode::Speech && FZLSocialInputValidation::ValidateText(Input) != EZLSocialInputValidationResult::Valid)
	{
		SetStatus(FText::FromString(TEXT("拒绝：输入必须为 1–512 个字符")), true);
		return;
	}
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
	if (InputMode == EZLSocialSandboxInputMode::Speech && InputBox != nullptr)
	{
		InputBox->SetText(FText::GetEmpty());
	}
	AddInteractionRecord(InputMode, Input);
	SetStatus(FText::FromString(TEXT("已接受：正在处理输入")), false);
}

void UZLSocialSandboxWidget::HandleResetClicked()
{
	if (ResetHandler)
	{
		ResetHandler();
		ClearInteractionRecords();
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
		SpeechModeCombo->SetVisibility(SelectedItem == ActionOption ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (ActionCombo != nullptr)
	{
		ActionCombo->SetVisibility(SelectedItem == ActionOption ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (InputBox != nullptr)
	{
		InputBox->SetVisibility(SelectedItem == ActionOption ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UZLSocialSandboxWidget::HandleTargetChanged(FString, ESelectInfo::Type)
{
	if (SelectionHandler) { SelectionHandler(); }
}

void UZLSocialSandboxWidget::HandleInteractionHistoryToggled()
{
	if (InteractionHistoryPanel == nullptr)
	{
		return;
	}

	const bool bOpening = InteractionHistoryPanel->GetVisibility() != ESlateVisibility::Visible;
	InteractionHistoryPanel->SetVisibility(bOpening ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (InteractionHistoryToggleText != nullptr)
	{
		InteractionHistoryToggleText->SetText(FText::FromString(bOpening ? TEXT("收起记录") : TEXT("行动记录")));
	}
}

UWidget* UZLSocialSandboxWidget::GenerateComboOption(FString Item)
{
	UTextBlock* Option = WidgetTree->ConstructWidget<UTextBlock>();
	Option->SetText(FText::FromString(Item));
	Option->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 16.0f)));
	Option->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.97f, 1.0f)));
	return Option;
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

FString UZLSocialSandboxWidget::GetSelectedActionInput() const
{
	if (ActionCombo == nullptr)
	{
		return FString();
	}

	const FString Selected = ActionCombo->GetSelectedOption();
	if (Selected == FaceOption) { return TEXT("面向"); }
	if (Selected == ApproachOption) { return TEXT("靠近"); }
	if (Selected == MoveAwayOption) { return TEXT("远离"); }
	if (Selected == AttackOption) { return TEXT("攻击"); }
	if (Selected == StopOption) { return TEXT("停止"); }
	return FString();
}

void UZLSocialSandboxWidget::AddInteractionRecord(const EZLSocialSandboxInputMode InputMode, const FString& Input)
{
	const FString Target = TargetCombo == nullptr ? NoTargetOption : TargetCombo->GetSelectedOption();
	const FString Type = InputMode == EZLSocialSandboxInputMode::Speech
		? FString::Printf(TEXT("对话 · %s"), SpeechModeCombo == nullptr ? *TalkOption : *SpeechModeCombo->GetSelectedOption())
		: TEXT("行动");
	AppendInteractionRecord(FText::FromString(FString::Printf(TEXT("[%s → %s] %s"), *Type, *Target, *Input)), FLinearColor(0.92f, 0.95f, 1.0f));
}

void UZLSocialSandboxWidget::AppendInteractionRecord(const FText& Text, const FLinearColor& Color)
{
	if (InteractionHistoryList == nullptr)
	{
		return;
	}

	while (InteractionHistoryList->GetChildrenCount() >= 12)
	{
		InteractionHistoryList->RemoveChildAt(0);
	}

	UTextBlock* Record = WidgetTree->ConstructWidget<UTextBlock>();
	Record->SetText(Text);
	Record->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 15.0f)));
	Record->SetColorAndOpacity(FSlateColor(Color));
	Record->SetAutoWrapText(true);
	InteractionHistoryList->AddChild(Record);
	InteractionHistoryList->ScrollToEnd();
}

void UZLSocialSandboxWidget::ClearInteractionRecords()
{
	if (InteractionHistoryList != nullptr)
	{
		InteractionHistoryList->ClearChildren();
	}
}
