// Copyright Lost in Game Studio. All Right Reserved

#include "SPaletteTab.h"

#include "SlateOptMacros.h"
#include "SPaletteItem.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "VirtualControlSetup.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPaletteTab::Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InVCDEditor)
{
	VCDEditor = InVCDEditor;
	//Todo: Make list and grid view
	
	ChildSlot
	[
		SNew(SBox)
		.Padding(FMargin(10.0f))
		.Visibility(EVisibility::SelfHitTestInvisible)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			.ScrollBarAlwaysVisible(false)

			+SScrollBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SAssignNew(WrapBox, SWrapBox)
				.Visibility(EVisibility::SelfHitTestInvisible)
				.Orientation(Orient_Horizontal)
				.InnerSlotPadding(FVector2D(10.0f))
#if ENGINE_MAJOR_VERSION > 4
				.HAlign(HAlign_Left)
#endif

				.UseAllottedSize(true)
				.UseAllottedWidth(true)
			]
		]
	];

	CreateItem();
}

void SPaletteTab::CreateItem()
{
	WrapBox->AddSlot()
	[
		SNew(SPaletteItem, SharedThis(this))
		.TooltipText(INVTEXT("Create virtual button"))
		.ImageName(FName("Palette.Button.Icon"))
		.ItemName(INVTEXT("Virtual Button"))
		.Type(EControlType::Button)
	];

	WrapBox->AddSlot()
	[
		SNew(SPaletteItem, SharedThis(this))
		.TooltipText(INVTEXT("Create virtual joystick"))
		.ImageName(FName("Palette.Joystick.Icon"))
		.ItemName(INVTEXT("Virtual Joystick"))
		.Type(EControlType::Joystick)
	];

	WrapBox->AddSlot()
	[
		SNew(SPaletteItem, SharedThis(this))
		.TooltipText(INVTEXT("Create Touch Region"))
		.ImageName(FName("Palette.TouchRegion.Icon"))
		.ItemName(INVTEXT("Touch Region"))
		.Type(EControlType::TouchRegion)
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
