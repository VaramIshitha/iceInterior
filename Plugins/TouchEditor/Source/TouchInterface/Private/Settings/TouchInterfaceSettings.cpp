// Copyright Lost in Game Studio. All Rights Reserved.

#include "Settings/TouchInterfaceSettings.h"

#include "CustomTouchInterfaceScaling.h"
#include "Engine/UserInterfaceSettings.h"
#include "Runtime/Launch/Resources/Version.h"

DEFINE_LOG_CATEGORY_STATIC(LogTouchInterfaceSettings, All, All);

UTouchInterfaceSettings::UTouchInterfaceSettings()
{
	CategoryName = "Plugins";
	SectionName = "Touch Interface";
	
	//General
	DefaultVirtualControlSetup="/TouchInterfaceDesigner/Defaults/VCS_Default.VCS_Default";
	bShowInDesktopPlatform = false;
	
	//Scaling
	ScalingMode = EScalingMode::DesignSize;
	ScaleMultiplier = 1.0f;
	DesignWidth = 1920.0f;
	
	//Recognizer
	bEnableGestureRecognizer = false;
	
	//Shape Recognizer
	bEnableShapeRecognizer = false;
	ShapeDotDistance = 10.0f;
	CornerDetectionThreshold = 0.6f;
	MaxPoint = 30;
	bUseTimer = true;
	DelayBetweenEndDrawAndComputation = 1.5f;
	MinMatchingScoreToTriggerEvent = 60.0f;
	bDrawUserShape = true;
	PointBrush = "/TouchInterfaceDesigner/Defaults/DefaultPointBrush.DefaultPointBrush";
	DrawLineColor = FLinearColor::Gray;
	DrawBrushColor = FLinearColor::White;
	bDrawLines = true;
	bDrawPoint = true;
	DrawLineSize = 5.0f;
	DrawBrushSize = 8.0f;
	DrawerZOrder = 1;
	
	//Inputs
	AutoMoveThreshold = 0.95f;
	AutoMoveDirectionThreshold = 0.98f;
	AutoMoveHoldDuration = 0.8f;
	DragToSprintTrigger = 400.0f;
	DragToSprintThreshold = 40.0f;

#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0
	bUseEnhancedInput = true;
#else
	bUseEnhancedInput = false;
#endif
	
	//Save Feature
	SaveSlotName = "VirtualControlConfiguration";

	//Debug
	bShowInDesktopPlatform = false;
	bDrawDebug = false;
	DebugOpacity = 0.8f;
}

#if WITH_EDITOR
void UTouchInterfaceSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UTouchInterfaceSettings, ScalingMode))
	{
		if (ScalingMode == EScalingMode::Custom)
		{
			
#if ENGINE_MAJOR_VERSION > 4
			const TObjectPtr<UClass> CustomScalingClass = CustomTouchInterfaceScalingClass.TryLoadClass<UCustomTouchInterfaceScaling>();
#else
			const UClass* CustomScalingClass = CustomTouchInterfaceScalingClass.TryLoadClass<UCustomTouchInterfaceScaling>();
#endif
			
			if (CustomScalingClass == nullptr)
			{
				UE_LOG(LogTouchInterfaceSettings, Error, TEXT("Custom Touch Interface Scaling not found!"));
				CustomTouchInterfaceScalingInstance = nullptr;
			}
			else
			{
				if (CustomTouchInterfaceScalingInstance == nullptr)
				{					
					CustomTouchInterfaceScalingInstance = CustomScalingClass->GetDefaultObject<UCustomTouchInterfaceScaling>();
				}
			}
		}
		else
		{
			CustomTouchInterfaceScalingInstance = nullptr;
		}
	}
}
#endif

float UTouchInterfaceSettings::GetScaleFactor(const FVector2D Size, const float LayoutScale, const bool InEditorMode) const
{
	float Scale = 1.0f;
	switch (ScalingMode)
	{
	case EScalingMode::NONE:
		// In editor No Scaling but in runtime Unreal Apply DPIScale on all geometry so we undoing this
		Scale = InEditorMode ? 1.0f : 1.0f / LayoutScale;
		break;

	case EScalingMode::DesignSize:
		{
			const float UndoDPIScaling = InEditorMode ? 1.0f : 1.0f / LayoutScale;
			Scale = (Size.GetMax() / DesignWidth) * UndoDPIScaling;
		}
		break;

	case EScalingMode::DPI:
		Scale = InEditorMode ? GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(Size.X, Size.Y)) : 1.0f;
		break;

	case EScalingMode::Custom:
		if (CustomTouchInterfaceScalingInstance != nullptr)
		{
			const float UndoDPIScaling = InEditorMode ? 1.0f : 1.0f / LayoutScale;
			Scale = CustomTouchInterfaceScalingInstance->GetScaleFactor(Size) * UndoDPIScaling;
			break;
		}
	}
	
	return Scale * ScaleMultiplier;
}

bool UTouchInterfaceSettings::GetResolutionSizeFromDpiCurveScale(const float Scale, float& ResolutionSize) const
{
	const FRichCurve* Curve = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->UIScaleCurve.GetRichCurveConst();
	for (auto Itr = Curve->GetKeyIterator(); Itr; ++Itr)
	{
		if (Itr->Value == Scale)
		{
			ResolutionSize = Itr->Time;
			return true;
		}
	}

	return false;
}
