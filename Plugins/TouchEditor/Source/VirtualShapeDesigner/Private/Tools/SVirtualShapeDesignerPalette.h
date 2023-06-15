// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

enum class EDrawTools;
class FVirtualShapeDesignerEditor;

//Todo: Animated icon on hover

class SVirtualShapeDesignerPalette : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SVirtualShapeDesignerPalette){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualShapeDesignerEditor> InVirtualShapeDesignerEditor);

	void MakeToolWidgets();
	
	void AddTool(FName FriendlyName, EDrawTools Type, FName BrushName);
	
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	const ISlateStyle& GetSlateStyle() const;
	
	void SelectTool(EDrawTools Type, bool bPropagate);

	//void EnableTool();
	//void DisableTool();

	FButtonStyle UnselectedStyle;
	FButtonStyle SelectedStyle;

private:
	FReply HandleOnToolSelected(EDrawTools ToolType);

	TWeakPtr<FVirtualShapeDesignerEditor> DesignerEditor;
	
	TSharedPtr<SVerticalBox> ToolContainer;

	TMap<EDrawTools, TSharedPtr<SButton>> Tools;

	EDrawTools PreviousModeSelected;
};
