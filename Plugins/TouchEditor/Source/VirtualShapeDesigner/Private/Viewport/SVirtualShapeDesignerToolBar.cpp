// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualShapeDesignerToolBar.h"

#include "SViewportToolBarComboMenu.h"
#include "VirtualShapeDesignerCommands.h"

void SVirtualShapeDesignerToolBar::Construct(const FArguments& InArgs)
{
	CommandList = InArgs._CommandList;
	OnGridSnappingChanged = InArgs._OnGridSnappingChanged;
	OnGridSpacingChanged = InArgs._OnGridSpacingChanged;
	bGridSnappingEnabled = InArgs._EnableGridSnapping;
	GridSpacingValue = InArgs._GridSpacing;

	ChildSlot
	[
		MakeToolBar(InArgs._Extenders)
	];

	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}

TSharedRef<SWidget> SVirtualShapeDesignerToolBar::MakeToolBar(const TSharedPtr<FExtender> InExtenders)
{
	FToolBarBuilder ToolBarBuilder(CommandList, FMultiBoxCustomization::None, InExtenders);	

#if ENGINE_MAJOR_VERSION > 4
	const FName ToolBarStyle = "EditorViewportToolBar";
	ToolBarBuilder.SetStyle(&FAppStyle::Get(), ToolBarStyle);
#else
	const FName ToolBarStyle = "ViewportMenu";
	ToolBarBuilder.SetStyle(&FEditorStyle::Get(), ToolBarStyle);
#endif
	
	ToolBarBuilder.SetLabelVisibility(EVisibility::Collapsed);
	
	ToolBarBuilder.BeginSection("GridSnapping");
	{
		FUICommandInfo* CommandInfo = FVirtualShapeDesignerCommands::Get().GridSnappingCommand.Get();

		ToolBarBuilder.AddWidget(SNew(SViewportToolBarComboMenu)
			.Style(ToolBarStyle)
			.BlockLocation(EMultiBlockLocation::Start)
			.Cursor(EMouseCursor::Default)
			.IsChecked(this, &SVirtualShapeDesignerToolBar::GridSnappingEnabled)
			.OnCheckStateChanged(this, &SVirtualShapeDesignerToolBar::ToggleGridSnapping)
			.Label(this, &SVirtualShapeDesignerToolBar::GetGridSpacingLabel)
			.OnGetMenuContent(this, &SVirtualShapeDesignerToolBar::OnGetGridSpacingMenuContent)
			.ToggleButtonToolTip(CommandInfo->GetDescription())
			.MenuButtonToolTip(INVTEXT("Set grid spacing value"))
#if ENGINE_MAJOR_VERSION > 4
			.Icon(FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("EditorViewport.LocationGridSnap")))
#else
			.Icon(FSlateIcon(FEditorStyle::GetStyleSetName(), TEXT("EditorViewport.LocationGridSnap")))
#endif
			.ParentToolBar(SharedThis(this))
			);
	}
	ToolBarBuilder.EndSection();

	return ToolBarBuilder.MakeWidget();
}

void SVirtualShapeDesignerToolBar::ToggleGridSnapping(ECheckBoxState State)
{
	if (State == ECheckBoxState::Checked)
	{
		bGridSnappingEnabled = true;
		OnGridSnappingChanged.ExecuteIfBound(true);
	}
	else
	{
		bGridSnappingEnabled = false;
		OnGridSnappingChanged.ExecuteIfBound(false);
	}
}

ECheckBoxState SVirtualShapeDesignerToolBar::GridSnappingEnabled() const
{
	return bGridSnappingEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

FText SVirtualShapeDesignerToolBar::GetGridSpacingLabel() const
{
	return FText::AsNumber(GridSpacingValue);
}

TSharedRef<SWidget> SVirtualShapeDesignerToolBar::OnGetGridSpacingMenuContent()
{
	FMenuBuilder MenuBuilder(false, CommandList);

	TArray<int32> GridSpacingValues = {10, 20, 30, 40, 50, 100};

	MenuBuilder.BeginSection(FName("Snap"), INVTEXT("Snap Sizes"));
	for (const int32 Value : GridSpacingValues)
	{
		FString TextString = FString::FromInt(Value);
		FUIAction GridSpacingAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerToolBar::EditGridSpacingValue, Value), FCanExecuteAction(), FIsActionChecked::CreateSP(this, &SVirtualShapeDesignerToolBar::GridSpacingIsSelected, Value));
		MenuBuilder.AddMenuEntry(FText::AsNumber(Value), FText::Format(INVTEXT("Sets grid size to {0}"), FText::AsNumber(Value)), FSlateIcon(), GridSpacingAction, NAME_None, EUserInterfaceActionType::RadioButton);
	}
	MenuBuilder.EndSection();
	
	return MenuBuilder.MakeWidget();
}

void SVirtualShapeDesignerToolBar::EditGridSpacingValue(const int32 Spacing)
{
	GridSpacingValue = Spacing;
	OnGridSpacingChanged.ExecuteIfBound(Spacing);
}

bool SVirtualShapeDesignerToolBar::GridSpacingIsSelected(const int32 Spacing)
{
	return Spacing == GridSpacingValue;
}
