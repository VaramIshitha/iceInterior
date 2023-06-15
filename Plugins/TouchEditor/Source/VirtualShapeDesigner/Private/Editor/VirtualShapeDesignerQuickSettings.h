// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

class UTouchInterfaceSettings;

class SVirtualShapeDesignerQuickSettings : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVirtualShapeDesignerQuickSettings)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	const ISlateStyle& GetSlateStyle() const;

private:
	void HandleOnSliderValueCommitted(float Value, ETextCommit::Type CommitType) const;

	UTouchInterfaceSettings* RuntimeSettings;
};
