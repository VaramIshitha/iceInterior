// Copyright Lost in Game Studio. All Rights Reserved

#include "VirtualShapeDesignerCommands.h"

#define LOCTEXT_NAMESPACE "VirtualShapeDesignerCommands"

void FVirtualShapeDesignerCommands::RegisterCommands()
{
	//Todo: select mode by default
	//Todo: Press shift to pass to add mode
	//Todo: Press ctrl to pass to remove mode
	
	UI_COMMAND(FreeDrawCommand, "Free Draw", "Draw shape freely. Point will be added automatically", EUserInterfaceActionType::Button, FInputChord(EKeys::F));
	UI_COMMAND(AddPointCommand, "Add Point", "Add Point", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::A));
	UI_COMMAND(SelectPointCommand, "Select Point", "Select and move point", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::E));
	UI_COMMAND(RemovePointCommand, "Remove Point", "Remove point", EUserInterfaceActionType::Button, FInputChord(EKeys::R));
	UI_COMMAND(FlattenHorizontallyCommand, "Flatten", "Flatten Horizontally", EUserInterfaceActionType::Button, FInputChord(EKeys::H));
	UI_COMMAND(FlattenVerticallyCommand, "Flatten", "Flatten Vertically", EUserInterfaceActionType::Button, FInputChord(EKeys::V));

	UI_COMMAND(CompileCommand, "Compile", "Generate data needed for shape recognizer", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Shift, EKeys::C));
	UI_COMMAND(GridSnappingCommand, "Grid Snapping", "Toggle Grid Snapping", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::S));
	UI_COMMAND(SimplifyCommand, "Simplify", "Simplify shape by removing unessessary point", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(FitToViewCommand, "Recenter", "Recenter draw", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::F));
}

#undef LOCTEXT_NAMESPACE
