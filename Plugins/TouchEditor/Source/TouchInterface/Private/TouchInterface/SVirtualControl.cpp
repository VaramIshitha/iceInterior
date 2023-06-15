// Copyright Lost in Game Studio. All Rights Reserved.

#include "TouchInterface/SVirtualControl.h"
#include "SlateOptMacros.h"
#include "VirtualControlEvent.h"
#include "Runtime/Launch/Resources/Version.h" //Needed for android compilation
#include "EnhancedInputSubsystems.h"
#include "TouchInterfaceStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

DEFINE_LOG_CATEGORY_STATIC(LogVirtualControl, All, All);

#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION >= 1
FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
FInputDeviceId PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(FSlateApplicationBase::SlateAppPrimaryPlatformUser);
#endif

void SVirtualControl::Construct(const FArguments& InArgs)
{
	GetEnhancedInputSubsystem();
}

void SVirtualControl::InitDefaultValues()
{
	bIsPressed = false;
	bRefreshPosition = true;
	bUseLandscapePosition = true;
	CapturePointerIndex = -1;
	bMustBeReset = false;
	DebugOpacity = STouchInterface::GetDebugOpacity();
	SetCanTick(false);
	
	if (VirtualControl.VirtualControlEvent)
	{
		VirtualControlEventInstance = NewObject<UVirtualControlEvent>(GetEnhancedInputSubsystem(), VirtualControl.VirtualControlEvent);
		VirtualControlEventInstance->AddToRoot();
		VirtualControlEventInstance->SetButtonName(VirtualControl.ControlName);
		VirtualControlEventInstance->SetPlayerIndex(ParentWidget->GetControllerId());
		VirtualControlEventInstance->SetVirtualControlWidget(SharedThis(this));
	}
}

SVirtualControl::~SVirtualControl()
{
	if (VirtualControlEventInstance)
	{
		UE_LOG(LogVirtualControl, Log, TEXT("Remove VCE from Root"))
		VirtualControlEventInstance->RemoveFromRoot();
	}
}

bool SVirtualControl::OnPress(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	bIsPressed = true;
	CapturePointerIndex = Event.GetPointerIndex();

	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());

	//Todo: When recenter, the virtual control must not exceed the screen. With joystick, keep thumb offset in mind
	if (VirtualControl.bRecenterOnTouch) Recenter(LocalCoord);
	
	if (VirtualControlEventInstance)
	{
		VirtualControlEventInstance->OnTouchBegin(MyGeometry, Event, CurrentTime);
	}
	
	return VirtualControl.bBlockTouchRegion;
}

void SVirtualControl::OnMove(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	if (VirtualControlEventInstance)
	{
		VirtualControlEventInstance->OnTouchMove(MyGeometry, Event, CurrentTime, ElapsedTime);
	}
}

void SVirtualControl::OnRelease(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	bIsPressed = false;
	CapturePointerIndex = -1;
	LastElapsedTime = ElapsedTime;
	ElapsedTime = 0.0f;

	if (VirtualControlEventInstance)
	{
		VirtualControlEventInstance->OnTouchEnd(MyGeometry, Event, CurrentTime, LastElapsedTime);
	}
}

void SVirtualControl::OnTick(const FGeometry& MyGeometry, const float InScaleFactor, const double InCurrentTime, const float InDeltaTime, const bool InForceUpdate, const bool OrientToLandscape)
{
	if (VirtualControl.bIsChild)
	{
		// If this is a child, do nothing
		return;
	}
	
	CurrentTime = InCurrentTime;
	DeltaTime = InDeltaTime;

	if (bIsPressed)
	{
		ElapsedTime += InDeltaTime;
	}

	if (InForceUpdate)
	{
		bRefreshPosition = true;
	}

	CurrentScaleFactor = InScaleFactor;
	bUseLandscapePosition = OrientToLandscape;
	
	if (bRefreshPosition)
	{		
		FVector2D Center;

		if (bAutoPositioning)
		{
			//Todo: Generate Position from current orientation
		}
		else
		{
			Center = bUseLandscapePosition ? VirtualControl.LandscapeCenter : VirtualControl.PortraitCenter;
		}

		CalculateCorrectedValues(Center, FVector2D(0.0f), MyGeometry, InScaleFactor);

		//Todo: Add in control settings if user want to clamp control position into screen
		AlignBoxIntoScreen(CorrectedCenter, CorrectedVisualSize, MyGeometry.GetLocalSize());
	
		SetOffset(CorrectedCenter, CorrectedVisualSize);

		if (VirtualControl.IsParent())
		{
			for (const TSharedPtr<SVirtualControl>& Child : ChildControls)
			{
				// Refresh child position with current parent center
				Child->RefreshChild(MyGeometry, Center, InScaleFactor);
			}
		}
	
		bRefreshPosition = false;
	}

	if (VirtualControl.IsParent())
	{
		// If this control has children, call OnTick on children
		for (const TSharedPtr<SVirtualControl>& Child : ChildControls)
		{
			Child->OnTick(MyGeometry, InScaleFactor, InCurrentTime, InDeltaTime, false, OrientToLandscape);
		}
	}
}

int32 SVirtualControl::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FLinearColor InteractionZoneColor = FLinearColor::Blue;
	InteractionZoneColor.A = DebugOpacity;
	
	if (DrawDebug.Get())
	{
		switch (VirtualControl.InteractionShape)
		{
		case EHitTestType::Square:
			{
				const FSlateBrush* InteractionZoneBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
				const FVector2D InteractionZonePosition = (CorrectedVisualSize * 0.5f - CorrectedInteractionSize * 0.5f) - (CorrectedCenter - AbsoluteCenter);
				FSlateDrawElement::MakeBox(OutDrawElements,LayerId, AllottedGeometry.ToPaintGeometry(CorrectedInteractionSize, FSlateLayoutTransform(InteractionZonePosition)), InteractionZoneBrush, ESlateDrawEffect::None, InteractionZoneColor);
			}
			break;
		case EHitTestType::Circle:
			{
				const FSlateBrush* InteractionZoneBrush = FTouchInterfaceStyle::Get().GetBrush("WhiteCircle");
				const FVector2D InteractionZonePosition = (CorrectedVisualSize * 0.5f - CorrectedInteractionRadiusSize) - (CorrectedCenter - AbsoluteCenter);
				FSlateDrawElement::MakeBox(OutDrawElements,LayerId, AllottedGeometry.ToPaintGeometry(FVector2D(CorrectedInteractionRadiusSize * 2.0f), FSlateLayoutTransform(InteractionZonePosition)), InteractionZoneBrush, ESlateDrawEffect::None, InteractionZoneColor);
			}
			break;
		}		
		

		TSharedPtr<const FCompositeFont> Font;
		FSlateDrawElement::MakeText(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(CorrectedVisualSize, FSlateLayoutTransform(1.0f, FVector2D(0.0f, CorrectedVisualSize.Y + 5.0f))), VirtualControl.ControlName.ToString(), FCoreStyle::Get().GetFontStyle("NormalFont"));
	}

	return LayerId;
}

FVector2D SVirtualControl::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return CorrectedVisualSize;
}

void SVirtualControl::DrawLayer(const FVisualLayer& InLayer, const FVector2D InSize, const FVector2D InBrushSize, const FVector2D InOffset, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{	
	const FSlateBrush* Brush = &InLayer.Brush;
	FLinearColor BackgroundTint = Brush->GetTint(InWidgetStyle);
	BackgroundTint.A *= CurrentOpacity.Get();	
	
	const FVector2D BrushSize = InLayer.bUseBrushSize ? Brush->ImageSize * CurrentScaleFactor : InBrushSize;
	const FVector2D BrushOffset = InOffset + (InSize * 0.5 - BrushSize * 0.5 + InLayer.Offset);
				
	FSlateDrawElement::MakeBox
	(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry
		(
			BrushSize, FSlateLayoutTransform(BrushOffset)
		),
		Brush,
		ESlateDrawEffect::None,
		BackgroundTint
	);
}

bool SVirtualControl::IsInside(const FVector2D FingerPosition)
{
	if (CapturePointerIndex == -1)
	{
		if (GetVisibility().IsHitTestVisible())
		{
			switch (VirtualControl.InteractionShape)
			{
			case EHitTestType::Square:
				return
				FingerPosition.X >= AbsoluteCenter.X - CorrectedInteractionSize.X * 0.5f &&
				FingerPosition.X <= AbsoluteCenter.X + CorrectedInteractionSize.X * 0.5f &&
				FingerPosition.Y >= AbsoluteCenter.Y - CorrectedInteractionSize.Y * 0.5f &&
				FingerPosition.Y <= AbsoluteCenter.Y + CorrectedInteractionSize.Y * 0.5f;
			case EHitTestType::Circle:
				return FMath::Abs(FVector2D::Distance(AbsoluteCenter, FingerPosition)) <= CorrectedInteractionRadiusSize;
			}
		}
	}

	return false;
}

UMaterialInstanceDynamic* SVirtualControl::GetLayerDynamicMaterialInstance(const FName LayerName, UObject* InOuter)
{
	for (FVisualLayer& Layer : VirtualControl.VisualLayers)
	{
		if (Layer.ExposedLayerName.IsEqual(LayerName))
		{
			FSlateBrush& Brush = Layer.Brush;

			//Brush.GetRenderingResource().GetResourceProxy()->Resource->GetType() == ESlateShaderResource::Material

			UObject* Resource = Brush.GetResourceObject();
	
			// If we already have a dynamic material, return it.
			if (UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Resource))
			{
				return DynamicMaterial;
			}
			// If the resource has a material interface we'll just update the brush to have a dynamic material.
			else if (UMaterialInterface* Material = Cast<UMaterialInterface>(Resource))
			{
				DynamicMaterial = UMaterialInstanceDynamic::Create(Material, InOuter, Layer.ExposedLayerName);
				Brush.SetResourceObject(DynamicMaterial);
				
				return DynamicMaterial;
			}
		}
	}
	//layer not found or resource does not contain material so return null
	return nullptr;
}

bool SVirtualControl::ApplyModification()
{
	return false;
}

UEnhancedInputLocalPlayerSubsystem* SVirtualControl::GetEnhancedInputSubsystem()
{
	if (!InputSubsystem)
	{
		InputSubsystem = ParentWidget->GetEnhancedInputSubsystem();
	}
	
	return InputSubsystem;
}

void SVirtualControl::HandleOnOrientationChanged(const bool bOrientToLandscape)
{
	//Todo: Refresh visual and position, flush input
	
	bUseLandscapePosition = bOrientToLandscape;
	bRefreshPosition = true;
}

void SVirtualControl::Reset()
{
	bMustBeReset = false;
	CorrectedCenter = AbsoluteCenter;
	SetOffset(CorrectedCenter, CorrectedVisualSize);

	if (VirtualControl.IsParent())
	{
		for (const TSharedPtr<SVirtualControl>& Child : ChildControls)
		{
			//Call reset child
			Child->ResetChild(AbsoluteCenter);
		}
	}
}

void SVirtualControl::ResetChild(const FVector2D ParentPosition)
{
	bMustBeReset = false;
	CorrectedCenter = AbsoluteCenter = ParentPosition + CorrectedOffset;
	SetOffset(CorrectedCenter, CorrectedVisualSize);
}

void SVirtualControl::FlushPressedKey()
{
	if (bIsPressed)
	{
		//Release all input
	}
}

bool SVirtualControl::AddChild(TSharedPtr<SVirtualControl> Other)
{
	if (Other)
	{
		ChildControls.Add(Other);
		return true;
	}
	return false;
}

bool SVirtualControl::RemoveChild(TSharedPtr<SVirtualControl> ChildToRemove)
{
	if (ChildControls.Contains(ChildToRemove))
	{
		ChildControls.Remove(ChildToRemove);
		return true;
	}
	
	return false;
}

const UTouchInterfaceSettings* SVirtualControl::GetSettings()
{
	if (!Settings)
	{
		Settings = GetDefault<UTouchInterfaceSettings>();
	}
	
	return Settings;
}

void SVirtualControl::CalculateCorrectedValues(const FVector2D& Center, const FVector2D Offset, const FGeometry& AllottedGeometry, const float InScaleFactor)
{
	if (VirtualControl.bIsChild)
	{
		//Scaled by ScaleFactor
		CorrectedOffset = Offset * InScaleFactor;
	}
	else
	{
		CorrectedOffset = Offset;
	}
	
	AbsoluteCenter = Center;
	ResolveRelativePosition(AbsoluteCenter, AllottedGeometry.GetLocalSize());
	AbsoluteCenter += CorrectedOffset;
	CorrectedCenter = AbsoluteCenter;
	CorrectedVisualSize = VirtualControl.VisualSize * InScaleFactor;

	switch (VirtualControl.InteractionShape)
	{
	case EHitTestType::Square:
		CorrectedInteractionSize = VirtualControl.InteractionSize * InScaleFactor;
		break;
	case EHitTestType::Circle:
		CorrectedInteractionRadiusSize = VirtualControl.InteractionRadiusSize * InScaleFactor;
		CircleHitMaxLenght = FMath::Sqrt(CorrectedInteractionRadiusSize);
		break;
	}
}

void SVirtualControl::Recenter(const FVector2D DesiredPosition)
{
	bMustBeReset = false;
	CorrectedCenter = DesiredPosition;
	SetOffset(CorrectedCenter, CorrectedVisualSize);
	
	if (VirtualControl.IsParent())
	{
		for (const TSharedPtr<SVirtualControl>& Child : ChildControls)
		{
			//Recenter child
			Child->RecenterChild(DesiredPosition);
		}
	}
}

void SVirtualControl::RefreshChild(const FGeometry& MyGeometry, const FVector2D ParentCenter, const float InScaleFactor)
{
	CalculateCorrectedValues(ParentCenter, VirtualControl.ParentOffset, MyGeometry, InScaleFactor);

	//Todo: Add in control settings if user want to clamp control position into screen
	AlignBoxIntoScreen(CorrectedCenter, CorrectedVisualSize, MyGeometry.GetLocalSize());

	SetOffset(CorrectedCenter, CorrectedVisualSize);	
}

void SVirtualControl::RecenterChild(const FVector2D ParentPosition)
{
	if (VirtualControl.bMoveWhenParentRecenter)
	{
		AbsoluteCenter = CorrectedCenter = ParentPosition + CorrectedOffset;
		SetOffset(AbsoluteCenter, CorrectedVisualSize);
	}
}

void SVirtualControl::SetLocalPosition(const FVector2D NewPosition, const bool IsNormalized)
{
	FVector2D Center = NewPosition;
	if (IsNormalized)
	{
		ResolveRelativePosition(Center, ParentWidget->GetTickSpaceGeometry().GetLocalSize());
	}

	CorrectedCenter = AbsoluteCenter = NewPosition;

	SetOffset(CorrectedCenter, CorrectedVisualSize);
}

void SVirtualControl::ApplyActionInput(const FKey& InputKey, const bool bIsOnPressed)
{
	if (!InputKey.IsValid()) return;
	
	if (bIsOnPressed)
	{
#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION >= 1
		FSlateApplication::Get().OnControllerButtonPressed(InputKey.GetFName(), FPlatformMisc::GetPlatformUserForUserIndex(ParentWidget->GetControllerId()), PrimaryInputDevice, false);
#else
		FSlateApplication::Get().OnControllerButtonPressed(InputKey.GetFName(), ParentWidget->GetControllerId(), false);
#endif
	}
	else
	{
#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION >= 1
		FSlateApplication::Get().OnControllerButtonReleased(InputKey.GetFName(), FPlatformMisc::GetPlatformUserForUserIndex(ParentWidget->GetControllerId()), PrimaryInputDevice, false);
#else
		FSlateApplication::Get().OnControllerButtonReleased(InputKey.GetFName(), ParentWidget->GetControllerId(), false);
#endif
	}
}

void SVirtualControl::ApplyAxisInput(const FKey& InputKey, const float Value)
{
	if (!InputKey.IsValid()) return;
	
#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION >= 1
	FSlateApplication::Get().OnControllerAnalog(InputKey.GetFName(), FPlatformMisc::GetPlatformUserForUserIndex(ParentWidget->GetControllerId()), PrimaryInputDevice, Value);
#else
	FSlateApplication::Get().OnControllerAnalog(InputKey.GetFName(), ParentWidget->GetControllerId(), Value);
#endif
	
}

void SVirtualControl::SetOffset(const FVector2D NewPosition, const FVector2D NewVisualSize) const
{	
	if (!CanvasSlot) return;

#if ENGINE_MAJOR_VERSION > 4
	CanvasSlot->SetOffset(FMargin(NewPosition.X, NewPosition.Y, NewVisualSize.X, NewVisualSize.Y));
#else
	CanvasSlot->Offset(FMargin(NewPosition.X, NewPosition.Y, NewVisualSize.X, NewVisualSize.Y));
#endif
}

void SVirtualControl::AlignBoxIntoScreen(FVector2D& Position, const FVector2D& Size, const FVector2D& ScreenSize)
{
	//If control size > screen size, do nothing
	if (Size.X > ScreenSize.X || Size.Y > ScreenSize.Y)
	{
		UE_LOG(LogVirtualControl, Verbose, TEXT("Do nothing"));
		return;
	}

	//Clamp X to min
	if (Position.X - Size.X * 0.5f < 0.f)
	{
		UE_LOG(LogVirtualControl, Verbose, TEXT("Clamp X to min"));
		Position.X = Size.X * 0.5f;
	}

	//Clamp X to max
	if (Position.X + Size.X * 0.5f > ScreenSize.X)
	{
		UE_LOG(LogVirtualControl, Verbose, TEXT("Clamp X to max"));
		Position.X = ScreenSize.X - Size.X * 0.5f;
	}

	//Clamp Y to min
	if (Position.Y - Size.Y * 0.5f < 0.f)
	{
		UE_LOG(LogVirtualControl, Verbose, TEXT("Clamp Y to min"));
		Position.Y = Size.Y * 0.5f;
	}

	//Clamp Y to max
	if (Position.Y + Size.Y * 0.5f > ScreenSize.Y)
	{
		UE_LOG(LogVirtualControl, Verbose, TEXT("Clamp Y to max"));
		Position.Y = ScreenSize.Y - Size.Y * 0.5f;
	}
}

void SVirtualControl::ResolveRelativePosition(FVector2D& Position, const FVector2D RelativeTo)
{
	Position.X = Position.X * RelativeTo.X;
	Position.Y = Position.Y * RelativeTo.Y;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
