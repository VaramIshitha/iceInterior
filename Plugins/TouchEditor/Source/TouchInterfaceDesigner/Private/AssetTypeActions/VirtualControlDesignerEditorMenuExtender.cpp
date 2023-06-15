// Copyright Lost in Game Studio. All Rights Reserved

#include "VirtualControlDesignerEditorMenuExtender.h"

//#include "AssetRegistryHelpers.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IAssetTools.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "TouchInterfaceDesignerSettings.h"
#include "Editor/TouchInterfaceDesignerStyle.h"
#include "VirtualControlSetup.h"

#if ENGINE_MINOR_VERSION > 0
#include "Editor/Transactor.h"
#endif

#include "Factories/VirtualControlSetupFactory.h"
#include "GameFramework/TouchInterface.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/STouchInterfaceConverter.h"

#define LOCTEXT_NAMESPACE "TouchDesignerEditorMenuExtender"

static TSharedPtr<SWindow> ConvertWindow;

void FVirtualControlDesignerEditorMenuExtender::StartupMenuExtender()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FVirtualControlDesignerEditorMenuExtender::OnExtendContentBrowserAssetSelectionMenu));
	ContentBrowserExtenderDelegateHandle = CBMenuAssetExtenderDelegates.Last().GetHandle();
}

void FVirtualControlDesignerEditorMenuExtender::ShutdownMenuExtender()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate){ return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
}

TSharedRef<FExtender> FVirtualControlDesignerEditorMenuExtender::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddMenuExtension
	(
		"CommonAssetActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateStatic(&FVirtualControlDesignerEditorMenuExtender::CreateMenu, SelectedAssets)
	);

	return Extender;
}

void FVirtualControlDesignerEditorMenuExtender::CreateMenu(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets)
{
	if (SelectedAssets.Num() > 1)
	{
		for (auto AssetItr = SelectedAssets.CreateConstIterator(); AssetItr; ++AssetItr)
		{
			const FAssetData& Asset = *AssetItr;

#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0
			if (!Asset.IsRedirector() && Asset.AssetClassPath.GetAssetName() != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
			{
				if (!Asset.GetClass()->IsChildOf(UTouchInterface::StaticClass()))
				{
					return;
				}
			}
#else
			if (!Asset.IsRedirector() && Asset.AssetName != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
			{
				if (!Asset.GetClass()->IsChildOf(UTouchInterface::StaticClass()))
				{
					return;
				}
			}
#endif
			
		}
		
		FNotificationInfo ErrorNotification(FText(LOCTEXT("ErrorMsgKey", "Multi-select functionality is not yet implemented!")));
		FSlateNotificationManager::Get().AddNotification(ErrorNotification);
		return;
	}

	// Create Menu entry for each asset selected
	for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		const FAssetData& Asset = *AssetIt;

#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0
		if (!Asset.IsRedirector() && Asset.AssetClassPath.GetAssetName() != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
		{
			if (Asset.GetClass()->IsChildOf(UTouchInterface::StaticClass()))
			{
				MenuBuilder.BeginSection("VirtualControlDesignerEditor", LOCTEXT("ASSET_CONTEXT", "Virtual Control Designer"));
				MenuBuilder.AddMenuEntry
				(
					LOCTEXT("ConvertLabelKey", "Convert..."),
					LOCTEXT("ConvertToolTipKey","Convert this Touch Interface to Virtual Control Designer format"),
					//Todo: Add convert icon
					FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(),""),
					FUIAction(FExecuteAction::CreateStatic(&FVirtualControlDesignerEditorMenuExtender::OpenConvertWindow, SelectedAssets)),
					NAME_None,
					EUserInterfaceActionType::Button
				);
				MenuBuilder.EndSection();
			}
		}
#else
		if (!Asset.IsRedirector() && Asset.AssetName != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
		{
			if (Asset.GetClass()->IsChildOf(UTouchInterface::StaticClass()))
			{
				MenuBuilder.BeginSection("VirtualControlDesignerEditor", LOCTEXT("ASSET_CONTEXT", "Virtual Control Designer"));
				MenuBuilder.AddMenuEntry
				(
					LOCTEXT("ConvertLabelKey", "Convert..."),
					LOCTEXT("ConvertToolTipKey","Convert this Touch Interface to Virtual Control Designer format"),
					//Todo: Add convert icon
					FSlateIcon(FVirtualControlDesignerEditorStyle::GetStyleSetName(),""),
					FUIAction(FExecuteAction::CreateStatic(&FVirtualControlDesignerEditorMenuExtender::OpenConvertWindow, SelectedAssets)),
					NAME_None,
					EUserInterfaceActionType::Button
				);
				MenuBuilder.EndSection();
			}
		}
#endif
		
	}
}

void FVirtualControlDesignerEditorMenuExtender::OpenConvertWindow(TArray<FAssetData> SelectedAssets)
{
	// Prevent Multi window
	if (ConvertWindow.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(ConvertWindow, EFocusCause::SetDirectly);
		return;
	}
	
	//Get Asset as UObject, then cast to UTouchInterface. Allow to recover data in it and then use to create new Touch Designer Interface asset
	UTouchInterface* SelectedTouchInterface = Cast<UTouchInterface>(SelectedAssets[0].GetAsset());
	if (SelectedTouchInterface)
	{
		ConvertWindow = SNew(SWindow)
		.Title(LOCTEXT("WindowTitleKey", "Convert Touch Interface Setup"))
		.Type(EWindowType::Normal)
		.HasCloseButton(true)
		.ClientSize(FVector2D(1280,720))
		//.IsPopupWindow(true)
		//.IsTopmostWindow(true);
		.Content()
		[
			SNew(STouchInterfaceConverter)
			.OnConvert(FOnConvertAccepted::CreateStatic(&FVirtualControlDesignerEditorMenuExtender::Convert, SelectedTouchInterface, SelectedAssets[0]))
			.OnCancel(FSimpleDelegate::CreateStatic(&FVirtualControlDesignerEditorMenuExtender::Cancel))
			.SelectedTouchInterface(SelectedTouchInterface)
		];

		ConvertWindow->SetOnWindowClosed(FOnWindowClosed::CreateStatic(&FVirtualControlDesignerEditorMenuExtender::HandleOnConvertWindowClosed));

		// Get Active Top Level Window
		auto ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
		if (ParentWindow)
		{
			// Add new window that i make before and set parent window (my window will not disappear if i click outside window or focus on any other windows)
			FSlateApplication::Get().AddWindowAsNativeChild(ConvertWindow.ToSharedRef(), ParentWindow.ToSharedRef());
		}
		else
		{
			// Add new window that i make before. Warning! With "AddWindow" function, if IsTopmostWindow == false, so window disappear when you click outside of window or when loose focus
			FSlateApplication::Get().AddWindow(ConvertWindow.ToSharedRef());
		}
	}
}

void FVirtualControlDesignerEditorMenuExtender::Convert(FConvertSettings ConvertSettings, UTouchInterface* SelectedTouchInterface, FAssetData SelectedAsset)
{
	// Destroy Convert window
	FSlateApplication::Get().RequestDestroyWindow(ConvertWindow.ToSharedRef());
	ConvertWindow = nullptr;
	
	// Create New Touch Designer Interface asset
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UVirtualControlSetupFactory* DesignerInterfaceFactory = NewObject<UVirtualControlSetupFactory>();
	UObject* NewAsset = AssetTools.CreateAssetWithDialog(DesignerInterfaceFactory->GetSupportedClass(), DesignerInterfaceFactory);
	UVirtualControlSetup* NewTouchDesignerInterface = Cast<UVirtualControlSetup>(NewAsset);

	if (NewTouchDesignerInterface)
	{
		// Fill Data from Conversion settings
#if ENGINE_MAJOR_VERSION > 4
		TObjectPtr<UTexture2D> BackgroundButtonImage = nullptr;
		TObjectPtr<UTexture2D> BackgroundJoystickImage = nullptr;
		TObjectPtr<UTexture2D> ThumbJoystickImage = nullptr;
#else
		UTexture2D* BackgroundButtonImage = nullptr;
		UTexture2D* BackgroundJoystickImage = nullptr;
		UTexture2D* ThumbJoystickImage = nullptr;
#endif
		
		// Load Default Button Image from Touch Designer Editor Settings
		const FSoftObjectPath DefaultBackgroundObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultButtonImage;
		if(DefaultBackgroundObject.IsValid())
		{
			BackgroundButtonImage = LoadObject<UTexture2D>(nullptr, *DefaultBackgroundObject.ToString());
		}

		// Load Default Joystick Image
		const FSoftObjectPath DefaultBackgroundJoystickObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultBackgroundJoystickImage;
		if (DefaultBackgroundJoystickObject.IsValid())
		{
			BackgroundJoystickImage = LoadObject<UTexture2D>(nullptr, *DefaultBackgroundJoystickObject.ToString());
		}

		// Load Default Thumb Image
		const FSoftObjectPath DefaultThumbObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultThumbJoystickImage;
		if (DefaultThumbObject.IsValid())
		{
			ThumbJoystickImage = LoadObject<UTexture2D>(nullptr, *DefaultThumbObject.ToString());
		}
		
		TArray<FTouchInputControl> TouchInterfaceControl = SelectedTouchInterface->Controls;
		TArray<FVirtualControl> VirtualControls = {};

		int32 ControlNameIndex = 0;
		for (FTouchInputControl& InterfaceControl : TouchInterfaceControl)
		{
			// Check if Touch Interface control contain any image (prevent empty control creation)
			if (InterfaceControl.Image2 || InterfaceControl.Image1)
			{
				ControlNameIndex++;
				FVirtualControl ConvertedControl = FVirtualControl();

				//ConvertedControl.bIsJoystick = (InterfaceControl.Image1 && InterfaceControl.ThumbSize > FVector2D::ZeroVector) ? true : false;

				FString Name = FString::Printf(TEXT("Control_%d"), ControlNameIndex);
				ConvertedControl.ControlName = *Name;

				// Check if center position is absolute or not
				FVector2D TouchInterfaceCenter = InterfaceControl.Center;
				FVector2D TouchDesignerInterfacePosition = FVector2D::ZeroVector;
				TouchDesignerInterfacePosition.X = TouchInterfaceCenter.X > 1.0f ? FMath::Clamp(TouchInterfaceCenter.X / ConvertSettings.SizeReference.X, 0.0f, 1.0f) : TouchInterfaceCenter.X;
				TouchDesignerInterfacePosition.Y = TouchInterfaceCenter.Y > 1.0f ? FMath::Clamp(TouchInterfaceCenter.Y / ConvertSettings.SizeReference.Y, 0.0f, 1.0f) : TouchInterfaceCenter.Y;
				ConvertedControl.LandscapeCenter = TouchDesignerInterfacePosition;

				ConvertedControl.VisualSize.X = InterfaceControl.VisualSize.X > 1.0f ? InterfaceControl.VisualSize.X : (float)ConvertSettings.SizeReference.X * InterfaceControl.VisualSize.X;
				ConvertedControl.VisualSize.Y = InterfaceControl.VisualSize.Y > 1.0f ? InterfaceControl.VisualSize.Y : (float)ConvertSettings.SizeReference.Y * InterfaceControl.VisualSize.Y;

				ConvertedControl.ThumbSize.X = InterfaceControl.ThumbSize.X > 1.0f ? InterfaceControl.ThumbSize.X : (float)ConvertSettings.SizeReference.X * InterfaceControl.ThumbSize.X;
				ConvertedControl.ThumbSize.Y = InterfaceControl.ThumbSize.Y > 1.0f ? InterfaceControl.ThumbSize.Y : (float)ConvertSettings.SizeReference.Y * InterfaceControl.ThumbSize.Y;

				ConvertedControl.InteractionSize.X = InterfaceControl.InteractionSize.X > 1.0f ? InterfaceControl.InteractionSize.X : (float)ConvertSettings.SizeReference.X * InterfaceControl.InteractionSize.X;
				ConvertedControl.InteractionSize.Y = InterfaceControl.InteractionSize.Y > 1.0f ? InterfaceControl.InteractionSize.Y : (float)ConvertSettings.SizeReference.Y * InterfaceControl.InteractionSize.Y;

				TArray<FVisualLayer> Layers;
		
				if (ConvertSettings.bKeepImage)
				{
					//Check if control has thumb image
					if (InterfaceControl.Image1)
					{
#if ENGINE_MAJOR_VERSION > 4
						TObjectPtr<UTexture2D> BackgroundTexture = InterfaceControl.Image2 ? InterfaceControl.Image2 : BackgroundJoystickImage;
#else
						UTexture2D* BackgroundTexture = InterfaceControl.Image2 ? InterfaceControl.Image2 : BackgroundJoystickImage;
#endif
						Layers.Add(FVisualLayer(BackgroundTexture, 13));

#if ENGINE_MAJOR_VERSION > 4
						TObjectPtr<UTexture2D> ThumbTexture = InterfaceControl.Image1;
#else
						UTexture2D* ThumbTexture = InterfaceControl.Image1;
#endif
						Layers.Add(FVisualLayer(ThumbTexture, 14));
						
					}
					else
					{
#if ENGINE_MAJOR_VERSION > 4
						TObjectPtr<UTexture2D> BackgroundTexture = InterfaceControl.Image2 ? InterfaceControl.Image2 : BackgroundButtonImage;
#else
						UTexture2D* BackgroundTexture = InterfaceControl.Image2 ? InterfaceControl.Image2 : BackgroundButtonImage;
#endif
						Layers.Add(FVisualLayer(BackgroundTexture, 13));
					}
				}
				else
				{
					//Check if control has thumb image
					if (InterfaceControl.Image1)
					{
						Layers.Add(FVisualLayer(BackgroundJoystickImage, 13));
						Layers.Add(FVisualLayer(ThumbJoystickImage, 14));
					}
					else
					{
						Layers.Add(FVisualLayer(BackgroundButtonImage, 13));
					}
				}
		
				ConvertedControl.VisualLayers = Layers;
				
				//Check if control has thumb image
				if (InterfaceControl.Image1)
				{
					ConvertedControl.Type = EControlType::Joystick;
					ConvertedControl.HorizontalInputKey = InterfaceControl.MainInputKey;
					ConvertedControl.VerticalInputKey = InterfaceControl.AltInputKey;
				}
				else
				{
					ConvertedControl.Type = EControlType::Button;
					ConvertedControl.ButtonInputKey = InterfaceControl.MainInputKey;
				}
				
				VirtualControls.Add(ConvertedControl);
			}
		}

		NewTouchDesignerInterface->StartupDelay = SelectedTouchInterface->StartupDelay;
		NewTouchDesignerInterface->ActivationDelay = SelectedTouchInterface->ActivationDelay;
		NewTouchDesignerInterface->ActiveOpacity = SelectedTouchInterface->ActiveOpacity;
		NewTouchDesignerInterface->InactiveOpacity = SelectedTouchInterface->InactiveOpacity;
		NewTouchDesignerInterface->TimeUntilDeactivated = SelectedTouchInterface->TimeUntilDeactive;
		NewTouchDesignerInterface->TimeUntilReset = SelectedTouchInterface->TimeUntilReset;
		
		NewTouchDesignerInterface->VirtualControls = VirtualControls;
		NewTouchDesignerInterface->Modify();

		if (ConvertSettings.bDeleteAfterConversion)
		{
			bool bDeleteSucceeded = DeleteObject(SelectedAsset);

			FText DeleteText = bDeleteSucceeded ? LOCTEXT("SuccessDeleteKey", "Touch Interface was successfully Deleted") : LOCTEXT("FailDeleteKey","Touch Interface was not removed because reference was found");
			FNotificationInfo DeleteNotificationInfo(DeleteText);
			DeleteNotificationInfo.ExpireDuration = 5.0f;
			FSlateNotificationManager::Get().AddNotification(DeleteNotificationInfo);
		}
		
		FNotificationInfo SuccessNotificationInfo(LOCTEXT("SuccessMsgKey", "Touch Interface was successfully converted"));
		SuccessNotificationInfo.bUseSuccessFailIcons=true;
		SuccessNotificationInfo.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(SuccessNotificationInfo);
	}
}

void FVirtualControlDesignerEditorMenuExtender::Cancel()
{
	FSlateApplication::Get().RequestDestroyWindow(ConvertWindow.ToSharedRef());
	//ConvertWindow = nullptr;
	//FNotificationInfo CancelNotificationInfo(LOCTEXT("CancelMsgKey", "Conversion was cancelled by user"));
	//FSlateNotificationManager::Get().AddNotification(CancelNotificationInfo);
}

void FVirtualControlDesignerEditorMenuExtender::HandleOnConvertWindowClosed(const TSharedRef<SWindow>& SWindow)
{
	ConvertWindow = nullptr;
	FNotificationInfo CancelNotificationInfo(LOCTEXT("CancelMsgKey", "Conversion was cancelled by user"));
	FSlateNotificationManager::Get().AddNotification(CancelNotificationInfo);
}

bool FVirtualControlDesignerEditorMenuExtender::DeleteObject(FAssetData SelectedAsset)
{
	FString SafePackagePath = UPackageTools::SanitizePackageName(SelectedAsset.PackagePath.ToString());
	FString SafeAssetName = ObjectTools::SanitizeObjectName(SelectedAsset.AssetName.ToString());

	if (SafePackagePath.IsEmpty() || SafeAssetName.IsEmpty())
	{
		return false;
	}
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	const FString PackageName = SafePackagePath + TEXT("/") + SafeAssetName;
#if ENGINE_MINOR_VERSION >= 26 || ENGINE_MAJOR_VERSION > 4
	UPackage* Pkg = CreatePackage(*PackageName);
#else
	UPackage* Pkg = CreatePackage(nullptr, *PackageName);
#endif
	UObject* ExistingObject = StaticFindObject(UObject::StaticClass(), Pkg, *SafeAssetName);

	if (ExistingObject == nullptr)
	{
		return false;
	}
	
	// Try to fixup a redirector before we delete it
	if (ExistingObject->GetClass()->IsChildOf(UObjectRedirector::StaticClass()))
	{
		AssetTools.FixupReferencers({Cast<UObjectRedirector>(ExistingObject)});
	}

	FReferencerInformationList Refs;

	// Check and see whether we are referenced by any objects that won't be garbage collected.
	bool bIsReferenced = IsReferenced(ExistingObject, GARBAGE_COLLECTION_KEEPFLAGS, EInternalObjectFlags::GarbageCollectionKeepFlags, true, &Refs);
	if (bIsReferenced)
	{
		// determine whether the transaction buffer is the only thing holding a reference to the object
		// and if so, offer the user the option to reset the transaction buffer.
		GEditor->Trans->DisableObjectSerialization();
		bIsReferenced = IsReferenced(ExistingObject, GARBAGE_COLLECTION_KEEPFLAGS, EInternalObjectFlags::GarbageCollectionKeepFlags, true, &Refs);
		GEditor->Trans->EnableObjectSerialization();

		if (bIsReferenced)
		{
			return false;
		}
	}

	// Only ref to this object is the transaction buffer, clear the transaction buffer
	GEditor->Trans->Reset( NSLOCTEXT( "UnrealEd", "DeleteSelectedItem", "Delete Selected Item" ) );

	// Mark its package as dirty as we're going to delete it.
	ExistingObject->MarkPackageDirty();

	// Remove standalone flag so garbage collection can delete the object.
	ExistingObject->ClearFlags( RF_Standalone );

	// Notify the asset registry
	FAssetRegistryModule::AssetDeleted(ExistingObject);

	// Delete objects from content browser and disk
	ObjectTools::ForceDeleteObjects({ExistingObject}, false);

	// Force CG
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	return true;
}

#undef LOCTEXT_NAMESPACE
