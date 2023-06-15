// Copyright Lost in Game Studio. All Rights Reserved.


#include "VirtualShapeDesignerEditorStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

//SlateStyleMacros does not exist in 4.27
#if ENGINE_MAJOR_VERSION > 4
#include "Styling/SlateStyleMacros.h"
#endif

#define RootToContentDir Style->RootToContentDir

#if ENGINE_MAJOR_VERSION < 5
#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#endif

TSharedPtr<ISlateStyle> FVirtualShapeDesignerEditorStyle::StyleSet = nullptr;

void FVirtualShapeDesignerEditorStyle::RegisterStyle()
{
	if (StyleSet.IsValid()) return;

	StyleSet = Create();
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FVirtualShapeDesignerEditorStyle::UnregisterStyle()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

TSharedRef<ISlateStyle> FVirtualShapeDesignerEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("VirtualShapeDesignerEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("TouchInterfaceDesigner")->GetBaseDir()/TEXT("Resources"));

	//Set icon for virtual shape class
	Style->Set("ClassThumbnail.VirtualShape", new IMAGE_BRUSH("Icons/Classes/VirtualShapeClass_64px", FVector2D(64)));
	Style->Set("ClassIcon.VirtualShape", new IMAGE_BRUSH("Icons/Classes/VirtualShapeClass_16px", FVector2D(16)));

	//Tab icons
	Style->Set("LevelEditor.Tab.Hierarchy", new IMAGE_BRUSH("Icons/Tabs/Hierarchy_16px", FVector2D(16.0f)));
	Style->Set("LevelEditor.Tab.Palette", new IMAGE_BRUSH("Icons/Tabs/Palette_16px", FVector2D(16.0f)));
	Style->Set("LevelEditor.Tab.Preview", new IMAGE_BRUSH("Icons/Tabs/Preview_16px", FVector2D(16.0f)));
	Style->Set("LevelEditor.Tab.Viewport", new IMAGE_BRUSH("Icons/Tabs/Viewports_16px", FVector2D(16.0f)));
	Style->Set("LevelEditor.Tab.Detail", new IMAGE_BRUSH("Icons/Tabs/Details_16px", FVector2D(16.0f)));

	const FVector2D ToolIconSize(30.0f);
	
	//Tool icons
	Style->Set("Palette.FreeDraw.Icon", new IMAGE_BRUSH("Icons/Toolbar/FreeDrawIcon_64px", ToolIconSize));
	Style->Set("Palette.AddPoint.Icon", new IMAGE_BRUSH("Icons/Toolbar/AddPointIcon_64px", ToolIconSize));
	Style->Set("Palette.SelectPoint.Icon", new IMAGE_BRUSH("Icons/Toolbar/SelectPointIcon_64px", ToolIconSize));
	Style->Set("Palette.RemovePoint.Icon", new IMAGE_BRUSH("Icons/Toolbar/RemovePointIcon_64px", ToolIconSize));

	//Toolbar button icon
	//Style->Set("VirtualShapeDesigner.GridSnappingCommand", new IMAGE_BRUSH("Icons/Toolbar/GridSnapping_16px", FVector2D(16.0f)));
	Style->Set("VirtualShapeDesigner.CornerThreshold", new IMAGE_BRUSH("Icons/Toolbar/CornerThreshold_64px", FVector2D(40.0f)));

	const FSlateColor& SelectedColor = FAppStyle::Get().GetSlateColor("SelectionColor_Pressed");
	const FSlateColor& HoverColor = FAppStyle::Get().GetSlateColor("SelectionColor_Inactive");
	
	const FButtonStyle UnselectedButton = FButtonStyle()
		.SetNormal(BOX_BRUSH("Common/Button/simple_round_normal", FMargin(4 / 16.0f), FLinearColor(1, 1, 1, 0)))
		.SetHovered(BOX_BRUSH("Common/Button/simple_round_normal", FMargin(4 / 16.0f), FLinearColor(1, 1, 1, 0.1f)))
		.SetPressed(BOX_BRUSH("Common/Button/simple_round_normal", FMargin(4 / 16.0f), SelectedColor))
		.SetNormalPadding(FMargin(0, 0, 0, 1))
		.SetPressedPadding(FMargin(0, 1, 0, 0));
	Style->Set("UnselectedButton", UnselectedButton);

	//const FLinearColor& SelectedColor = FEditorStyle::Get().GetColor("SelectionColor_Pressed");
	//const FLinearColor& SelectedColor = FLinearColor(0.701f, 0.225f, 0.003f);
	
	const FSlateColor& PressedSelectedColor = FAppStyle::Get().GetSlateColor("SelectionColor");
	
	const FButtonStyle SelectedButton = FButtonStyle()
		.SetNormal(BOX_BRUSH("Common/Button/simple_round_normal", FMargin(4 / 16.0f), SelectedColor))
		.SetHovered(BOX_BRUSH("Common/Button/simple_round_normal", FMargin(4 / 16.0f), SelectedColor))
		.SetPressed(BOX_BRUSH("Common/Button/simple_round_normal", FMargin(4 / 16.0f), SelectedColor))
		.SetNormalPadding(FMargin(0, 0, 0, 1))
		.SetPressedPadding(FMargin(0, 1, 0, 0));
	Style->Set("SelectedButton", SelectedButton);

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

	Style->Set("StartPoint", new IMAGE_BRUSH("StartPoint_32px", FVector2D(32)));
	Style->Set("EndPoint", new IMAGE_BRUSH("EndPoint_32px", FVector2D(32)));

	return Style;
}
