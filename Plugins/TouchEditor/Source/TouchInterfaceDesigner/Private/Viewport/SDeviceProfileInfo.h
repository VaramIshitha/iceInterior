// Copyright Lost in Game Studio. All Right Reserved


#pragma once


class TOUCHINTERFACEDESIGNER_API SDeviceProfileInfo : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SDeviceProfileInfo){}
		SLATE_ARGUMENT(FString, DefaultProfile)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateDeviceProfile(const FString ProfileName);

private:
	FText GetDesignerDeviceProfileName() const;
	FText GetDesignerSize() const;
	FText GetDeviceProfileName() const;
	FText GetScreenDefinition() const;
	FText GetMobileDeviceContentScale() const;
	FText GetScreenRenderResolution() const;
	
	FText DesignerDeviceProfileName;
	FText DesignerSize;
	FText DeviceProfileName;
	FText ScreenDefinition;
	FText MobileDeviceContentScaleText;
	FText ScreenRenderResolution;
	FText ScreenFormat;
};
