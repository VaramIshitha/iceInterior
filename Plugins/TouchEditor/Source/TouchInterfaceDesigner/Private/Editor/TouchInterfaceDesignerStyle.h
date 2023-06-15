// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "Styling/ISlateStyle.h"

class FTouchInterfaceDesignerStyle
{
public:
	static void RegisterStyle();
	static void UnregisterStyle();

	/** Reload textures used by slate renderer */
	static void ReloadTextures();

	/** Get instance of Virtual Control Designer SlateStyle */
	static const ISlateStyle& Get()
	{
		return *StyleSet;
	}

	/** Get StyleSet name of Virtual Control Designer SlateStyle */
	static const FName& GetStyleSetName()
	{
		return StyleSet->GetStyleSetName();
	}

private:
	static TSharedRef<ISlateStyle> Create();
	static TSharedPtr<ISlateStyle> StyleSet;
};
