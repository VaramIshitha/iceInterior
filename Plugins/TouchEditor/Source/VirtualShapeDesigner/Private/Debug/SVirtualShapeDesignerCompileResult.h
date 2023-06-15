// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

//Todo: Show time needed to recognize shape with drawer
//Todo: Use MessageLogModule ? (Developer/MessageLog)

class SVirtualShapeDesignerCompileResult : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVirtualShapeDesignerCompileResult){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	void UpdateCompileResult();

private:
	const ISlateStyle& GetSlateStyle() const;
};
