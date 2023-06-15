// Copyright Lost in Game Studio. All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SPaletteTab;
enum class EControlType : uint8;

/**
 * 
 */
class SPaletteItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaletteItem)
		{
		}
	SLATE_ARGUMENT(FText, TooltipText)
	SLATE_ARGUMENT(FText, ItemName)
	SLATE_ARGUMENT(FName, ImageName)
	SLATE_ARGUMENT(EControlType, Type)

	SLATE_EVENT(FSimpleDelegate, OnClicked)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<SPaletteTab> PaletteTab);

	//SWidget
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	//SWidget

private:
	FReply HandleItemClicked(const FGeometry& Geometry, const FPointerEvent& Event);

	const FSlateBrush* GetBorderImage() const;
	FSlateColor GetBorderColor() const;
	
	FText TooltipText;
	FText ItemName;
	FName ImageName;

	FSimpleDelegate OnClicked;

	EControlType Type_Internal;
};
