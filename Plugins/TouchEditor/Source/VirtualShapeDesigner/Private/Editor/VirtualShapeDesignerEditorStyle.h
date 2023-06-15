// Copyright Lost in Game Studio. All Rights Reserved.


#pragma once

#include "Styling/ISlateStyle.h"

class FVirtualShapeDesignerEditorStyle
{
public:
	static void RegisterStyle();
	static void UnregisterStyle();

	static const ISlateStyle& Get() { return *StyleSet; }

	static const FName& GetStyleSetName() { return StyleSet->GetStyleSetName(); }

private:
	static TSharedRef<ISlateStyle> Create();
	
	static TSharedPtr<ISlateStyle> StyleSet;
};
