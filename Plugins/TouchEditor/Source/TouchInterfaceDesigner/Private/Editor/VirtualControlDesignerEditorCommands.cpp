// Copyright Lost in Game Studio. All Rights Reserved

#include "VirtualControlDesignerEditorCommands.h"
#include "TouchInterfaceDesignerStyle.h"

#define LOCTEXT_NAMESPACE "VirtualControlDesignerEditorCommands"

void FVirtualControlDesignerCommands::RegisterCommands()
{
	// Virtual Control Designer shortcut commands
	
	UI_COMMAND(CreateButtonCommand, "CreateButton", "Add Button in canvas", EUserInterfaceActionType::None, FInputChord(EKeys::B));
	UI_COMMAND(CreateJoystickCommand, "CreateJoystick", "Add Joystick in canvas", EUserInterfaceActionType::None, FInputChord(EKeys::J));
	UI_COMMAND(CreateTouchRegionCommand, "CreateTouchRegion", "Add Touch Region in canvas", EUserInterfaceActionType::None, FInputChord(EKeys::R));
	
	FVirtualControlDesignerCommands& TouchDesignerEditorCommands = *Instance.Pin();

	// Virtual Control Designer Toolbar commands
	
	UI_COMMAND(OpenControlDesignerCommand, "Designer", "Create your own button and joystick design", EUserInterfaceActionType::Button, FInputChord());
	
	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		ShowGeneralSettingsCommand,
		"General Settings",
		LOCTEXT("General Settings", "Settings"),
		LOCTEXT("GeneralSettingsDescKey", "Show general settings of Touch Designer Interface"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(),"GeneralSettingsIcon"),
		EUserInterfaceActionType::Button,
		FInputChord()
	);

	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		ShowBackgroundSettingsCommand,
		"BackgroundSettings",
		LOCTEXT("BackgroundSettingsKey", "Background"),
		LOCTEXT("BackgroundSettingsDescKey", "Show background settings of Designer Surface"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "BackgroundSettingsIcon"),
		EUserInterfaceActionType::Button,
		FInputChord()
	);

	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		SaveAsPresetCommand,
		"SaveAsPreset",
		LOCTEXT("SaveAsPresetKey", "Save As Preset"),
		LOCTEXT("SaveAsPresetDescKey", "Save your current Touch Designer Interface as new preset"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "SaveAsPresetIcon"),
		EUserInterfaceActionType::Button,
		FInputChord()
	);

	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		GeneratePortraitPositionCommand,
		"GeneratePortrait",
		LOCTEXT("GeneratePortraitKey", "Generate Portrait"),
		LOCTEXT("GeneratePortraitDescKey", "Estimate portrait control position from landscape orientation and generate data for portrait orientation"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "GeneratePortraitIcon"),
		EUserInterfaceActionType::Button,
		FInputChord()
	);
	
	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		OpenPresetCommand,
		"OpenPreset",
		LOCTEXT("OpenPresetKey", "Open Preset"),
		LOCTEXT("OpenPresetDescKey", "Open Preset window and apply pre-defined control data and positon to current Touch Designer Interface"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "OpenPresetIcon"),
		EUserInterfaceActionType::Button,
		FInputChord()
	);

	UI_COMMAND(RecenterCanvasCommand, "Recenter", "Recenter Canvas in viewport", EUserInterfaceActionType::Button, FInputChord(EKeys::F));


	// Designer viewport Toolbar commands
	
	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		ToggleOutlineCommand,
		"ToggleDashedOutline",
		LOCTEXT("ToggleDashedOutlineKey", "Toggle Dashed Outline"),
		LOCTEXT("ToggleDashedOutlineDescKey", "Enable or disable showing the canvas dashed outline"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "DashedOutlineSettingIcon"),
		EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::G)
	);

	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		ToggleOpacityCommand,
		"ToggleOpacity",
		LOCTEXT("ToggleOpacityKey", "Toggle Opacity"),
		LOCTEXT("ToggleOpacityDescKey", "Show active or inactive opacity value of controls"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "OpacitySettingIcon"),
		EUserInterfaceActionType::ToggleButton,
		FInputChord(EKeys::O)
	);
	
	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		ToggleConstraintCommand,
		"ToggleConstraint",
		LOCTEXT("ToggleConstraintKey", "Toggle Constraint"),
		LOCTEXT("ToggleConstraintDescKey", "Enable or disable constraint to canvas for controls"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "ConstraintSettingIcon"),
		EUserInterfaceActionType::ToggleButton,
		FInputChord(EKeys::C)
	);

	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		ToggleOrientationCommand,
		"ToggleOrientation",
		LOCTEXT("ToggleOrientationKey", "Toggle Orientation"),
		LOCTEXT("ToggleOrientationDescKey", "Set canvas to landscape or portrait orientation"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "OrientationSettingIcon"),
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::P)
	);

	FUICommandInfo::MakeCommandInfo
	(
		TouchDesignerEditorCommands.AsShared(),
		TogglePressedPreviewCommand,
		"TogglePressedPreview",
		LOCTEXT("TogglePressedPreviewKey", "Toggle Pressed Preview"),
		LOCTEXT("TogglePressedPreviewDesc", "Show pressed or released visual of virtual control"),
		FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "PressedPreviewSettingIcon"),
		EUserInterfaceActionType::ToggleButton,
		FInputChord(EKeys::S)
	);

	// Virtual Control Preview Toolbar commands
	
	UI_COMMAND(TogglePreviewOpacityStateCommand, "Preview Opacity State", "Preview virtual control opacity state", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE