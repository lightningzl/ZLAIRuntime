#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZLSocialSandboxWidget.generated.h"

class UButton;
class UComboBoxString;
class UEditableTextBox;
class UBorder;
class UScrollBox;
class UTextBlock;
class UWidget;

UENUM()
enum class EZLSocialSandboxInputMode : uint8
{
	Speech,
	Action
};

UCLASS()
class ZL_API UZLSocialSandboxWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	using FSubmitHandler = TFunction<FText(EZLSocialSandboxInputMode, FName, FName, const FString&)>;
	using FResetHandler = TFunction<void()>;
	using FSelectionHandler = TFunction<void()>;

	void SetSubmitHandler(FSubmitHandler InHandler) { SubmitHandler = MoveTemp(InHandler); }
	void SetResetHandler(FResetHandler InHandler) { ResetHandler = MoveTemp(InHandler); }
	void SetSelectionHandler(FSelectionHandler InHandler) { SelectionHandler = MoveTemp(InHandler); }
	void SetTargets(const TArray<FName>& StableIds, const TArray<FText>& DisplayNames);
	void SetStatus(const FText& Status, bool bIsError);
	void SetInspectorText(const FText& Text);
	void AppendInteractionRecord(const FText& Text, const FLinearColor& Color);
	FName GetSelectedTargetId() const;
	void SelectTarget(FName TargetId);

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleSubmitClicked();

	UFUNCTION()
	void HandleResetClicked();

	UFUNCTION()
	void HandleInputCommitted(const FText& Text, ETextCommit::Type CommitType);

	UFUNCTION()
	void HandleInputModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleTargetChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	UWidget* GenerateComboOption(FString Item);

	UFUNCTION()
	void HandleInteractionHistoryToggled();

	FName GetSelectedSpeechMode() const;
	FString GetSelectedActionInput() const;
	void AddInteractionRecord(EZLSocialSandboxInputMode InputMode, const FString& Input);
	void ClearInteractionRecords();

	UPROPERTY()
	TObjectPtr<UComboBoxString> InputModeCombo;

	UPROPERTY()
	TObjectPtr<UComboBoxString> SpeechModeCombo;

	UPROPERTY()
	TObjectPtr<UComboBoxString> ActionCombo;

	UPROPERTY()
	TObjectPtr<UComboBoxString> TargetCombo;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> InputBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY()
	TObjectPtr<UTextBlock> InspectorText;

	UPROPERTY()
	TObjectPtr<UBorder> InteractionHistoryPanel;

	UPROPERTY()
	TObjectPtr<UTextBlock> InteractionHistoryToggleText;

	UPROPERTY()
	TObjectPtr<UScrollBox> InteractionHistoryList;

	TMap<FString, FName> TargetIdsByOption;
	FSubmitHandler SubmitHandler;
	FResetHandler ResetHandler;
	FSelectionHandler SelectionHandler;
};
