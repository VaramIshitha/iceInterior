// Copyright Lost in Game Studio. All Rights Reserved

#include "STouchInterfaceConverter.h"

#include "TouchInterfaceDesignerSettings.h"
#include "Preview/STouchInterfacePreviewer.h"
#include "GameFramework/TouchInterface.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Viewport/SViewportZoomPan.h"

#define LOCTEXT_NAMESPACE "STouchDesignerEditorConvertWidget"

void STouchInterfaceConverter::Construct(const FArguments& InArgs)
{
	ConvertSettings = FConvertSettings();
	OnConvert = InArgs._OnConvert;
	OnCancel = InArgs._OnCancel;
	SelectedTouchInterface = InArgs._SelectedTouchInterface;

	// Load Default Button Image from Virtual Control Designer Editor Settings
	const FSoftObjectPath DefaultBackgroundObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultButtonImage;
	if(DefaultBackgroundObject.IsValid())
	{
		BackgroundButtonImage = LoadObject<UTexture2D>(nullptr, *DefaultBackgroundObject.ToString());
	}

	// Load Default Joystick Image
	const FSoftObjectPath DefaultBackgroundJoystickObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultBackgroundJoystickImage;
	if (DefaultBackgroundJoystickObject.IsValid())
	{
		BackgroundJoystickImage = LoadObject<UTexture2D>(nullptr, *DefaultBackgroundJoystickObject.ToString());
	}

	// Load Default Thumb Image
	const FSoftObjectPath DefaultThumbObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultThumbJoystickImage;
	if (DefaultThumbObject.IsValid())
	{
		ThumbJoystickImage = LoadObject<UTexture2D>(nullptr, *DefaultThumbObject.ToString());
	}
	
	ChildSlot
	[
		SNew(SSplitter)
		.Orientation(Orient_Horizontal)
#if ENGINE_MAJOR_VERSION > 4
		.Style(FAppStyle::Get(),"SplitterDark")
#else
		.Style(FEditorStyle::Get(),"SplitterDark")
#endif


		
		+SSplitter::Slot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(10)
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PreviewKey","Preview"))
#if ENGINE_MAJOR_VERSION > 4
				.TextStyle(FAppStyle::Get(), "Persona.RetargetManager.ImportantText")
#else
				.TextStyle(FEditorStyle::Get(), "Persona.RetargetManager.ImportantText")
#endif

			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				// Arrange Children Zoom and offset
				SAssignNew(ZoomPan, SViewportZoomPan)
				.Visibility(EVisibility::SelfHitTestInvisible)
				.ZoomAmount(this, &STouchInterfaceConverter::GetPreviewerScale)
				.ViewOffset(this, &STouchInterfaceConverter::GetPreviewerOffset)
				[
					SNew(SBox)
					.Visibility(EVisibility::SelfHitTestInvisible)
					.WidthOverride(this, &STouchInterfaceConverter::GetPreviewerWidth)
					.HeightOverride(this, &STouchInterfaceConverter::GetPreviewerHeight)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(TouchInterfacePreviewer, STouchInterfacePreviewer)
						.VirtualControls(GenerateVirtualControlData(SelectedTouchInterface->Controls))
					]
				]
			]
		]

		+SSplitter::Slot()
		.Value(0.25f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(4)
			[
				CreateConvertSettingsUI()
			]
			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(SSpacer)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Text(LOCTEXT("CancelKey", "Cancel"))
					.OnClicked(this, &STouchInterfaceConverter::HandleOnCancelConversion)
				]
				+SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Text(LOCTEXT("AcceptKey", "Convert"))
					.OnClicked(this, &STouchInterfaceConverter::HandleOnAcceptConversion)
				]
			]
		]
	];

	RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(this, &STouchInterfaceConverter::AutoRefresh));
}

/*int32 STouchDesignerEditor_ConvertWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	GLog->Log("ConvertWidget Layer = " + FString::FromInt(LayerId));
	const FSlateBrush* BackgroundImage = FEditorStyle::GetBrush(TEXT("Graph.Panel.SolidBackground"));
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BackgroundImage);

	return LayerId;
}*/

TSharedRef<SWidget> STouchInterfaceConverter::CreateConvertSettingsUI()
{		
	return SNew(SVerticalBox)

	+SVerticalBox::Slot()
	.Padding(10)
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ConvertOptionKey","Settings"))
#if ENGINE_MAJOR_VERSION > 4
		.TextStyle(FAppStyle::Get(), "Persona.RetargetManager.ImportantText")
#else
		.TextStyle(FEditorStyle::Get(), "Persona.RetargetManager.ImportantText")
#endif
	]

	// Keep Image Setting
	+SVerticalBox::Slot()
	.HAlign(HAlign_Fill)
	.Padding(2)
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("KeepImageKey", "Keep Image"))
			.ToolTipText(LOCTEXT("KeepImageToolTipKey", "Keep already defined image on Touch Interface if any or use default Touch Designer Image"))
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		[
			SNew(SCheckBox)
			//.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
			.IsChecked(true)
			.OnCheckStateChanged(this, &STouchInterfaceConverter::HandleOnKeepImageSettingChanged)
		]
	]
	
	// Is In Landscape Setting
	+SVerticalBox::Slot()
	.HAlign(HAlign_Fill)
	.Padding(2)
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IsInLandscapeKey", "Is In Landscape"))
			.ToolTipText(LOCTEXT("IsInLandscapeToolTipKey", "True if the Touch Interface was created for use in landscape mode"))
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		[
			SNew(SCheckBox)
			//.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
			.IsChecked(true)
			.OnCheckStateChanged(this, &STouchInterfaceConverter::HandleOnIsInLandscapeSettingChanged)
		]
	]

	+SVerticalBox::Slot()
	.HAlign(HAlign_Fill)
	.Padding(2)
	.AutoHeight()
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SizeReferenceKey", "Touch Interface Resolution"))
			.ToolTipText(LOCTEXT("SizeReferenceToolTipKey", "The resolution in which the Touch Interface was created"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(4)
			[
				SNew(SNumericEntryBox<int32>)
				.Value(this, &STouchInterfaceConverter::GetSizeX)
				.OnValueCommitted(this, &STouchInterfaceConverter::HandleOnWidthSettingChanged)
				.AllowSpin(false)
#if ENGINE_MAJOR_VERSION > 4
				.Font(FAppStyle::Get().GetFontStyle("LayersView.LayerNameFont"))
#else
				.Font(FEditorStyle::Get().GetFontStyle("LayersView.LayerNameFont"))
#endif

				.MinValue(1280)
				.MaxValue(2560)
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(4)
			[
				SNew(SNumericEntryBox<int32>)
				.OverrideTextMargin(FMargin(2))
				.Value(this, &STouchInterfaceConverter::GetSizeY)
				.OnValueCommitted(this, &STouchInterfaceConverter::HandleOnHeightSettingChanged)
				.AllowSpin(false)
#if ENGINE_MAJOR_VERSION > 4
				.Font(FAppStyle::Get().GetFontStyle("LayersView.LayerNameFont"))
#else
				.Font(FEditorStyle::Get().GetFontStyle("LayersView.LayerNameFont"))
#endif

				.MinValue(720)
				.MaxValue(1440)
			]
		]
	]
	
	+SVerticalBox::Slot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	.Padding(0,5)
	[
		SNew(SSeparator)
		.Orientation(Orient_Horizontal)
		.Thickness(4)
		.ColorAndOpacity(FSlateColor(FLinearColor::Black))
	]
	
	// Delete After Conversion Setting
	+SVerticalBox::Slot()
	.HAlign(HAlign_Fill)
	.Padding(2)
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DeleteAfterKey", "Delete After Conversion"))
			.ToolTipText(LOCTEXT("DeleteAfterToolTipKey", "True if you want to delete Touch Interface from content browser after conversion"))
		]
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		[
			SNew(SCheckBox)
			//.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
			.IsChecked(false)
			.OnCheckStateChanged(this, &STouchInterfaceConverter::HandleOnDeleteAfterConversionSettingChanged)
		]
	];
}

TArray<FVirtualControl> STouchInterfaceConverter::GenerateVirtualControlData(TArray<FTouchInputControl> EpicTouchInterfaceControl)
{
	TArray<FVirtualControl> PreviewData;
	for (FTouchInputControl& TouchInterfaceControl : EpicTouchInterfaceControl)
	{
		FVirtualControl VirtualControlData = FVirtualControl();
		VirtualControlData.LandscapeCenter = ResolvePosition(TouchInterfaceControl.Center);
		VirtualControlData.VisualSize = ResolveVisual(TouchInterfaceControl.VisualSize);
		VirtualControlData.ThumbSize = ResolveVisual(TouchInterfaceControl.ThumbSize);

		TArray<FVisualLayer> Layers;
		
		if (ConvertSettings.bKeepImage)
		{
			//Check if control has thumb image
			if (TouchInterfaceControl.Image1)
			{
#if ENGINE_MAJOR_VERSION > 4
				TObjectPtr<UTexture2D> BackgroundTexture = TouchInterfaceControl.Image2 ? TouchInterfaceControl.Image2 : BackgroundJoystickImage;
#else
				UTexture2D* BackgroundTexture = TouchInterfaceControl.Image2 ? TouchInterfaceControl.Image2 : BackgroundJoystickImage;
#endif
				
				Layers.Add(FVisualLayer(BackgroundTexture, 13));

#if ENGINE_MAJOR_VERSION > 4
				TObjectPtr<UTexture2D> ThumbTexture = TouchInterfaceControl.Image1;
#else
				UTexture2D* ThumbTexture = TouchInterfaceControl.Image1;
#endif
				
				Layers.Add(FVisualLayer(ThumbTexture, 14));
			}
			else
			{
#if ENGINE_MAJOR_VERSION > 4
				TObjectPtr<UTexture2D> BackgroundTexture = TouchInterfaceControl.Image2 ? TouchInterfaceControl.Image2 : BackgroundButtonImage;
#else
				UTexture2D* BackgroundTexture = TouchInterfaceControl.Image2 ? TouchInterfaceControl.Image2 : BackgroundButtonImage;
#endif
				Layers.Add(FVisualLayer(BackgroundTexture, 13));
			}
		}
		else
		{
			//Check if control has thumb image
			if (TouchInterfaceControl.Image1)
			{
				Layers.Add(FVisualLayer(BackgroundJoystickImage, 13));
				Layers.Add(FVisualLayer(ThumbJoystickImage, 14));
			}
			else
			{
				Layers.Add(FVisualLayer(BackgroundButtonImage, 13));
			}
		}

		if (TouchInterfaceControl.Image1)
		{
			VirtualControlData.Type = EControlType::Joystick;
		}
		else
		{
			VirtualControlData.Type =EControlType::Button;
		}
		
		VirtualControlData.VisualLayers = Layers;
		
		PreviewData.Add(VirtualControlData);
	}

	return PreviewData;
}

FVector2D STouchInterfaceConverter::ResolvePosition(const FVector2D Value)
{
	FVector2D ResolvedValue;
	const FVector2D TouchInterfacePreviewerSize = FVector2D(ConvertSettings.SizeReference);
	
	ResolvedValue.X = Value.X > 1.0f ? Value.X / TouchInterfacePreviewerSize.X : Value.X;
	ResolvedValue.Y = Value.Y > 1.0f ? Value.Y / TouchInterfacePreviewerSize.Y : Value.Y;
	
	return ResolvedValue;
}

FVector2D STouchInterfaceConverter::ResolveVisual(const FVector2D Value)
{
	FVector2D ResolvedValue;
	const FVector2D TouchInterfacePreviewerSize = FVector2D(ConvertSettings.SizeReference);

	ResolvedValue.X = Value.X > 1.0f ? Value.X : Value.X * TouchInterfacePreviewerSize.X;
	ResolvedValue.Y = Value.Y > 1.0f ? Value.Y : Value.Y * TouchInterfacePreviewerSize.Y;

	return ResolvedValue;
}

float STouchInterfaceConverter::GetPreviewerScale() const
{
	const FVector2D GeometrySize = ZoomPan->GetTickSpaceGeometry().GetLocalSize();
	FVector2D Scale;
	Scale.X = GeometrySize.X / ConvertSettings.SizeReference.X;
	Scale.Y = GeometrySize.Y / ConvertSettings.SizeReference.Y;

	return Scale.GetMin();
}

FVector2D STouchInterfaceConverter::GetPreviewerOffset() const
{
	const FVector2D ViewportSize = ZoomPan->GetTickSpaceGeometry().GetLocalSize();
	const FVector2D ScaledDesignerSize = FVector2D(ConvertSettings.SizeReference) * GetPreviewerScale();
	
	const FVector2D StartPanningOffset = (ViewportSize - ScaledDesignerSize) / 2;
	
	return -StartPanningOffset;
}

FOptionalSize STouchInterfaceConverter::GetPreviewerWidth() const
{
	return ConvertSettings.SizeReference.X;
}

FOptionalSize STouchInterfaceConverter::GetPreviewerHeight() const
{
	return ConvertSettings.SizeReference.Y;
}

EActiveTimerReturnType STouchInterfaceConverter::AutoRefresh(double InCurrentTime, float InDeltaTime)
{
	const FVector2D GeometrySize = ZoomPan->GetTickSpaceGeometry().GetLocalSize();

	if (GeometrySize == FVector2D::ZeroVector)
	{
		UE_LOG(LogTemp, Display, TEXT("Wait for geometry"));
		return EActiveTimerReturnType::Continue;
	}

	UpdatePreviewData();
	return EActiveTimerReturnType::Stop;
}

TOptional<int32> STouchInterfaceConverter::GetSizeX() const
{
	return ConvertSettings.SizeReference.X;
}

TOptional<int32> STouchInterfaceConverter::GetSizeY() const
{
	return ConvertSettings.SizeReference.Y;
}

void STouchInterfaceConverter::HandleOnKeepImageSettingChanged(ECheckBoxState NewState)
{
	ConvertSettings.bKeepImage = NewState == ECheckBoxState::Checked ? true : false;
	UpdatePreviewData();
}

void STouchInterfaceConverter::HandleOnIsInLandscapeSettingChanged(ECheckBoxState NewState)
{
	ConvertSettings.bIsInLandscape = NewState == ECheckBoxState::Checked ? true : false;
	UpdatePreviewData();
}

void STouchInterfaceConverter::HandleOnDeleteAfterConversionSettingChanged(ECheckBoxState NewState)
{
	ConvertSettings.bDeleteAfterConversion = NewState == ECheckBoxState::Checked ? true : false;
}

void STouchInterfaceConverter::HandleOnWidthSettingChanged(int32 NewValue, ETextCommit::Type CommitType)
{
	FIntPoint Size = ConvertSettings.SizeReference;
	Size.X = FMath::Clamp(NewValue, 1280, 2560);
	ConvertSettings.SizeReference = Size;
	UpdatePreviewData();
}

void STouchInterfaceConverter::HandleOnHeightSettingChanged(int32 NewValue, ETextCommit::Type CommitType)
{
	FIntPoint Size = ConvertSettings.SizeReference;
	Size.Y = FMath::Clamp(NewValue, 720, 1440);
	ConvertSettings.SizeReference = Size;
	UpdatePreviewData();
}

void STouchInterfaceConverter::UpdatePreviewData()
{
	TouchInterfacePreviewer->UpdateData(GenerateVirtualControlData(SelectedTouchInterface->Controls), ConvertSettings.SizeReference);
}

FReply STouchInterfaceConverter::HandleOnAcceptConversion()
{
	if (OnConvert.IsBound()) OnConvert.Execute(ConvertSettings);
	return FReply::Handled();
}

FReply STouchInterfaceConverter::HandleOnCancelConversion()
{
	if (OnCancel.IsBound()) OnCancel.Execute();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE