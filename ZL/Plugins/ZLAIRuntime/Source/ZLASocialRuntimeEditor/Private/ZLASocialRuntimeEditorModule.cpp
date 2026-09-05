#include "ZLSocialPersonaDataTableTools.h"

#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DesktopPlatformModule.h"
#include "Engine/DataTable.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailCustomization.h"
#include "IDesktopPlatform.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "ZLSocialPersona.h"

namespace
{
	using FJsonImportCallback = TFunction<void(const FString&)>;

	void OpenPersonaJsonImportWindow(const FText& Title, FJsonImportCallback OnImport)
	{
		const TSharedRef<SWindow> Window = SNew(SWindow).Title(Title).ClientSize(FVector2D(720.0f, 520.0f)).SupportsMaximize(false).SupportsMinimize(false);
		TSharedPtr<SMultiLineEditableTextBox> JsonTextBox;
		Window->SetContent(
			SNew(SBorder).Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SAssignNew(JsonTextBox, SMultiLineEditableTextBox)
					.HintText(NSLOCTEXT("ZLSocialPersona", "PasteJsonHint", "在此粘贴完整 JSON，然后点击导入"))
					.AutoWrapText(false)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 12.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
					[
						SNew(SButton).Text(NSLOCTEXT("ZLSocialPersona", "CancelJsonImport", "取消"))
						.OnClicked_Lambda([WeakWindow = TWeakPtr<SWindow>(Window)]()
						{
							if (const TSharedPtr<SWindow> PinnedWindow = WeakWindow.Pin()) { PinnedWindow->RequestDestroyWindow(); }
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth().Padding(8.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SButton).Text(NSLOCTEXT("ZLSocialPersona", "ConfirmJsonImport", "确认导入"))
						.OnClicked_Lambda([WeakWindow = TWeakPtr<SWindow>(Window), JsonTextBox, OnImport = MoveTemp(OnImport)]()
						{
							OnImport(JsonTextBox.IsValid() ? JsonTextBox->GetText().ToString() : FString());
							if (const TSharedPtr<SWindow> PinnedWindow = WeakWindow.Pin()) { PinnedWindow->RequestDestroyWindow(); }
							return FReply::Handled();
						})
					]
				]
			]);
		FSlateApplication::Get().AddWindow(Window);
	}

	void ExportPersonaBatchToClipboard(UDataTable& DataTable)
	{
		FString Json;
		FString Result;
		if (FZLSocialPersonaDataTableTools::ExportBatchJson(DataTable, Json, Result))
		{
			FPlatformApplicationMisc::ClipboardCopy(*Json);
			Result = TEXT("Persona batch JSON copied to the clipboard");
		}
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result));
	}

	void ExportPersonaBatchToFile(UDataTable& DataTable)
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if (!DesktopPlatform)
		{
			FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("ZLSocialPersona", "ExportDialogUnavailable", "无法打开文件保存窗口。"));
			return;
		}

		TArray<FString> SelectedFiles;
		const bool bSelected = DesktopPlatform->SaveFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			TEXT("导出 Persona 批量 JSON"),
			FString(),
			DataTable.GetName() + TEXT(".json"),
			TEXT("JSON 文件 (*.json)|*.json"),
			EFileDialogFlags::None,
			SelectedFiles);
		if (!bSelected || SelectedFiles.IsEmpty()) { return; }

		FString Json;
		FString Result;
		if (FZLSocialPersonaDataTableTools::ExportBatchJson(DataTable, Json, Result) && FFileHelper::SaveStringToFile(Json, *SelectedFiles[0]))
		{
			Result = FString::Printf(TEXT("Exported Persona batch JSON to %s"), *SelectedFiles[0]);
		}
		else if (Result.IsEmpty())
		{
			Result = TEXT("Persona batch JSON export failed");
		}
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result));
	}

	class FZLSocialPersonaAssetCustomization final : public IDetailCustomization
	{
	public:
		static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FZLSocialPersonaAssetCustomization>(); }

		virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
		{
			TArray<TWeakObjectPtr<UObject>> Objects;
			DetailBuilder.GetObjectsBeingCustomized(Objects);
			for (const TWeakObjectPtr<UObject>& Object : Objects)
			{
				if (UZLSocialPersonaAsset* Asset = Cast<UZLSocialPersonaAsset>(Object.Get())) { PersonaAsset = Asset; break; }
			}

			DetailBuilder.EditCategory(TEXT("JSON"))
				.AddCustomRow(NSLOCTEXT("ZLSocialPersona", "PasteImportFilter", "粘贴 JSON 导入"))
				.WholeRowContent()
				[
					SNew(SButton).Text(NSLOCTEXT("ZLSocialPersona", "PasteImportPersona", "粘贴 JSON 导入 Persona"))
					.OnClicked_Lambda([WeakAsset = PersonaAsset]()
					{
						OpenPersonaJsonImportWindow(NSLOCTEXT("ZLSocialPersona", "PasteImportPersonaTitle", "导入 Persona JSON"), [WeakAsset](const FString& Json)
						{
							if (UZLSocialPersonaAsset* Asset = WeakAsset.Get())
							{
								Asset->ImportPersonaJsonText(Json);
								FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Asset->LastJsonOperationResult));
							}
						});
						return FReply::Handled();
					})
				];
		}

	private:
		TWeakObjectPtr<UZLSocialPersonaAsset> PersonaAsset;
	};
}

class FZLASocialRuntimeEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		PropertyEditor.RegisterCustomClassLayout(UZLSocialPersonaAsset::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FZLSocialPersonaAssetCustomization::MakeInstance));
		PropertyEditor.NotifyCustomizationModuleChanged();

		FContentBrowserModule& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		ContentBrowserExtender = FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FZLASocialRuntimeEditorModule::ExtendDataTableAssetMenu);
		ContentBrowserExtenderHandle = ContentBrowserExtender.GetHandle();
		ContentBrowser.GetAllAssetViewContextMenuExtenders().Add(ContentBrowserExtender);
	}

	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("ContentBrowser")))
		{
			FContentBrowserModule& ContentBrowser = FModuleManager::GetModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
			ContentBrowser.GetAllAssetViewContextMenuExtenders().RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Extender)
			{
				return Extender.GetHandle() == ContentBrowserExtenderHandle;
			});
		}
		if (FModuleManager::Get().IsModuleLoaded(TEXT("PropertyEditor")))
		{
			FPropertyEditorModule& PropertyEditor = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
			PropertyEditor.UnregisterCustomClassLayout(UZLSocialPersonaAsset::StaticClass()->GetFName());
		}
	}

private:
	TSharedRef<FExtender> ExtendDataTableAssetMenu(const TArray<FAssetData>& SelectedAssets)
	{
		TArray<TWeakObjectPtr<UDataTable>> PersonaTables;
		for (const FAssetData& AssetData : SelectedAssets)
		{
			if (UDataTable* DataTable = Cast<UDataTable>(AssetData.GetAsset()); DataTable && DataTable->GetRowStruct() == FZLSocialPersonaRow::StaticStruct()) { PersonaTables.Add(DataTable); }
		}

		const TSharedRef<FExtender> Extender = MakeShared<FExtender>();
		if (PersonaTables.Num() != 1 || SelectedAssets.Num() != 1) { return Extender; }
		Extender->AddMenuExtension(TEXT("GetAssetActions"), EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateLambda([PersonaTable = PersonaTables[0]](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.AddMenuEntry(
				NSLOCTEXT("ZLSocialPersona", "PasteBatchImportLabel", "粘贴 Persona 批量 JSON"),
				NSLOCTEXT("ZLSocialPersona", "PasteBatchImportTooltip", "粘贴并导入严格验证的 Persona 批量 JSON。失败时不会修改 DataTable。"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([PersonaTable]()
				{
					OpenPersonaJsonImportWindow(NSLOCTEXT("ZLSocialPersona", "PasteBatchImportTitle", "导入 Persona 批量 JSON"), [PersonaTable](const FString& Json)
					{
						if (UDataTable* DataTable = PersonaTable.Get())
						{
							FString Result;
							FZLSocialPersonaDataTableTools::ImportBatchJson(*DataTable, Json, Result);
							FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result));
						}
					});
				})));
			MenuBuilder.AddMenuEntry(
				NSLOCTEXT("ZLSocialPersona", "ExportBatchCopyLabel", "复制 Persona 批量 JSON"),
				NSLOCTEXT("ZLSocialPersona", "ExportBatchCopyTooltip", "将当前 Persona DataTable 导出为严格 JSON 并复制到剪贴板。"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([PersonaTable]()
				{
					if (UDataTable* DataTable = PersonaTable.Get()) { ExportPersonaBatchToClipboard(*DataTable); }
				})));
			MenuBuilder.AddMenuEntry(
				NSLOCTEXT("ZLSocialPersona", "ExportBatchFileLabel", "导出 Persona 批量 JSON 到文件"),
				NSLOCTEXT("ZLSocialPersona", "ExportBatchFileTooltip", "选择保存位置并将当前 Persona DataTable 导出为严格 JSON。"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([PersonaTable]()
				{
					if (UDataTable* DataTable = PersonaTable.Get()) { ExportPersonaBatchToFile(*DataTable); }
				})));
		}));
		return Extender;
	}

	FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtender;
	FDelegateHandle ContentBrowserExtenderHandle;
};

IMPLEMENT_MODULE(FZLASocialRuntimeEditorModule, ZLASocialRuntimeEditor)
