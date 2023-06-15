// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Input/DragAndDrop.h"

enum class EControlType:uint8;

/**
 * This drag drop operation allows palette item from the palette to be dragged and dropped into the designer surface
 * in order to spawn new virtual control widgets.
 */
class FPaletteItemDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FPaletteItemDragDropOp, FDragDropOperation)

	//DragAndDrop
	virtual void OnDragged(const FDragDropEvent& DragDropEvent) override;
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;
	virtual FVector2D GetDecoratorPosition() const override;
	virtual FCursorReply OnCursorQuery() override;
	//DragAndDrop
	
	EControlType GetControlType() const { return ControlType; }
	void SetCursor(EMouseCursor::Type Cursor);
	
	static TSharedRef<FPaletteItemDragDropOp> New(const TSharedPtr<SWidget>& InDraggedWidget, const EControlType InType, const FName InIconName, const FText InName);

private:
	TSharedPtr<SWidget> DraggedWidget;

	EControlType ControlType;

	FName IconStyleName;
	FText ItemName;

	FVector2D CurrentPosition;

	EMouseCursor::Type MouseCursor;
};
