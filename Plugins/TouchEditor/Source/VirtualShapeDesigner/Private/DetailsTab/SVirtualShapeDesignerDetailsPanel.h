// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "SSingleObjectDetailsPanel.h"

class UVirtualShape;
class FVirtualShapeDesignerEditor;

//Todo: While i do not make two mode, hide some value in viewport mode (all boolean) and hide some value in drawer mode (reference image, friendly name...)

class SVirtualShapeDesignerDetailsPanel : public SSingleObjectDetailsPanel
{
public:
	SLATE_BEGIN_ARGS(SVirtualShapeDesignerDetailsPanel){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualShapeDesignerEditor> InVirtualShapeDesigner);

	virtual UObject* GetObjectToObserve() const override;

	virtual TSharedRef<SWidget> PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget) override;

	virtual void Refresh();

private:
	TWeakPtr<FVirtualShapeDesignerEditor> VirtualShapeDesignerPtr;
};

class FVirtualShapeDesignerDetailsPanelCustomization : public IDetailCustomization
{
	FVirtualShapeDesignerDetailsPanelCustomization();
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void ForceRefresh() const;

	IDetailLayoutBuilder* DetailLayoutBuilder;

	TWeakObjectPtr<UVirtualShape> VirtualShapeEdited;

	TWeakPtr<FVirtualShapeDesignerEditor> VirtualShapeDesignerEditor;
};
