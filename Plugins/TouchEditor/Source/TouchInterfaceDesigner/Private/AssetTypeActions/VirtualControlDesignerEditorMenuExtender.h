// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class UTouchInterface;

struct FConvertSettings
{
	bool bKeepImage;
	bool bIsInLandscape;
	bool bDeleteAfterConversion;
		
	FIntPoint SizeReference;

	// Default Constructor
	FConvertSettings() :
	bKeepImage(true),
	bIsInLandscape(true),
	bDeleteAfterConversion(false),
	SizeReference(1920,1080)
	{
		
	}
};

class FVirtualControlDesignerEditorMenuExtender : public TSharedFromThis<FVirtualControlDesignerEditorMenuExtender>
{
public:	
	void StartupMenuExtender();
	void ShutdownMenuExtender();

private:
	FDelegateHandle ContentBrowserExtenderDelegateHandle;
	
	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);

	static void CreateMenu(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);
	static void OpenConvertWindow(TArray<FAssetData> SelectedAssets);
	static void Convert(FConvertSettings ConvertSettings, UTouchInterface* SelectedTouchInterface, FAssetData SelectedAsset);
	static void Cancel();
	static bool DeleteObject(FAssetData SelectedAsset);

	static void HandleOnConvertWindowClosed(const TSharedRef<SWindow>& SWindow);
};
