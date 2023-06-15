// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

#include "IDetailCustomization.h"
#include "SSingleObjectDetailsPanel.h"

struct FVirtualControl;
class UVirtualControlSetup;
class FVirtualControlDesignerEditor;

class SVirtualControlDesignerEditorDetailsPanel : public SSingleObjectDetailsPanel
{
public:
	SLATE_BEGIN_ARGS(SVirtualControlDesignerEditorDetailsPanel){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments InArgs, TSharedPtr<FVirtualControlDesignerEditor> InTouchDesignerEditor);

	// Begin SSingleObjectDetailPanel Interface
	virtual UObject* GetObjectToObserve() const override;

	virtual TSharedRef<SWidget> PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget) override;
	// End SSingleObjectDetailPanel Interface

	virtual void Refresh();

private:
	// Pointer back to owning Touch Designer Editor instance (the keeper of state)
	TWeakPtr<FVirtualControlDesignerEditor> TouchDesignerEditorPtr;
};

class FTouchDesignerEditor_DetailsTab : public IDetailCustomization
{

	FTouchDesignerEditor_DetailsTab();
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** Begin IDetailCustomization Interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;	
	/** End IDetailCustomization Interface */
	
private:
	void BuildGeneralSettings(IDetailLayoutBuilder& DetailBuilder);
	void BuildBackgroundSettings(IDetailLayoutBuilder& DetailBuilder);
	void BuildControlSettings(IDetailLayoutBuilder& DetailBuilder, const UVirtualControlSetup*& TouchDesignerInterface);

	void BuildButtonSettings(IDetailLayoutBuilder& DetailBuilder, const FVirtualControl& VirtualControl);
	void BuildJoystickSettings(IDetailLayoutBuilder& DetailBuilder/*const UVirtualControlSetup& VCS*/);
	void BuildTouchRegionSettings(IDetailLayoutBuilder& DetailBuilder/*const UVirtualControlSetup& VCS*/);

	void ForceRefresh() const;
	
	void OnOpacityPropertyChanged() const;
	void OnControlPropertyChanged() const;
	
	// Associated detail layout builder
	IDetailLayoutBuilder* DetailLayoutBuilder;

	bool InLandscapeOrientation;

	TWeakObjectPtr<UVirtualControlSetup> VirtualControlSetupEdited;
	
	//Selected Element in VirtualControls array
	TSharedPtr<IPropertyHandle> ArrayElement;

	TWeakPtr<FVirtualControlDesignerEditor> TouchDesignerEditorPtr;
};
