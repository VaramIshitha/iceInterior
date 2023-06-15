// Copyright Lost in Game Studio. All Right Reserved

#include "SPaletteItemDraggedWidget.h"
#include "SlateOptMacros.h"
#include "VirtualControlSetup.h"
#include "TouchInterfaceDesignerSettings.h"
#include "Viewport/STouchInterfaceDesignerViewport.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPaletteItemDraggedWidget::Construct(const FArguments& InArgs)
{
	ScaleFactor = InArgs._ScaleFactor;
	CurrentType = InArgs._Type;
	
	const UTouchInterfaceDesignerSettings* Settings = GetDefault<UTouchInterfaceDesignerSettings>();
	
	switch (CurrentType)
	{
	case EControlType::Button:
		{
			// Load Default Button Image from Touch Designer Editor Settings
			const FSoftObjectPath DefaultBackgroundObject = Settings->DefaultButtonImage;
			if(DefaultBackgroundObject.IsValid())
			{
				UTexture2D* BackgroundImage = LoadObject<UTexture2D>(nullptr, *DefaultBackgroundObject.ToString());
				FSlateBrush Background;
				Background.SetResourceObject(BackgroundImage);
			
				BackgroundToPaint = Background;
			}		
			DesiredSize = FVector2D(60.0f);
		}
		break;
	case EControlType::Joystick:
		{
			// Load Default Joystick Image
			const FSoftObjectPath DefaultBackgroundJoystickObject = Settings->DefaultBackgroundJoystickImage;
			if (DefaultBackgroundJoystickObject.IsValid())
			{
				UTexture2D* BackgroundImage = LoadObject<UTexture2D>(nullptr, *DefaultBackgroundJoystickObject.ToString());
				FSlateBrush Background;
				Background.SetResourceObject(BackgroundImage);
				BackgroundToPaint = Background;
			}

			// Load Default Thumb Image
			const FSoftObjectPath DefaultThumbObject = Settings->DefaultThumbJoystickImage;
			if (DefaultThumbObject.IsValid())
			{
				UTexture2D* ThumbImage = LoadObject<UTexture2D>(nullptr, *DefaultThumbObject.ToString());
				FSlateBrush Thumb;
				Thumb.SetResourceObject(ThumbImage);
				ThumbToPaint = Thumb;
			}
			DesiredSize = FVector2D(100.0f);
		}
		break;
	case EControlType::TouchRegion:
		{
			//Todo: Make brush for this or get existing brush in editor style
#if ENGINE_MAJOR_VERSION > 4
			const FSlateBrush* Background = FAppStyle::Get().GetBrush("WhiteBrush");
#else
			const FSlateBrush* Background = FEditorStyle::Get().GetBrush("WhiteBrush");
#endif
			
			BackgroundToPaint = *Background;
			DesiredSize = FVector2D(300.0f, 300.0f);
		}		
		break;
	default:
		break;
	}
}

int32 SPaletteItemDraggedWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FLinearColor BackgroundTint = BackgroundToPaint.GetTint(InWidgetStyle);
	BackgroundTint.A *= 0.5f;
		
	FSlateDrawElement::MakeBox
	(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		&BackgroundToPaint,
		ESlateDrawEffect::None,
		BackgroundTint
	);
	
	if (CurrentType == EControlType::Joystick)
	{
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2D(60.0f), FSlateLayoutTransform(ScaleFactor.Get(), (DesiredSize * 0.5f - 60.0f * 0.5) * ScaleFactor.Get())),
			&ThumbToPaint,
			ESlateDrawEffect::None,
			BackgroundTint
		);
	}
	
	return LayerId;
}

FVector2D SPaletteItemDraggedWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return DesiredSize * ScaleFactor.Get();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
