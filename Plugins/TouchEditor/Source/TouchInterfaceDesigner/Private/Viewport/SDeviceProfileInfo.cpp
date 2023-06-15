#include "SDeviceProfileInfo.h"

#include "DeviceProfiles/DeviceProfile.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Editor/TouchInterfaceDesignerStyle.h"
#include "Settings/TouchInterfaceDesignerDeviceProfile.h"


void SDeviceProfileInfo::Construct(const FArguments& InArgs)
{
#if ENGINE_MAJOR_VERSION > 4
	const FSlateFontInfo SlateFontInfo = FAppStyle::Get().GetFontStyle("Normal");
#else
	const FSlateFontInfo SlateFontInfo = FEditorStyle::Get().GetFontStyle("Normal");
#endif
	

	ChildSlot
	[
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.Padding(FMargin(10.0f))
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(FTouchInterfaceDesignerStyle::Get().GetBrush("RoundedWhiteBox"))
			.ForegroundColor(FSlateColor(FLinearColor::White))
			.BorderBackgroundColor(FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.4f)))
			[
				SNew(SVerticalBox)

				// Designer Device Profile
				+SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Margin(FMargin(2.0f))
					.Font(SlateFontInfo)
					.Text(SharedThis(this), &SDeviceProfileInfo::GetDesignerDeviceProfileName)
				]

				// Designer Size
				+SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Margin(FMargin(2.0f))
					.Font(SlateFontInfo)
					.Text(SharedThis(this), &SDeviceProfileInfo::GetDesignerSize)
				]

				// Device Profile
				+SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Margin(FMargin(2.0f))
					.Font(SlateFontInfo)
					.Text(SharedThis(this), &SDeviceProfileInfo::GetDeviceProfileName)
				]

				// Device Screen Definition and Format
				+SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Margin(FMargin(2.0f))
					.Font(SlateFontInfo)
					.Text(SharedThis(this), &SDeviceProfileInfo::GetScreenDefinition)
				]

				// Device content scale
				+SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Margin(FMargin(2.0f))
					.Font(SlateFontInfo)
					.Text(SharedThis(this), &SDeviceProfileInfo::GetMobileDeviceContentScale)
				]

				// Device Screen render resolution
				+SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Margin(FMargin(2.0f))
					.Font(SlateFontInfo)
					.Text(SharedThis(this), &SDeviceProfileInfo::GetScreenRenderResolution)
				]
			]
		]
	];

	UpdateDeviceProfile(InArgs._DefaultProfile);
}

void SDeviceProfileInfo::UpdateDeviceProfile(const FString ProfileName)
{
	FDesignerDeviceProfile DesignerDeviceProfile;
	if (FTouchInterfaceDesignerDeviceProfile::GetProfile(ProfileName, DesignerDeviceProfile))
	{
		DesignerDeviceProfileName = FText::FromString(ProfileName);

		const int32 NewDesignerSizeX = FMath::Floor(1080.0f * (DesignerDeviceProfile.ScreenDefinition.X / DesignerDeviceProfile.ScreenDefinition.Y));
	
		FFormatNamedArguments Args;
		Args.Add(TEXT("X"), NewDesignerSizeX);
		Args.Add(TEXT("Y"), 1080);
	
		DesignerSize = FText::Format(INVTEXT("Designer Size : {X} x {Y}"), Args);
	
		if (DesignerDeviceProfile.bIsGeneric)
		{
			DeviceProfileName = FText::FromString("Device Profile : No Profile");
			MobileDeviceContentScaleText = FText::FromString("Device Content Scale : 1.0");

			const int32 ScreenRenderX = FMath::RoundToInt(DesignerDeviceProfile.ScreenDefinition.X);
			constexpr int32 ScreenRenderY = 1080;
			const FString Resolution = FString::Printf(TEXT("Render Resolution : %d x %d"), ScreenRenderX, ScreenRenderY);
			ScreenRenderResolution = FText::FromString(Resolution);
		}
		else
		{
			DeviceProfileName = FText::FromString(FString::Printf(TEXT("Device Profile : %s"), *DesignerDeviceProfile.DeviceProfileName));

			const UDeviceProfile* Profile = UDeviceProfileManager::Get().FindProfile(DesignerDeviceProfile.DeviceProfileName, false);
			if (Profile)
			{
				float MobileDeviceContentScale;
				Profile->GetConsolidatedCVarValue(TEXT("r.MobileContentScaleFactor"), MobileDeviceContentScale);
			
				FNumberFormattingOptions NumberFormatOptions;
				NumberFormatOptions.AlwaysSign = false;
				NumberFormatOptions.UseGrouping = false;
				NumberFormatOptions.RoundingMode = HalfToZero;
				NumberFormatOptions.MinimumIntegralDigits = 1;
				NumberFormatOptions.MaximumIntegralDigits = 324;
				NumberFormatOptions.MinimumFractionalDigits = 0;
				NumberFormatOptions.MaximumFractionalDigits = 2;

				FString Number = FText::AsNumber(MobileDeviceContentScale, &NumberFormatOptions).ToString();

				const FString ContentScaleString = FString::Printf(TEXT("Device Content Scale : %s"), *Number);
			
				MobileDeviceContentScaleText = FText::FromString(ContentScaleString);

				if (Profile->DeviceType == "IOS")
				{
					//Multiply Logical resolution by MobileDeviceContentScale
				
					const int32 ScreenRenderX = FMath::RoundToInt(DesignerDeviceProfile.LogicalResolution.X * MobileDeviceContentScale);
					const int32 ScreenRenderY = FMath::RoundToInt(DesignerDeviceProfile.LogicalResolution.Y * MobileDeviceContentScale);
					const FString Resolution = FString::Printf(TEXT("Render Resolution : %d x %d"), ScreenRenderX, ScreenRenderY);
					ScreenRenderResolution = FText::FromString(Resolution);
				}
				else if (Profile->DeviceType == "Android")
				{
					//1 = 720p, 2 = 1440
				
					const int32 ScreenRenderX = FMath::RoundToInt((720.0f * MobileDeviceContentScale) * (DesignerDeviceProfile.ScreenDefinition.X / DesignerDeviceProfile.ScreenDefinition.Y));
					const int32 ScreenRenderY = FMath::RoundToInt(720.0f * MobileDeviceContentScale);
					const FString Resolution = FString::Printf(TEXT("Render Resolution : %d x %d"), ScreenRenderX, ScreenRenderY);
					ScreenRenderResolution = FText::FromString(Resolution);
				}
			}
		}

		const FString Definition = FString::Printf(TEXT("Screen Definition : %d x %d (%s)"), FMath::RoundToInt(DesignerDeviceProfile.ScreenDefinition.X), FMath::RoundToInt(DesignerDeviceProfile.ScreenDefinition.Y), *DesignerDeviceProfile.ScreenFormat);
		ScreenDefinition = FText::FromString(Definition);
	}
	else
	{
		//Fallback to default size
		FFormatNamedArguments Args;
		Args.Add(TEXT("X"), 1920);
		Args.Add(TEXT("Y"), 1080);
	
		DesignerSize = FText::Format(INVTEXT("Designer Size : {X} x {Y}"), Args);
	
		DesignerDeviceProfileName = FText::FromString("Generic 16/9");
		DeviceProfileName = FText::FromString("No Profile");

		MobileDeviceContentScaleText = FText::FromString("No Scale");

		const FString Definition = FString::Printf(TEXT("Screen Definition : %d x %d (16/9)"), 1920, 1080);
		ScreenDefinition = FText::FromString(Definition);
		const FString Resolution = FString::Printf(TEXT("Render Resolution : %d x %d"), 1920, 1080);
		ScreenRenderResolution = FText::FromString(Resolution);
	}
}

FText SDeviceProfileInfo::GetDesignerDeviceProfileName() const
{
	return DesignerDeviceProfileName;
}

FText SDeviceProfileInfo::GetDesignerSize() const
{
	return DesignerSize;
}

FText SDeviceProfileInfo::GetDeviceProfileName() const
{
	return DeviceProfileName;
}

FText SDeviceProfileInfo::GetScreenDefinition() const
{
	return ScreenDefinition;
}

FText SDeviceProfileInfo::GetMobileDeviceContentScale() const
{
	return MobileDeviceContentScaleText;
}

FText SDeviceProfileInfo::GetScreenRenderResolution() const
{
	return ScreenRenderResolution;
}
