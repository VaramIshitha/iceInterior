// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FVirtualControlDesignerCommands : public TCommands<FVirtualControlDesignerCommands>
{
public:
	FVirtualControlDesignerCommands()
		: TCommands<FVirtualControlDesignerCommands>
		(
			TEXT("VirtualControlDesignerEditor"),
			NSLOCTEXT("Contexts", "VirtualControlDesignerEditor", "Virtual Control Designer Editor"),
			NAME_None,
#if ENGINE_MAJOR_VERSION > 4
			FAppStyle::GetAppStyleSetName()
#else
			FEditorStyle::GetStyleSetName()
#endif

		)
	{
		
	}
	
	virtual void RegisterCommands() override;

	/** Virtual Control Designer shortcut commands */
	TSharedPtr<FUICommandInfo> CreateJoystickCommand;
	TSharedPtr<FUICommandInfo> CreateButtonCommand;
	TSharedPtr<FUICommandInfo> CreateTouchRegionCommand;
	
	/** Virtual Control Designer Toolbar commands */
	TSharedPtr<FUICommandInfo> OpenControlDesignerCommand;
	TSharedPtr<FUICommandInfo> ShowGeneralSettingsCommand;
	TSharedPtr<FUICommandInfo> ShowBackgroundSettingsCommand;
	TSharedPtr<FUICommandInfo> SaveAsPresetCommand;
	TSharedPtr<FUICommandInfo> GeneratePortraitPositionCommand;
	TSharedPtr<FUICommandInfo> OpenPresetCommand;
	TSharedPtr<FUICommandInfo> RecenterCanvasCommand;

	/** Designer viewport Toolbar commands */
	TSharedPtr<FUICommandInfo> ToggleOutlineCommand;
	TSharedPtr<FUICommandInfo> ToggleOrientationCommand;
	TSharedPtr<FUICommandInfo> ToggleOpacityCommand;
	TSharedPtr<FUICommandInfo> ToggleConstraintCommand;
	TSharedPtr<FUICommandInfo> TogglePressedPreviewCommand;

	/** Virtual Control Preview Toolbar commands */
	TSharedPtr<FUICommandInfo> TogglePreviewOpacityStateCommand;
};
