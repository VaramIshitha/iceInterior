// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FVirtualShapeDesignerCommands : public TCommands<FVirtualShapeDesignerCommands>
{
public:
	FVirtualShapeDesignerCommands()
	: TCommands<FVirtualShapeDesignerCommands>
		(
		TEXT("VirtualShapeDesigner"),
			INVTEXT("Virtual Shape Designer"),
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

	/** Virtual Shape Designer shortcut commands */
	//Todo: Undo/Redo

	/** Virtual Shape Designer Tool commands */
	TSharedPtr<FUICommandInfo> FreeDrawCommand;
	TSharedPtr<FUICommandInfo> AddPointCommand;
	TSharedPtr<FUICommandInfo> SelectPointCommand;
	TSharedPtr<FUICommandInfo> RemovePointCommand;
	TSharedPtr<FUICommandInfo> FlattenHorizontallyCommand;
	TSharedPtr<FUICommandInfo> FlattenVerticallyCommand;
	
	/** Virtual Shape Designer Toolbar commands */
	//TSharedPtr<FUICommandInfo> CornerThresholdCommand;

	/** Virtual Shape Designer Viewport Toolbar commands */
	TSharedPtr<FUICommandInfo> CompileCommand;
	TSharedPtr<FUICommandInfo> GridSnappingCommand;
	TSharedPtr<FUICommandInfo> SimplifyCommand;
	//TSharedPtr<FUICommandInfo> FitToViewCommand;
	//Todo: Direction, line, point, bound, order...
};
