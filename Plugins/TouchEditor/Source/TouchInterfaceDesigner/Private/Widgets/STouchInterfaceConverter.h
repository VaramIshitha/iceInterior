// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

#include "Classes/VirtualControlSetup.h"
#include "AssetTypeActions/VirtualControlDesignerEditorMenuExtender.h"
#include "GameFramework/TouchInterface.h"

class UTouchInterface;
class UVirtualControlSetup;
class STouchInterfacePreviewer;

DECLARE_DELEGATE_OneParam(FOnConvertAccepted, FConvertSettings)

class STouchInterfaceConverter : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STouchInterfaceConverter) {}
	
	SLATE_ARGUMENT(UTouchInterface*, SelectedTouchInterface)
	SLATE_EVENT(FOnConvertAccepted, OnConvert)
	SLATE_EVENT(FSimpleDelegate, OnCancel)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	//virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	
private:	
	TSharedRef<SWidget> CreateConvertSettingsUI();
	
	TArray<FVirtualControl> GenerateVirtualControlData(TArray<FTouchInputControl> EpicTouchInterfaceControl);

	FVector2D ResolvePosition(const FVector2D Value);
	FVector2D ResolveVisual(const FVector2D Value);

	float GetPreviewerScale() const;
	FVector2D GetPreviewerOffset() const;

	FOptionalSize GetPreviewerWidth() const;
	FOptionalSize GetPreviewerHeight() const;

	EActiveTimerReturnType AutoRefresh(double InCurrentTime, float InDeltaTime);
	
	TOptional<int32> GetSizeX() const;
	TOptional<int32> GetSizeY() const;

	void HandleOnKeepImageSettingChanged(ECheckBoxState NewState);
	void HandleOnIsInLandscapeSettingChanged(ECheckBoxState NewState);
	void HandleOnDeleteAfterConversionSettingChanged(ECheckBoxState NewState);
	void HandleOnWidthSettingChanged(int32 NewValue, ETextCommit::Type CommitType);
	void HandleOnHeightSettingChanged(int32 NewValue, ETextCommit::Type CommitType);

	void UpdatePreviewData();
	
	FReply HandleOnAcceptConversion();
	FReply HandleOnCancelConversion();
	
	TSharedPtr<STouchInterfacePreviewer> TouchInterfacePreviewer;

	TSharedPtr<class SViewportZoomPan> ZoomPan;

	FConvertSettings ConvertSettings;

	UTouchInterface* SelectedTouchInterface;

#if ENGINE_MAJOR_VERSION > 4
	TObjectPtr<UTexture2D> BackgroundButtonImage;
	TObjectPtr<UTexture2D> BackgroundJoystickImage;
	TObjectPtr<UTexture2D> ThumbJoystickImage;
#else
	UTexture2D* BackgroundButtonImage;
	UTexture2D* BackgroundJoystickImage;
	UTexture2D* ThumbJoystickImage;
#endif

	FOnConvertAccepted OnConvert;
	FSimpleDelegate OnCancel;

	float CurrentScale;
};
