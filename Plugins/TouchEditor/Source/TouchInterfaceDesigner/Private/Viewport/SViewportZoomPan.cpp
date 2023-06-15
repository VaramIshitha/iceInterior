// Copyright Lost in Game Studio. All Rights Reserved

#include "SViewportZoomPan.h"

void SViewportZoomPan::Construct(const FArguments& InArgs)
{
	bHasRelativeLayoutScale = true;

	ViewOffset = InArgs._ViewOffset;
	ZoomAmount = InArgs._ZoomAmount;

	ChildSlot
	[
		InArgs._Content.Widget
	];
}

void SViewportZoomPan::SetContent(const TSharedRef<SWidget>& InContent)
{
	ChildSlot
	[
		InContent
	];
}

void SViewportZoomPan::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const EVisibility ChildVisibility = ChildSlot.GetWidget()->GetVisibility();
	if (ArrangedChildren.Accepts(ChildVisibility))
	{
#if ENGINE_MAJOR_VERSION > 4
		const FMargin SlotPadding(ChildSlot.GetPadding());
#else
		const FMargin SlotPadding(ChildSlot.SlotPadding.Get());
#endif		

		const AlignmentArrangeResult XResult = AlignChild<Orient_Horizontal>(AllottedGeometry.GetLocalSize().X, ChildSlot, SlotPadding, 1);
		const AlignmentArrangeResult YResult = AlignChild<Orient_Vertical>(AllottedGeometry.GetLocalSize().Y, ChildSlot, SlotPadding, 1);
		
		const FVector2D ChildOffset = FVector2D(XResult.Offset, YResult.Offset) - ViewOffset.Get();
		
		ArrangedChildren.AddWidget(ChildVisibility, AllottedGeometry.MakeChild(ChildSlot.GetWidget(), ChildOffset, ChildSlot.GetWidget()->GetDesiredSize(), ZoomAmount.Get()));
	}
}

float SViewportZoomPan::GetRelativeLayoutScale(const int32 ChildIndex, float LayoutScaleMultiplier) const
{
	return ZoomAmount.Get();
}
