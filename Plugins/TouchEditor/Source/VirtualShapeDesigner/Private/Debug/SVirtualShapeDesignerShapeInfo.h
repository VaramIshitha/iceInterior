// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

//Todo: Show useful info with viewport like number of point, line, corner...
//Todo: When data is generated, show time needed
//Todo: Use MessageLogModule ?

class UVirtualShape;

class SVirtualShapeDesignerShapeInfo : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVirtualShapeDesignerShapeInfo){}
		SLATE_ATTRIBUTE(UVirtualShape*, VirtualShapeEdited)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	void UpdateInfo();
	
private:
	TSharedRef<SWidget> ConstructViewportInfo();

	TSharedRef<SWidget> GenerateTitle(const FText Title);
	
	const ISlateStyle& GetSlateStyle() const;

	TAttribute<UVirtualShape*> VirtualShapeEdited;
};
