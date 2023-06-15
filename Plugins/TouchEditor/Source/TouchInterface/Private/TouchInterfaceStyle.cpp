// Copyright Lost in Game Studio. All Rights Reserved.


#include "TouchInterfaceStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

#if ENGINE_MAJOR_VERSION > 4
#include "Styling/SlateStyleMacros.h"
#endif

//Begin Needed for mobile
#include "Framework/Application/SlateApplication.h"
//End Needed for mobile

#define RootToContentDir Style->RootToContentDir

#if ENGINE_MAJOR_VERSION < 5
#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#endif



TSharedPtr<ISlateStyle> FTouchInterfaceStyle::StyleSet = nullptr;

void FTouchInterfaceStyle::RegisterStyle()
{
	if (StyleSet.IsValid()) return;

	StyleSet = Create();
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FTouchInterfaceStyle::UnregisterStyle()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

void FTouchInterfaceStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<ISlateStyle> FTouchInterfaceStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("TouchInterfaceStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("TouchInterfaceDesigner")->GetBaseDir()/TEXT("Resources"));

	
	// Default visual for control (joystick and button)
	Style->Set("DefaultBackgroundButton", new IMAGE_BRUSH("DefaultButton", FVector2D(128)));
	Style->Set("DefaultJoystickBackground", new IMAGE_BRUSH("DefaultJoystickBackground", FVector2D(256)));
	Style->Set("DefaultJoystickThumb", new IMAGE_BRUSH("DefaultJoystickThumb", FVector2D(128)));

	
	// Some useful resources
	Style->Set("WhiteCircle", new IMAGE_BRUSH("WhiteCircle_32px", FVector2D(32)));
	Style->Set("WhiteCircularBorder", new IMAGE_BRUSH("T_InteractionRadius", FVector2D(128)));
	
	return Style;
}