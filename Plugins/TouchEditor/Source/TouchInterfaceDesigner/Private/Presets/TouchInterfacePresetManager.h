// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

class UVirtualControlSetup;
class UTouchInterfacePreset;

DECLARE_DELEGATE_ThreeParams(FOnPresetSelected, const UTouchInterfacePreset*, const bool, const bool)
DECLARE_DELEGATE(FOnPresetWindowClosed)

class FTouchInterfacePresetManager : public TSharedFromThis<FTouchInterfacePresetManager>
{
public:
	FTouchInterfacePresetManager();
	~FTouchInterfacePresetManager();

	void StartupTouchInterfacePresetManager();
	void ShutdownTouchInterfacePresetManager();

	void OpenPresetWindow();
	void ClosePresetWindow();

	void SavePreset(const UVirtualControlSetup* CurrentSetup);
	
	void RefreshPresetCache();

private:
	void CachePreset();
	void ClearPresetCache();

	void HandleOnPresetSelected(const FAssetData SelectedPreset, const bool bAddVirtualControl, const bool bApplySettings);
	
	FReply HandleCreateNewPreset(const UVirtualControlSetup* CurrentSetup);

public:
	FOnPresetSelected OnPresetSelected;
	FOnPresetWindowClosed OnPresetWindowClosed;
	
private:
	// Used for saving current instance of modal window created
	TSharedPtr<SWindow> ModalWindow;

	//Todo: Create new widget file for this ?
	// Used to save preset name when we ask to user the name of preset that he want to create
	FText PresetName;

	// Contain data of all presets that currently exist in content browser when we call CachePresetAssetData()
	TArray<FAssetData> PresetInContentBrowser;
};
