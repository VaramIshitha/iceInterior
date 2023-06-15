// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

class TOUCHINTERFACEDESIGNER_API SViewportZoomPan : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SViewportZoomPan) {}
		SLATE_DEFAULT_SLOT(FArguments, Content)

		SLATE_ATTRIBUTE(FVector2D, ViewOffset)
		SLATE_ATTRIBUTE(float, ZoomAmount)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetContent(const TSharedRef<SWidget>& InContent);

protected:
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

	virtual float GetRelativeLayoutScale(const int32 ChildIndex, float LayoutScaleMultiplier) const override;

	TAttribute<FVector2D> ViewOffset;
	TAttribute<float> ZoomAmount;
	
};
