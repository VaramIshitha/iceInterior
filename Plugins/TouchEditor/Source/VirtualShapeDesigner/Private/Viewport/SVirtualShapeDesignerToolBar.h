// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once
#include "SViewportToolBar.h"

class FExtender;
class FUICommandList;

enum class ECheckBoxState : uint8;

//Todo: Grid snapping and spacing in setting ?

/**
 * 
 */
class SVirtualShapeDesignerToolBar : public SViewportToolBar
{
	DECLARE_DELEGATE_OneParam(FOnGridSnappingChangedSignature, bool);
	DECLARE_DELEGATE_OneParam(FOnGridSpacingChangedSignature, int32)

public:
	SLATE_BEGIN_ARGS(SVirtualShapeDesignerToolBar){}
		SLATE_ARGUMENT(TSharedPtr<FUICommandList>, CommandList)
		SLATE_ARGUMENT(TSharedPtr<FExtender>, Extenders)
		SLATE_ARGUMENT(int32, GridSpacing)
		SLATE_ARGUMENT(bool, EnableGridSnapping)
		SLATE_EVENT(FOnGridSnappingChangedSignature, OnGridSnappingChanged)
		SLATE_EVENT(FOnGridSpacingChangedSignature, OnGridSpacingChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedRef<SWidget> MakeToolBar(const TSharedPtr<FExtender> InExtenders);

private:
	// Begin Grid Snapping
	void ToggleGridSnapping(ECheckBoxState State);
	ECheckBoxState GridSnappingEnabled() const;
	FText GetGridSpacingLabel() const;

	TSharedRef<SWidget> OnGetGridSpacingMenuContent();
	
	void EditGridSpacingValue(const int32 Spacing);
	bool GridSpacingIsSelected(const int32 Spacing);

	bool bGridSnappingEnabled;
	int32 GridSpacingValue;
	// End Grid Snapping

	TSharedPtr<FUICommandList> CommandList;
	
	FOnGridSnappingChangedSignature OnGridSnappingChanged;
	FOnGridSpacingChangedSignature OnGridSpacingChanged;
};
