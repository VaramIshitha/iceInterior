// Copyright Lost in Game Studio. All Rights Reserved.

#include "TouchInterfacePresetManager.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "STouchInterfacePresetWindow.h"
#include "TouchInterfacePreset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "Factories/TouchInterfacePresetFactory.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

DEFINE_LOG_CATEGORY_STATIC(LogTouchInterfacePresetManager, All, All);

FTouchInterfacePresetManager::FTouchInterfacePresetManager()
{
	
}

FTouchInterfacePresetManager::~FTouchInterfacePresetManager()
{
	
}

void FTouchInterfacePresetManager::StartupTouchInterfacePresetManager()
{
	CachePreset();
}

void FTouchInterfacePresetManager::ShutdownTouchInterfacePresetManager()
{
	
}

void FTouchInterfacePresetManager::OpenPresetWindow()
{
	if (PresetInContentBrowser.Num() > 0)
	{
		ModalWindow = SNew(SWindow)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.Title(INVTEXT("Select preset to apply to your touch interface"))
		.ClientSize(FVector2D(1280,720))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		//.SizingRule(ESizingRule::FixedSize)
		//.LayoutBorder(FMargin(5))
		[
			SNew(STouchInterfacePresetWindow, PresetInContentBrowser)
			.OnApply(FOnApplyPreset::CreateSP(this, &FTouchInterfacePresetManager::HandleOnPresetSelected))
			.OnCancel(FSimpleDelegate::CreateSP(this, &FTouchInterfacePresetManager::ClosePresetWindow))
		];

		FSlateApplication::Get().AddModalWindow(ModalWindow.ToSharedRef(), FSlateApplication::Get().GetActiveTopLevelWindow());
	}
	else
	{
		FNotificationInfo NotificationInfo(INVTEXT("There is not preset in Presets Folder"));
		NotificationInfo.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}
}

void FTouchInterfacePresetManager::ClosePresetWindow()
{
	if (ModalWindow)
	{
		FSlateApplication::Get().RequestDestroyWindow(ModalWindow.ToSharedRef());
		ModalWindow = nullptr;	
	}
}

void FTouchInterfacePresetManager::SavePreset(const UVirtualControlSetup* CurrentSetup)
{
	//Todo: Make better design (make dedicated widget)
	
	ModalWindow = SNew(SWindow)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	.Title(INVTEXT("Choose Preset Name"))
	.SizingRule(ESizingRule::Autosized)
	[
		SNew(SBox)
		.Padding(10)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(4)
			.FillHeight(1)
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(INVTEXT("TouchInterfaceDesigner/Preset/"))
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SEditableTextBox)
					.ForegroundColor(FSlateColor(FLinearColor::Black))
					.HintText(INVTEXT("Enter Name"))
					.OnVerifyTextChanged(FOnVerifyTextChanged::CreateLambda([this](const FText& InText, FText& ErrorText)
					{
						if (InText.IsEmptyOrWhitespace())
						{
							if (InText.IsEmpty())
							{
								ErrorText = INVTEXT("Fill Preset name");
							}
							else
							{
								ErrorText = INVTEXT("Whitespace not allowed");
							}
							
							PresetName = FText();
							return false;
						}

						if (FVirtualControlDesignerEditor::ContainAnySpace(InText))
						{
							ErrorText = INVTEXT("Space not allowed");
							PresetName = FText();
							return false;
						}

						for (auto Preset : PresetInContentBrowser)
						{
							if (Preset.AssetName == FName(InText.ToString()))
							{
								ErrorText = INVTEXT("This name already exist");
								PresetName = FText();
								return false;
							}
						}
						
						PresetName = InText;
						return true;
					}))
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ContentPadding(FMargin(2))
					.Text(INVTEXT("Cancel"))
					.OnClicked(FOnClicked::CreateLambda([this]()
					{
						FSlateApplication::Get().RequestDestroyWindow(ModalWindow.ToSharedRef());
						ModalWindow = nullptr;
						return FReply::Handled();
					}))
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ContentPadding(FMargin(2))
					.Text(INVTEXT("Save"))
					.IsEnabled_Lambda([this](){ return !PresetName.IsEmptyOrWhitespace(); })
					.OnClicked(FOnClicked::CreateSP(this, &FTouchInterfacePresetManager::HandleCreateNewPreset, CurrentSetup))
				]
			]
		]
	];
	
	FSlateApplication::Get().AddModalWindow(ModalWindow.ToSharedRef(), FSlateApplication::Get().GetActiveTopLevelWindow());
}

void FTouchInterfacePresetManager::RefreshPresetCache()
{
	PresetInContentBrowser.Empty();
	CachePreset();
}

void FTouchInterfacePresetManager::CachePreset()
{
	UE_LOG(LogTouchInterfacePresetManager, Log, TEXT("Cache Preset"));
	
	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	FARFilter Filter;

#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0
	Filter.ClassPaths.Add(FTopLevelAssetPath(UTouchInterfacePreset::StaticClass()));
#else
	Filter.ClassNames.Add("TouchInterfacePreset");
#endif
	
	Filter.PackagePaths.Add("/TouchInterfaceDesigner/Presets");
	AssetRegistry.GetAssets(Filter, PresetInContentBrowser);
}

void FTouchInterfacePresetManager::ClearPresetCache()
{
	PresetInContentBrowser.Empty();
}

void FTouchInterfacePresetManager::HandleOnPresetSelected(const FAssetData SelectedPreset, const bool bAddVirtualControls, const bool bApplySettings)
{	
	FSlateApplication::Get().RequestDestroyWindow(ModalWindow.ToSharedRef());
	ModalWindow = nullptr;
	
	const UTouchInterfacePreset* Preset = Cast<UTouchInterfacePreset>(SelectedPreset.GetAsset());

	OnPresetSelected.ExecuteIfBound(Preset, bAddVirtualControls, bApplySettings);
}

FReply FTouchInterfacePresetManager::HandleCreateNewPreset(const UVirtualControlSetup* CurrentSetup)
{
	FSlateApplication::Get().RequestDestroyWindow(ModalWindow.ToSharedRef());
	ModalWindow = nullptr;
						
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UTouchInterfacePresetFactory* PresetFactory = NewObject<UTouchInterfacePresetFactory>();
	UObject* NewAsset = AssetTools.CreateAsset(PresetName.ToString(),FString(TEXT("/TouchInterfaceDesigner/Presets")), PresetFactory->GetSupportedClass(), PresetFactory);
	UTouchInterfacePreset* Preset = Cast<UTouchInterfacePreset>(NewAsset);
						
	const FInterfaceSettings GeneralSettings
	(
		CurrentSetup->ActiveOpacity,
		CurrentSetup->InactiveOpacity,
		CurrentSetup->TimeUntilDeactivated,
		CurrentSetup->TimeUntilReset,
		CurrentSetup->ActivationDelay,
		CurrentSetup->StartupDelay
	);
						
	Preset->SavePreset(GeneralSettings, CurrentSetup->VirtualControls);
	Preset->Modify();
						
	FNotificationInfo SuccessNotificationInfo(INVTEXT("Preset was successfully saved"));
	SuccessNotificationInfo.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(SuccessNotificationInfo);

	RefreshPresetCache();
	
	return FReply::Handled();
}
