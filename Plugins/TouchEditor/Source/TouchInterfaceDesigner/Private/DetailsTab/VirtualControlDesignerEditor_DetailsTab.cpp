// Copyright Lost in Game Studio. All Rights Reserved

#include "VirtualControlDesignerEditor_DetailsTab.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "TouchInterfaceDesignerModule.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "Classes/VirtualControlSetup.h"
#include "Editor/TouchInterfaceDesignerStyle.h"
#include "Settings/TouchInterfaceSettings.h"

#define LOCTEXT_NAMESPACE "TouchDesignerEditorDetailsTab"
	
void SVirtualControlDesignerEditorDetailsPanel::Construct(const FArguments InArgs, TSharedPtr<FVirtualControlDesignerEditor> InTouchDesignerEditor)
{
	TouchDesignerEditorPtr = InTouchDesignerEditor;
	SSingleObjectDetailsPanel::Construct(SSingleObjectDetailsPanel::FArguments().HostCommandList(InTouchDesignerEditor->GetToolkitCommands()),true, true);

	if (FSlateApplication::IsInitialized())
	{
		TArray<UObject*> SelectedObjects;
		SelectedObjects.Add(GetObjectToObserve());
		PropertyView->SetObjects(SelectedObjects, true);
	}
}

UObject* SVirtualControlDesignerEditorDetailsPanel::GetObjectToObserve() const
{
	return TouchDesignerEditorPtr.Pin()->GetVirtualControlSetup();
}

TSharedRef<SWidget> SVirtualControlDesignerEditorDetailsPanel::PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget)
{
	return SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.FillHeight(1)
	[
		PropertyEditorWidget
	];
}

void SVirtualControlDesignerEditorDetailsPanel::Refresh()
{
	if (FSlateApplication::IsInitialized())
	{
		//UE_LOG(LogTemp, Log, TEXT("Force Refresh"))
		//TArray<UObject*> SelectedObjects;
		//SelectedObjects.Add(GetObjectToObserve());
		//PropertyView->SetObjects(SelectedObjects, true);
		PropertyView->ForceRefresh();
	}
}

//CUSTOMIZATION

FTouchDesignerEditor_DetailsTab::FTouchDesignerEditor_DetailsTab()
{
	DetailLayoutBuilder = nullptr;
	InLandscapeOrientation = true;
}

TSharedRef<IDetailCustomization> FTouchDesignerEditor_DetailsTab::MakeInstance()
{
	return MakeShareable(new FTouchDesignerEditor_DetailsTab());
}

void FTouchDesignerEditor_DetailsTab::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailLayoutBuilder = &DetailBuilder;

	// Get Touch Designer Interface that is modified as UObject 
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	TArray<UObject*> StrongObjects;
	CopyFromWeakArray(StrongObjects, ObjectsBeingCustomized);

	if (StrongObjects.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No object to modify"));
		return;
	}

	VirtualControlSetupEdited = Cast<UVirtualControlSetup>(StrongObjects[0]);
	if (!VirtualControlSetupEdited.Get(false))
	{
		UE_LOG(LogTemp, Error, TEXT("Virtual Control Setup is null"));
		return;
	}
	
	//const UVirtualControlSetup* TouchDesignerInterface = Cast<UVirtualControlSetup>(StrongObjects[0]);
	const UVirtualControlSetup* TouchDesignerInterface = VirtualControlSetupEdited.Get();

	//check(TouchDesignerInterface);
	
	FTouchInterfaceDesignerModule* EditorModule = &FModuleManager::LoadModuleChecked<FTouchInterfaceDesignerModule>("TouchInterfaceDesigner");
	TouchDesignerEditorPtr = EditorModule->GetVirtualControlDesignerEditor();

	//Hide Category from UVirtualControlSetup because we have custom category inside Detail Panel. All subcategory is hidden too.
	DetailBuilder.HideCategory("Virtual Control Setup");

	InLandscapeOrientation = TouchDesignerEditorPtr.Pin()->IsOrientationInLandscape();
	//Get viewport orientation (this is saved in UObject to avoid to include viewport
	//InLandscapeOrientation = TouchDesignerInterface->IsInLandscapeOrientation();
	
	if (TouchDesignerEditorPtr.IsValid())
	{
		switch (TouchDesignerEditorPtr.Pin()->GetDetailTabState())
		{
		case DTS_ShowGeneralProperties:
			BuildGeneralSettings(DetailBuilder);
			break;
		case DTS_ShowBackgroundProperties:
			BuildBackgroundSettings(DetailBuilder);
			break;
		case DTS_ShowControlProperties:
			BuildControlSettings(DetailBuilder, TouchDesignerInterface);
			break;
		}
	}
}

void FTouchDesignerEditor_DetailsTab::BuildGeneralSettings(IDetailLayoutBuilder& DetailBuilder)
{
	// Make categories
	IDetailCategoryBuilder& OpacityCat = DetailBuilder.EditCategory("Opacity", LOCTEXT("Opacity", "Opacity"));
	IDetailCategoryBuilder& ResetCat = DetailBuilder.EditCategory("Reset", LOCTEXT("Reset", "Reset/Recenter"));
	IDetailCategoryBuilder& DelayCat = DetailBuilder.EditCategory("Delay", LOCTEXT("Delay", "Delay"));
	IDetailCategoryBuilder& AdvancedCat = DetailBuilder.EditCategory("Advanced", LOCTEXT("Advanced", "Advanced"));
	
	// Get property in Touch Designer Interface
	const TSharedPtr<IPropertyHandle> ActiveOpacityProperty = DetailLayoutBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, ActiveOpacity));
	const TSharedPtr<IPropertyHandle> InactiveOpacityProperty = DetailLayoutBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, InactiveOpacity));
	const TSharedPtr<IPropertyHandle> TimeUntilResetProperty = DetailLayoutBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, TimeUntilReset));
	const TSharedPtr<IPropertyHandle> TimeUntilDeactivatedProperty = DetailLayoutBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, TimeUntilDeactivated));
	const TSharedPtr<IPropertyHandle> ActivationDelayProperty = DetailLayoutBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, ActivationDelay));
	const TSharedPtr<IPropertyHandle> StartupDelayProperty = DetailLayoutBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, StartupDelay));
	const TSharedPtr<IPropertyHandle> RotationModeProperty = DetailLayoutBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, bCalculatePortraitPositionAtRuntime));

	// Bind delegate on property value changed
	ActiveOpacityProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(TouchDesignerEditorPtr.Pin().Get(), &FVirtualControlDesignerEditor::NotifyOpacityChange));
	InactiveOpacityProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(TouchDesignerEditorPtr.Pin().Get(), &FVirtualControlDesignerEditor::NotifyOpacityChange));

	// Add property handle to opacity category
	OpacityCat.AddProperty(ActiveOpacityProperty);
	OpacityCat.AddProperty(InactiveOpacityProperty);
	OpacityCat.AddProperty(TimeUntilDeactivatedProperty);

	// Add property handle to reset category
	ResetCat.AddProperty(TimeUntilResetProperty);

	// Add property handle to delay category
	DelayCat.AddProperty(StartupDelayProperty);
	DelayCat.AddProperty(ActivationDelayProperty);

	// Add property handle to advanced category
	AdvancedCat.AddProperty(RotationModeProperty);
}

void FTouchDesignerEditor_DetailsTab::BuildBackgroundSettings(IDetailLayoutBuilder& DetailBuilder)
{
	// Get Background Settings Struct property in VirtualControlSetup object
	const TSharedPtr<IPropertyHandle> BackgroundSettingsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, BackgroundSettings));

	IDetailCategoryBuilder& BackgroundCat = DetailBuilder.EditCategory("Background", LOCTEXT("BackgroundSettingsKey", "Background Settings"));

	const TSharedPtr<IPropertyHandle> BackgroundColor = BackgroundSettingsProperty->GetChildHandle("FillColor");
	const TSharedPtr<IPropertyHandle> ImageProperty = BackgroundSettingsProperty->GetChildHandle("Image");
	const TSharedPtr<IPropertyHandle> UseBrushSizeProperty = BackgroundSettingsProperty->GetChildHandle("bFill");
	const TSharedPtr<IPropertyHandle> DrawDeviceMockup = BackgroundSettingsProperty->GetChildHandle("bEnableDeviceMockup");

	BackgroundCat.AddProperty(BackgroundColor);
	BackgroundCat.AddProperty(ImageProperty);
	BackgroundCat.AddProperty(UseBrushSizeProperty);
	BackgroundCat.AddProperty(DrawDeviceMockup);
}

void FTouchDesignerEditor_DetailsTab::BuildControlSettings(IDetailLayoutBuilder& DetailBuilder, const UVirtualControlSetup*& TouchDesignerInterface)
{
	//DetailBuilder.HideCategory("Virtual Control Setup|Virtual Control");

	if (!TouchDesignerInterface->VirtualControls.IsValidIndex(TouchDesignerInterface->GetSelectedControlIndex())) return;

	// Get Controls Array property (variables) in Touch Designer Interface object
	const TSharedPtr<IPropertyHandle> VirtualButtonsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UVirtualControlSetup, VirtualControls));

	//Get array from virtual buttons property
	const TSharedPtr<IPropertyHandleArray> Array = VirtualButtonsProperty->AsArray();

	//Get selected array element and save in variable
	ArrayElement = Array->GetElement(TouchDesignerInterface->GetSelectedControlIndex());
	
	const FVirtualControl VirtualControl = VirtualControlSetupEdited->GetSelectedControl(); /*TouchDesignerInterface->VirtualControls[TouchDesignerInterface->GetSelectedControlIndex()];*/
	
	switch (TouchDesignerInterface->GetSelectedControlType())
	{
	case EControlType::Button:
		BuildButtonSettings(DetailBuilder, VirtualControl);
		break;
	case EControlType::Joystick:
		BuildJoystickSettings(DetailBuilder);
		break;
	case EControlType::TouchRegion:
		BuildTouchRegionSettings(DetailBuilder);
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("ERROR! Unknow control type in detail tab"));
		break;
	}
}

void FTouchDesignerEditor_DetailsTab::BuildButtonSettings(IDetailLayoutBuilder& DetailBuilder, const FVirtualControl& VirtualControl)
{
	//Make buttons categories
	IDetailCategoryBuilder& ControlSettingsCat = DetailBuilder.EditCategory("ControlSettings", INVTEXT("Control"), ECategoryPriority::Important);
	IDetailCategoryBuilder& VisualSettingsCat = DetailBuilder.EditCategory("VisualSettings", INVTEXT("Visual"));
	IDetailCategoryBuilder& InputSettingsCat = DetailBuilder.EditCategory("InputSettings", INVTEXT("Input"));
	IDetailCategoryBuilder& SoundSettingsCat = DetailBuilder.EditCategory("SoundSettings", INVTEXT("Sound"));

	//Control
	const TSharedPtr<IPropertyHandle> StartHiddenProperty = ArrayElement->GetChildHandle("bStartHidden");
	const TSharedPtr<IPropertyHandle> RecenterOnTouchProperty = ArrayElement->GetChildHandle("bRecenterOnTouch");
	const TSharedPtr<IPropertyHandle> LandscapePositionProperty = ArrayElement->GetChildHandle("LandscapeCenter");
	const TSharedPtr<IPropertyHandle> PortraitPositionProperty = ArrayElement->GetChildHandle("PortraitCenter");
	
	const TSharedPtr<IPropertyHandle> ParentOffsetProperty = ArrayElement->GetChildHandle("ParentOffset");
	const TSharedPtr<IPropertyHandle> IsChildProperty = ArrayElement->GetChildHandle("bIsChild");
	const TSharedPtr<IPropertyHandle> ChildrenProperty = ArrayElement->GetChildHandle("Children");
	const TSharedPtr<IPropertyHandle> MoveWhenParentRecenterProperty = ArrayElement->GetChildHandle("bMoveWhenParentRecenter");
	
	//Visual
	const TSharedPtr<IPropertyHandle> VisualLayerProperty = ArrayElement->GetChildHandle("VisualLayers");
	//Todo: Make navigation like MakePlugin Tutorial (use MaxLayer setting)
 	const TSharedPtr<IPropertyHandle> VisualSizeProperty = ArrayElement->GetChildHandle("VisualSize");

	//Input
	const TSharedPtr<IPropertyHandle> BlockTouchRegionProperty = ArrayElement->GetChildHandle("bBlockTouchRegion");
	const TSharedPtr<IPropertyHandle> InteractionShapeProperty = ArrayElement->GetChildHandle("InteractionShape");
	const TSharedPtr<IPropertyHandle> InteractionRadiusProperty = ArrayElement->GetChildHandle("InteractionRadiusSize");
	const TSharedPtr<IPropertyHandle> InteractionSizeProperty = ArrayElement->GetChildHandle("InteractionSize");
	const TSharedPtr<IPropertyHandle> ButtonInputKeyProperty = ArrayElement->GetChildHandle("ButtonInputKey");

	//Enhanced Input
	const TSharedPtr<IPropertyHandle> ButtonActionProperty = ArrayElement->GetChildHandle("ButtonAction");

	//Event
	const TSharedPtr<IPropertyHandle> VirtualControlEventProperty = ArrayElement->GetChildHandle("VirtualControlEvent");

	//Sound
	const TSharedPtr<IPropertyHandle> SoundKeyProperty = ArrayElement->GetChildHandle("PressedSound");
	
	
	ControlSettingsCat.AddProperty(StartHiddenProperty);
	ControlSettingsCat.AddProperty(RecenterOnTouchProperty);
	
	
	//If virtual control is a child, show text and offset from parent
	bool bIsChild;
	IsChildProperty->GetValue(bIsChild);
	if (bIsChild)
	{
		const TSharedPtr<IPropertyHandle> ParentNameProperty = ArrayElement->GetChildHandle("ParentName");
		FString ParentName;
		ParentNameProperty->GetValue(ParentName);
		//const FString ParentState = FString::Printf(TEXT("Is relative to %s"), *ParentName);

		IDetailGroup& LinkGroup = ControlSettingsCat.AddGroup(FName("Position"), INVTEXT("Position"), false, true);
		LinkGroup.AddWidgetRow()
		[
			SNew(SHorizontalBox)
			
			//Text to indicate the current parent of child
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Is relative to"))
				.Font(DetailBuilder.GetDetailFont())
			]
			
			//Border that can be hover to show parent control
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(10.0f, 0.0f))
			[
				SNew(SBorder)
				.Padding(FMargin(10.0f, 2.0f))
				.BorderImage(FTouchInterfaceDesignerStyle::Get().GetBrush("RoundedWhiteBox"))
				.BorderBackgroundColor(FSlateColor(FColor::Silver))
				.ForegroundColor(FSlateColor(FLinearColor::Black))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ParentName))
					.Font(DetailBuilder.GetDetailFontBold())
				]
			]
		];

		LinkGroup.AddPropertyRow(ParentOffsetProperty.ToSharedRef());
		LinkGroup.AddPropertyRow(MoveWhenParentRecenterProperty.ToSharedRef());
	}
	else
	{
		//Show only property needed based on current editor orientation state
		if (InLandscapeOrientation)
		{
			ControlSettingsCat.AddProperty(LandscapePositionProperty);
		}
		else
		{
			ControlSettingsCat.AddProperty(PortraitPositionProperty);
		}
	}

	//Todo: Make custom row for VisualLayers
	VisualLayerProperty->SetPropertyDisplayName(INVTEXT("Button Layers"));
	VisualSettingsCat.AddProperty(VisualLayerProperty);
	VisualSettingsCat.AddProperty(VisualSizeProperty);

	InputSettingsCat.AddProperty(BlockTouchRegionProperty);
	
	InputSettingsCat.AddProperty(InteractionShapeProperty);
	InteractionShapeProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FTouchDesignerEditor_DetailsTab::ForceRefresh));
	InputSettingsCat.AddProperty(InteractionSizeProperty);
	InputSettingsCat.AddProperty(InteractionRadiusProperty);

	if (GetDefault<UTouchInterfaceSettings>()->bUseEnhancedInput)
	{
		InputSettingsCat.AddProperty(ButtonActionProperty);
	}
	else
	{
		InputSettingsCat.AddProperty(ButtonInputKeyProperty);
	}

	InputSettingsCat.AddProperty(VirtualControlEventProperty);
	
	SoundSettingsCat.AddProperty(SoundKeyProperty);

	// Set Delegate for Control value
	ArrayElement->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FTouchDesignerEditor_DetailsTab::OnControlPropertyChanged));
}

void FTouchDesignerEditor_DetailsTab::BuildJoystickSettings(IDetailLayoutBuilder& DetailBuilder/*const UVirtualControlSetup& VCS*/)
{
	//Make joystick categories
	IDetailCategoryBuilder& ControlSettingsCat = DetailBuilder.EditCategory("ControlSettings", INVTEXT("Control"), ECategoryPriority::Important);
	IDetailCategoryBuilder& VisualSettingsCat = DetailBuilder.EditCategory("VisualSettings", INVTEXT("Visual"));
	IDetailCategoryBuilder& InputSettingsCat = DetailBuilder.EditCategory("InputSettings", INVTEXT("Input"));
	IDetailCategoryBuilder& AutoMoveCat = DetailBuilder.EditCategory("AutoMove", INVTEXT("Auto Move"));
	IDetailCategoryBuilder& AutoRunCat = DetailBuilder.EditCategory("AutoRun", INVTEXT("Auto Run"));

	//Control
	const TSharedPtr<IPropertyHandle> StartHiddenProperty = ArrayElement->GetChildHandle("bStartHidden");
	const TSharedPtr<IPropertyHandle> RecenterOnTouchProperty = ArrayElement->GetChildHandle("bRecenterOnTouch");
	const TSharedPtr<IPropertyHandle> LandscapePositionProperty = ArrayElement->GetChildHandle("LandscapeCenter");
	const TSharedPtr<IPropertyHandle> PortraitPositionProperty = ArrayElement->GetChildHandle("PortraitCenter");

	const TSharedPtr<IPropertyHandle> ParentOffsetProperty = ArrayElement->GetChildHandle("ParentOffset");
	const TSharedPtr<IPropertyHandle> IsChildProperty = ArrayElement->GetChildHandle("bIsChild");
	const TSharedPtr<IPropertyHandle> ChildrenProperty = ArrayElement->GetChildHandle("Children");
	const TSharedPtr<IPropertyHandle> MoveWhenParentRecenterProperty = ArrayElement->GetChildHandle("bMoveWhenParentRecenter");

	//Visual
	const TSharedPtr<IPropertyHandle> VisualLayerProperty = ArrayElement->GetChildHandle("VisualLayers");
	const TSharedPtr<IPropertyHandle> VisualSizeProperty = ArrayElement->GetChildHandle("VisualSize");
	const TSharedPtr<IPropertyHandle> ThumbSizeProperty = ArrayElement->GetChildHandle("ThumbSize");

	//Input
	const TSharedPtr<IPropertyHandle> BlockTouchRegionProperty = ArrayElement->GetChildHandle("bBlockTouchRegion");
	const TSharedPtr<IPropertyHandle> InteractionShapeProperty = ArrayElement->GetChildHandle("InteractionShape");
	const TSharedPtr<IPropertyHandle> InteractionRadiusProperty = ArrayElement->GetChildHandle("InteractionRadiusSize");
	const TSharedPtr<IPropertyHandle> InteractionSizeProperty = ArrayElement->GetChildHandle("InteractionSize");
	const TSharedPtr<IPropertyHandle> HorizontalInputProperty = ArrayElement->GetChildHandle("HorizontalInputKey");
	const TSharedPtr<IPropertyHandle> VerticalInputProperty = ArrayElement->GetChildHandle("VerticalInputKey");

	//Enhanced Input
	const TSharedPtr<IPropertyHandle> JoystickActionProperty = ArrayElement->GetChildHandle("JoystickAction");
	//Enhanced Input
	
	const TSharedPtr<IPropertyHandle> PressEventProperty = ArrayElement->GetChildHandle("bSendPressAndReleaseEvent");
	const TSharedPtr<IPropertyHandle> ButtonInputKeyProperty = ArrayElement->GetChildHandle("ButtonInputKey");
	const TSharedPtr<IPropertyHandle> InputScaleProperty = ArrayElement->GetChildHandle("InputScale");
	const TSharedPtr<IPropertyHandle> ThumbValueCurveProperty = ArrayElement->GetChildHandle("ThumbValueCurve");
	const TSharedPtr<IPropertyHandle> UseTurnRateProperty = ArrayElement->GetChildHandle("bUseTurnRate");
	const TSharedPtr<IPropertyHandle> TurnRateProperty = ArrayElement->GetChildHandle("TurnRate");

	//Event
	const TSharedPtr<IPropertyHandle> VirtualControlEventProperty = ArrayElement->GetChildHandle("VirtualControlEvent");

	//Auto Move
	const TSharedPtr<IPropertyHandle> AutoMoveProperty = ArrayElement->GetChildHandle("bAutoMove");
	const TSharedPtr<IPropertyHandle> LockIconProperty = ArrayElement->GetChildHandle("LockIcon");

	//Auto Run
	const TSharedPtr<IPropertyHandle> DragToSprintProperty = ArrayElement->GetChildHandle("bDragToSprint");
	const TSharedPtr<IPropertyHandle> SprintButtonProperty = ArrayElement->GetChildHandle("SprintButton");
	const TSharedPtr<IPropertyHandle> SprintButtonSizeProperty = ArrayElement->GetChildHandle("SprintButtonVisualSize");
	const TSharedPtr<IPropertyHandle> SprintInputKeyProperty = ArrayElement->GetChildHandle("SprintInputKey");
	const TSharedPtr<IPropertyHandle> SprintActionProperty = ArrayElement->GetChildHandle("SprintAction");

	//Control
	ControlSettingsCat.AddProperty(StartHiddenProperty);
	ControlSettingsCat.AddProperty(RecenterOnTouchProperty);
	
	//If virtual control is a child, show text and offset from parent
	bool bIsChild;
	IsChildProperty->GetValue(bIsChild);
	if (bIsChild)
	{
		const TSharedPtr<IPropertyHandle> ParentNameProperty = ArrayElement->GetChildHandle("ParentName");
		FString ParentName;
		ParentNameProperty->GetValue(ParentName);
		//const FString ParentState = FString::Printf(TEXT("Is relative to %s"), *ParentName);

		IDetailGroup& LinkGroup = ControlSettingsCat.AddGroup(FName("Position"), INVTEXT("Position"), false, true);
		LinkGroup.AddWidgetRow()
		[
			SNew(SHorizontalBox)
			
			//Text to indicate the current parent of child
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Is relative to"))
				.Font(DetailBuilder.GetDetailFont())
			]
			
			//Border that can be hover to show parent control
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(10.0f, 0.0f))
			[
				SNew(SBorder)
				.Padding(FMargin(10.0f, 2.0f))
				.BorderImage(FTouchInterfaceDesignerStyle::Get().GetBrush("RoundedWhiteBox"))
				.BorderBackgroundColor(FSlateColor(FColor::Silver))
				.ForegroundColor(FSlateColor(FLinearColor::Black))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ParentName))
					.Font(DetailBuilder.GetDetailFontBold())
				]
			]
		];

		LinkGroup.AddPropertyRow(ParentOffsetProperty.ToSharedRef());
		LinkGroup.AddPropertyRow(MoveWhenParentRecenterProperty.ToSharedRef());
	}
	else
	{
		//Show only property needed based on current editor orientation state
		if (InLandscapeOrientation)
		{
			ControlSettingsCat.AddProperty(LandscapePositionProperty);
		}
		else
		{
			ControlSettingsCat.AddProperty(PortraitPositionProperty);
		}
	}

	//Visual
	VisualLayerProperty->SetPropertyDisplayName(INVTEXT("Joystick Layers"));
	VisualSettingsCat.AddProperty(VisualLayerProperty);
	VisualSettingsCat.AddProperty(VisualSizeProperty);
	VisualSettingsCat.AddProperty(ThumbSizeProperty);

	//Input
	InputSettingsCat.AddProperty(BlockTouchRegionProperty);
	InputSettingsCat.AddProperty(InteractionShapeProperty);
	InteractionShapeProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FTouchDesignerEditor_DetailsTab::ForceRefresh));
	InputSettingsCat.AddProperty(InteractionSizeProperty);
	InputSettingsCat.AddProperty(InteractionRadiusProperty);

	if (GetDefault<UTouchInterfaceSettings>()->bUseEnhancedInput)
	{
		InputSettingsCat.AddProperty(JoystickActionProperty);
	}
	else
	{
		InputSettingsCat.AddProperty(HorizontalInputProperty);
		InputSettingsCat.AddProperty(VerticalInputProperty);

		IDetailGroup& JoystickAdvancedGroup = InputSettingsCat.AddGroup(FName("PressAndRelease"), INVTEXT("Press and Release"));
		JoystickAdvancedGroup.AddPropertyRow(PressEventProperty.ToSharedRef());
		JoystickAdvancedGroup.AddPropertyRow(ButtonInputKeyProperty.ToSharedRef());
		
		InputSettingsCat.AddProperty(InputScaleProperty);
		InputSettingsCat.AddProperty(ThumbValueCurveProperty);

		InputSettingsCat.AddProperty(UseTurnRateProperty);
		UseTurnRateProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FTouchDesignerEditor_DetailsTab::ForceRefresh));
		
		bool bTurnRateEnabled;
		UseTurnRateProperty->GetValue(bTurnRateEnabled);
		if (bTurnRateEnabled)
		{
			InputSettingsCat.AddProperty(TurnRateProperty);
		}
	}

	InputSettingsCat.AddProperty(VirtualControlEventProperty);

	//Auto Move
	AutoMoveCat.AddProperty(AutoMoveProperty);
	AutoMoveCat.AddProperty(LockIconProperty);

	//Drag to sprint
	AutoRunCat.AddProperty(DragToSprintProperty);
	DragToSprintProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FTouchDesignerEditor_DetailsTab::ForceRefresh));

	bool bDragToSprintEnabled;
	DragToSprintProperty->GetValue(bDragToSprintEnabled);
	if (bDragToSprintEnabled)
	{
		AutoRunCat.AddProperty(SprintButtonProperty);
		AutoRunCat.AddProperty(SprintButtonSizeProperty);

		if (GetDefault<UTouchInterfaceSettings>()->bUseEnhancedInput)
		{
			AutoRunCat.AddProperty(SprintActionProperty);
		}
		else
		{
			AutoRunCat.AddProperty(SprintInputKeyProperty);
		}
	}

	// Set Delegate for Control value
	ArrayElement->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FTouchDesignerEditor_DetailsTab::OnControlPropertyChanged));
}

void FTouchDesignerEditor_DetailsTab::BuildTouchRegionSettings(IDetailLayoutBuilder& DetailBuilder/*const UVirtualControlSetup& VCS*/)
{
	//Make Touch Region categories
	IDetailCategoryBuilder& ControlSettingsCat = DetailBuilder.EditCategory("ControlSettings", INVTEXT("Control"), ECategoryPriority::Important);
	IDetailCategoryBuilder& InputSettingsCat = DetailBuilder.EditCategory("InputSettings", INVTEXT("Input"));

	//Control
	const TSharedPtr<IPropertyHandle> LandscapePositionProperty = ArrayElement->GetChildHandle("LandscapeCenter");
	const TSharedPtr<IPropertyHandle> PortraitPositionProperty = ArrayElement->GetChildHandle("PortraitCenter");

	const TSharedPtr<IPropertyHandle> ParentOffsetProperty = ArrayElement->GetChildHandle("ParentOffset");
	const TSharedPtr<IPropertyHandle> IsChildProperty = ArrayElement->GetChildHandle("bIsChild");
	const TSharedPtr<IPropertyHandle> ChildrenProperty = ArrayElement->GetChildHandle("Children");
	const TSharedPtr<IPropertyHandle> MoveWhenParentRecenterProperty = ArrayElement->GetChildHandle("bMoveWhenParentRecenter");

	//Input
	const TSharedPtr<IPropertyHandle> TouchRegionJoystickModeProperty = ArrayElement->GetChildHandle("bJoystickMode");
	const TSharedPtr<IPropertyHandle> InteractionSizeProperty = ArrayElement->GetChildHandle("InteractionSize");
	const TSharedPtr<IPropertyHandle> HorizontalInputProperty = ArrayElement->GetChildHandle("HorizontalInputKey");
	const TSharedPtr<IPropertyHandle> VerticalInputProperty = ArrayElement->GetChildHandle("VerticalInputKey");
	const TSharedPtr<IPropertyHandle> InputScaleProperty = ArrayElement->GetChildHandle("InputScale");

	//Enhanced Input
	const TSharedPtr<IPropertyHandle> JoystickActionProperty = ArrayElement->GetChildHandle("JoystickAction");

	//Event
	const TSharedPtr<IPropertyHandle> VirtualControlEventProperty = ArrayElement->GetChildHandle("VirtualControlEvent");
	
	
	//If virtual control is a child, show text and offset from parent
	bool bIsChild;
	IsChildProperty->GetValue(bIsChild);
	if (bIsChild)
	{
		const TSharedPtr<IPropertyHandle> ParentNameProperty = ArrayElement->GetChildHandle("ParentName");
		FString ParentName;
		ParentNameProperty->GetValue(ParentName);

		IDetailGroup& LinkGroup = ControlSettingsCat.AddGroup(FName("Position"), INVTEXT("Position"), false, true);
		LinkGroup.AddWidgetRow()
		[
			SNew(SHorizontalBox)
			
			//Text to indicate the current parent of child
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Is relative to"))
				.Font(DetailBuilder.GetDetailFont())
			]
			
			//Border that can be hover to show parent control
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(10.0f, 0.0f))
			[
				SNew(SBorder)
				.Padding(FMargin(10.0f, 2.0f))
				.BorderImage(FTouchInterfaceDesignerStyle::Get().GetBrush("RoundedWhiteBox"))
				.BorderBackgroundColor(FSlateColor(FColor::Silver))
				.ForegroundColor(FSlateColor(FLinearColor::Black))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ParentName))
					.Font(DetailBuilder.GetDetailFontBold())
				]
			]
		];

		LinkGroup.AddPropertyRow(ParentOffsetProperty.ToSharedRef());
		LinkGroup.AddPropertyRow(MoveWhenParentRecenterProperty.ToSharedRef());
	}
	else
	{
		//Show only property needed based on current editor orientation state
		if (InLandscapeOrientation)
		{
			ControlSettingsCat.AddProperty(LandscapePositionProperty);
		}
		else
		{
			ControlSettingsCat.AddProperty(PortraitPositionProperty);
		}
	}

	InputSettingsCat.AddProperty(TouchRegionJoystickModeProperty);
	InputSettingsCat.AddProperty(InteractionSizeProperty);

	InputScaleProperty->SetPropertyDisplayName(INVTEXT("Input Sensibility"));
	InputSettingsCat.AddProperty(InputScaleProperty);
	
	if (GetDefault<UTouchInterfaceSettings>()->bUseEnhancedInput)
	{
		JoystickActionProperty->SetPropertyDisplayName(INVTEXT("Touch Region Action"));
		InputSettingsCat.AddProperty(JoystickActionProperty);
	}
	else
	{
		InputSettingsCat.AddProperty(HorizontalInputProperty);
		InputSettingsCat.AddProperty(VerticalInputProperty);
	}

	InputSettingsCat.AddProperty(VirtualControlEventProperty);

	// Set Delegate for Control value
	ArrayElement->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FTouchDesignerEditor_DetailsTab::OnControlPropertyChanged));
}

void FTouchDesignerEditor_DetailsTab::ForceRefresh() const
{
	DetailLayoutBuilder->ForceRefreshDetails();
}

void FTouchDesignerEditor_DetailsTab::OnOpacityPropertyChanged() const
{
	TouchDesignerEditorPtr.Pin()->NotifyOpacityChange();
}

void FTouchDesignerEditor_DetailsTab::OnControlPropertyChanged() const
{
	TouchDesignerEditorPtr.Pin()->NotifyControlChange();
}

#undef LOCTEXT_NAMESPACE
