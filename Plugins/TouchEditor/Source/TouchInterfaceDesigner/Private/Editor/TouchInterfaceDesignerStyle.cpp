// Copyright Lost in Game Studio. All Rights Reserved.

#include "TouchInterfaceDesignerStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

#if ENGINE_MAJOR_VERSION > 4
#include "Styling/SlateStyleMacros.h"
#endif

#define RootToContentDir Style->RootToContentDir

#if ENGINE_MAJOR_VERSION < 5
#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#endif

TSharedPtr<ISlateStyle> FTouchInterfaceDesignerStyle::StyleSet = nullptr;

void FTouchInterfaceDesignerStyle::RegisterStyle()
{
	if (StyleSet.IsValid()) return;

	StyleSet = Create();
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FTouchInterfaceDesignerStyle::UnregisterStyle()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

void FTouchInterfaceDesignerStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<ISlateStyle> FTouchInterfaceDesignerStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("VirtualControlDesignerEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("TouchInterfaceDesigner")->GetBaseDir()/TEXT("Resources"));

	// Class Icon
	Style->Set("ClassThumbnail.VirtualControlSetup", new IMAGE_BRUSH("Icons/Classes/VirtualControlSetupClass_64px", FVector2D(64)));
	Style->Set("ClassIcon.VirtualControlSetup", new IMAGE_BRUSH("Icons/Classes/VirtualControlSetupClass_16px", FVector2D(16)));

	Style->Set("ClassThumbnail.TouchInterfacePreset", new IMAGE_BRUSH("Icons/Classes/TouchInterfacePresetClass_64px", FVector2D(64)));
	Style->Set("ClassIcon.TouchInterfacePreset", new IMAGE_BRUSH("Icons/Classes/TouchInterfacePresetClass_16px", FVector2D(16)));

	Style->Set("ClassThumbnail.VirtualControlManager", new IMAGE_BRUSH("Icons/Classes/VirtualControlManagerClass_64px", FVector2D(64)));
	Style->Set("ClassIcon.VirtualControlManager", new IMAGE_BRUSH("Icons/Classes/VirtualControlManagerClass_16px", FVector2D(16)));

	Style->Set("ClassThumbnail.GestureManager", new IMAGE_BRUSH("Icons/Classes/GestureManagerClass_64px", FVector2D(64)));
	Style->Set("ClassIcon.GestureManager", new IMAGE_BRUSH("Icons/Classes/GestureManagerClass_16px", FVector2D(16,16)));

	Style->Set("ClassThumbnail.DesignerHelper", new IMAGE_BRUSH("Icons/Classes/DesignerHelperClass_64px", FVector2D(64)));
	Style->Set("ClassIcon.DesignerHelper", new IMAGE_BRUSH("Icons/Classes/DesignerHelperClass_16px", FVector2D(16,16)));

	Style->Set("ClassThumbnail.VirtualControlEvent", new IMAGE_BRUSH("Icons/Classes/VirtualControlEventClass_64px", FVector2D(64)));
	Style->Set("ClassIcon.VirtualControlEvent", new IMAGE_BRUSH("Icons/Classes/VirtualControlEventClass_16px", FVector2D(16)));

	//Tab icons
	Style->Set("LevelEditor.Tab.Hierarchy", new IMAGE_BRUSH("Icons/Tabs/Hierarchy_16px", FVector2D(16.0f)));
	Style->Set("LevelEditor.Tab.Palette", new IMAGE_BRUSH("Icons/Tabs/Palette_16px", FVector2D(16.0f)));
	Style->Set("LevelEditor.Tab.Preview", new IMAGE_BRUSH("Icons/Tabs/Preview_16px", FVector2D(16.0f)));
	Style->Set("LevelEditor.Tab.Viewport", new IMAGE_BRUSH("Icons/Tabs/Viewports_16px", FVector2D(16.0f)));
	Style->Set("LevelEditor.Tab.Detail", new IMAGE_BRUSH("Icons/Tabs/Details_16px", FVector2D(16.0f)));
	
	// Control Type Icon used in hierarchy tabs
	Style->Set("Hierarchy.Button.Icon", new IMAGE_BRUSH("Icons/ControlButtonIcon_64px", FVector2D(20)));
	Style->Set("Hierarchy.Joystick.Icon", new IMAGE_BRUSH("Icons/ControlJoystickIcon_64px", FVector2D(20)));
	Style->Set("Hierarchy.TouchRegion.Icon", new IMAGE_BRUSH("Icons/ControlTouchRegionIcon_64px", FVector2D(20)));

	//Control Type Icon used in palette tab
	Style->Set("Palette.Button.Icon", new IMAGE_BRUSH("Icons/ControlButtonIcon_64px", FVector2D(32)));
	Style->Set("Palette.Joystick.Icon", new IMAGE_BRUSH("Icons/ControlJoystickIcon_64px", FVector2D(32)));
	Style->Set("Palette.TouchRegion.Icon", new IMAGE_BRUSH("Icons/ControlTouchRegionIcon_64px", FVector2D(32)));

	// Toolbar button icon
	Style->Set("GeneralSettingsIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarGeneralSettingsIcon_40px", FVector2D(40)));
	Style->Set("BackgroundSettingsIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarBackgroundSettingsIcon_40px", FVector2D(40)));
	Style->Set("DashedOutlineSettingIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarOutline_Icon_32px", FVector2D(20)));
	Style->Set("OpacitySettingIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarOpacity_Icon_32px", FVector2D(20)));
	Style->Set("ConstraintSettingIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarConstraint_Icon_32px", FVector2D(20)));
	Style->Set("OrientationSettingIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarOrientation_Icon_32px", FVector2D(20)));
	Style->Set("SaveAsPresetIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarSaveAsPresetIcon_40px", FVector2D(40)));
	Style->Set("OpenPresetIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarOpenPresetIcon_40px", FVector2D(40)));
	Style->Set("GeneratePortraitIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarGeneratePortraitIcon_40px", FVector2D(40)));
	Style->Set("PressedPreviewSettingIcon", new IMAGE_BRUSH("Icons/Toolbar/ToolbarPressedPreview_Icon_32px", FVector2D(20)));

	
	// Default visual for control (joystick and button)
	Style->Set("DefaultBackgroundButton", new IMAGE_BRUSH("DefaultButton", FVector2D(128)));
	Style->Set("DefaultJoystickBackground", new IMAGE_BRUSH("DefaultJoystickBackground", FVector2D(256)));
	Style->Set("DefaultJoystickThumb", new IMAGE_BRUSH("DefaultJoystickThumb", FVector2D(128)));

	
	// Some useful resources
	Style->Set("WhiteCircle", new IMAGE_BRUSH("WhiteCircle_32px", FVector2D(32)));
	Style->Set("WhiteCircularBorder", new IMAGE_BRUSH("T_InteractionRadius", FVector2D(128)));
	Style->Set("DashedImage", new IMAGE_BRUSH("DashedBorder", FVector2D(20,20)));
	Style->Set("DashedBorder", new BOX_BRUSH("DashedBorder", FMargin(0)));
	Style->Set("HoverColor", FLinearColor(FColor(81,81,81)));
	Style->Set("BlankBrush", new IMAGE_BRUSH("BlankIcon_8px", FVector2D(8,8), FLinearColor::White, ESlateBrushTileType::Both));
	Style->Set("RoundedWhiteImage", new IMAGE_BRUSH("T_RoundedBox_01", FVector2D(32), FLinearColor::White));
	Style->Set("RoundedWhiteBox", new BOX_BRUSH("T_RoundedBox_01", FMargin(0.2f), FLinearColor::White));
	Style->Set("TopRoundedWhiteBox", new BOX_BRUSH("T_TopRoundedBox_01", FMargin(0.2f), FLinearColor::White));
	
	/*FButtonStyle MyButton = FButtonStyle();
	StyleSet->Set("ControlButton", FButtonStyle(MyButton)
	.SetNormal(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, FEditorStyle::GetSlateColor("SelectionColor")))
	.SetHovered(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, FEditorStyle::GetSlateColor("SelectionColor")))
	.SetPressed(BOX_BRUSH("Common/FlatButton", 2.0f / 8.0f, FEditorStyle::GetSlateColor("SelectionColor_Pressed")))
	);*/
	
	return Style;
}
