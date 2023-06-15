// Copyright Lost in Game Studio, Inc. All Rights Reserved.

#include "TouchInterfaceDesignerSettings.h"
#include "Misc/FileHelper.h"

UTouchInterfaceDesignerSettings::UTouchInterfaceDesignerSettings()
{
	CategoryName = "Plugins";
	SectionName = "Touch Interface Designer";

	//VISUAL
	DefaultButtonImage="/TouchInterfaceDesigner/Defaults/T_DefaultButton_01.T_DefaultButton_01";
	DefaultBackgroundJoystickImage="/TouchInterfaceDesigner/Defaults/T_DefaultVirtualJoystick_Background.T_DefaultVirtualJoystick_Background";
	DefaultThumbJoystickImage="/TouchInterfaceDesigner/Defaults/T_DefaultVirtualJoystick_Thumb.T_DefaultVirtualJoystick_Thumb";
	MaxLayer = 10;

	//EDITOR
	bKeepCursorOffset = true;
	bZoomToPointerPosition = true;
	bShowDashedOutlineByDefault = true;
	InteractionVisualizationColor = FLinearColor::Red;

	//VIRTUAL SHAPE DESIGNER
	bEnableVirtualShapeEditor = false;
	ShapeLineColor = FLinearColor::White;
	ShapeDotColor = FLinearColor::Gray;

	ShapeLineSize = 5.0f;
	ShapeDotSize = 8.0f;
}
