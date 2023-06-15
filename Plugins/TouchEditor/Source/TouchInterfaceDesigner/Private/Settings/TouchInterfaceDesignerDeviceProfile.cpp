// Copyright Lost in Game Studio, Inc. All Rights Reserved.

#include "TouchInterfaceDesignerDeviceProfile.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

#if ENGINE_MAJOR_VERSION > 4
#include "Styling/SlateStyleMacros.h"
#endif

#define RootToContentDir Style->RootToContentDir

#if ENGINE_MAJOR_VERSION < 5
#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#endif

TMap<FString, FDesignerDeviceProfile> FTouchInterfaceDesignerDeviceProfile::DesignerDeviceProfiles;

FTouchInterfaceDesignerDeviceProfile::FTouchInterfaceDesignerDeviceProfile()
{
	
}

void FTouchInterfaceDesignerDeviceProfile::Initialize()
{
	FDesignerDeviceStyle::RegisterStyle();
	//FDesignerDeviceStyle::ReloadTextures();
	
	// COMMON PROFILE
	
	SetGenericProfile(TEXT("Generic"),TEXT("Generic 16/9"),TEXT("Android_Default"),TEXT("Generic.16/9"),FVector2D(1920.0f, 1080.0f),TEXT("16/9"));
	SetGenericProfile(TEXT("Generic"),TEXT("Generic 18/9"),TEXT("Android_Default"),TEXT("Generic.18/9"),FVector2D(2160.0f, 1080.0f),TEXT("18/9"));
	SetGenericProfile(TEXT("Generic"),TEXT("Generic 18.5/9"),TEXT("Android_Default"),TEXT("Generic.18.5/9"),FVector2D(2220.0f, 1080.0f),TEXT("18.5/9"));
	SetGenericProfile(TEXT("Generic"),TEXT("Generic 19/9"),TEXT("Android_Default"),TEXT("Generic.19/9"),FVector2D(2280.0f, 1080.0f),TEXT("19/9"));
	SetGenericProfile(TEXT("Generic"),TEXT("Generic 19.5/9"),TEXT("Android_Default"),TEXT("Generic.19.5/9"),FVector2D(2340.0f, 1080.0f),TEXT("19.5/9"));
	SetGenericProfile(TEXT("Generic"),TEXT("Generic 20/9"),TEXT("Android_Default"),TEXT("Generic.20/9"),FVector2D(2400.0f, 1080.0f),TEXT("20/9"));
	SetGenericProfile(TEXT("Generic"),TEXT("Generic 21/9"),TEXT("Android_Default"),TEXT("Generic.21/9"),FVector2D(2520.0f, 1080.0f),TEXT("21/9"));
		
	
	// ANDROID PROFILE

	SetAndroidProfile(TEXT("Sony"),TEXT("Sony Xperia 1 Mark II (2020)"),TEXT("Android_Adreno6xx"),TEXT("Sony.Xperia1.II"),FVector2D(3840.0f, 1644.0f),TEXT("21/9"));

	SetAndroidProfile(TEXT("Samsung"),TEXT("[Exynos] Samsung S20 (2020)"),TEXT("Android_Mali_G77"),TEXT("Samsung.S20"),FVector2D(3200.0f, 1440.0f),TEXT("20/9"));
	//SetAndroidProfile(TEXT("Samsung"),TEXT("[Exynos] Samsung Galaxy Note 20 Ultra (2020)"),TEXT("Android_Mali_G77"),TEXT("Samsung.Note20.Ultra"),FVector2D(3200.0f, 1440.0f),TEXT("20/9"));
	SetAndroidProfile(TEXT("Samsung"),TEXT("[Exynos] Samsung Galaxy Note 20 (2020)"),TEXT("Android_Mali_G77"),TEXT("Samsung.Note20"),FVector2D(2400.0f, 1080.0f),TEXT("20/9"));
	SetAndroidProfile(TEXT("Samsung"),TEXT("[Exynos] Samsung Galaxy A50 (2019)"),TEXT("Android_Mali_G72"),TEXT("Samsung.A50"),FVector2D(2340.0f, 1080.0f),TEXT("19.5/9"));
	//Todo: Version Adreno
	// Samsung Note 8 -> try to Render at 1280x720 because Device Content Scale of 1, if 2 = 2560x1440. But this is 18.5/9 so 1480x720
	//Set(TEXT("Samsung Note 8"), TEXT("Android_Adreno6xx"), TEXT("Samsung.Note.8"), FVector2D(2960.0f, 1440.0f));

	//Curved Screen
	//SetAndroidProfile(TEXT("Samsung"),TEXT("Samsung S10"),TEXT("Android_Mali_G76"),TEXT("Samsung.S10"),FVector2D(3040.0f, 1440.0f),TEXT("16/9"));

	//SetAndroidProfile(TEXT("Samsung"),TEXT("Samsung S22+"), TEXT("Android_Xclipse_920"), TEXT("Samsung.S22+"), FVector2D(3040.0f, 1440.0f),TEXT("16/9"));

	//Curved screen
	//SetAndroidProfile(TEXT("Samsung"),TEXT("Samsung S22 Ultra"), TEXT("Android_Xclipse_920"), TEXT("Samsung.S22.Ultra"), FVector2D(3088.0f, 1440.0f),TEXT("16/9"));
	
	//SetAndroidProfile(TEXT("Oppo"),TEXT("Oppo Find X5 Pro (2022)"),TEXT("Android_Adreno7xx"),TEXT("Oppo.FindX5.Pro"),FVector2D(3216.0f, 1440.0f),TEXT("20.1/9"));

	SetAndroidProfile(TEXT("Google"),TEXT("Google Pixel 5 (2020)"),TEXT("Android_Adreno6xx"),TEXT("Google.Pixel.5"),FVector2D(2340.0f, 1080.0f),TEXT("19.5/9"));
	
	SetAndroidProfile(TEXT("OnePlus"),TEXT("OnePlus 8 Pro (2020)"),TEXT("Android_Adreno6xx"),TEXT("OnePlus.8.Pro"),FVector2D(3168.0f, 1440.0f),TEXT("19.8/9"));

	//Todo: add some android tablet

	//IPHONE
	
	/** For example, a Retina iPad has actual resolution 2048x1536, but logical resolution 1024x768, so you would use 2.0 to get native resolution there.
	 * You can also use fractional values like 1.5 if you want more resolution but not the full performance hit from running at native resolution.
	 */
	
	//SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 11"), TEXT("iPhone11"), TEXT("iPhone11"), FVector2D(1792.0f, 828.0f), FVector2D(896.0f, 414.0f),TEXT("16/9"));
	SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 11 Pro (2019)"), TEXT("iPhone11Pro"), TEXT("iPhone11.Pro"), FVector2D(2436.0f, 1125.0f),FVector2D(812.0f, 375.0f),TEXT("19.5/9"));
	//SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 11 Pro Max"), TEXT("iPhone11ProMax"), TEXT("iPhone11.ProMax"), FVector2D(2688.0f, 1242.0f),FVector2D(896.0f, 414.0f),TEXT("16/9"));

	SetAppleProfile(TEXT("iPhone"),TEXT("iPhone SE 2nd gen (2020)"), TEXT("iPhoneSE2"), TEXT("iPhoneSE2"), FVector2D(1334.0f, 750.0f),FVector2D(667.0f, 375.0f),TEXT("16/9"));
	
	//SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 12 Mini"), TEXT("iPhone12Mini"), TEXT("iPhone12.Mini"), FVector2D(2340.0f, 1080.0f),FVector2D(812.0f, 375.0f),TEXT("16/9"));
	//SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 12"), TEXT("iPhone12"), TEXT("iPhone12"), FVector2D(2532.0f, 1170.0f),FVector2D(844.0f, 390.0f),TEXT("16/9"));
	SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 12 Pro (2020)"), TEXT("iPhone12Pro"), TEXT("iPhone12.Pro"), FVector2D(2532.0f, 1170.0f),FVector2D(844.0f, 390.0f),TEXT("19.5/9"));
	//SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 12 Pro Max"), TEXT("iPhone12ProMax"), TEXT("iPhone12.ProMax"), FVector2D(2778.0f, 1280.0f),FVector2D(926.0f, 428.0f),TEXT("16/9"));

	//SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 13 Mini"), TEXT("iPhone13Mini"), TEXT("iPhone13.Mini"), FVector2D(2340.0f, 1080.0f),FVector2D(812.0f, 375.0f),TEXT("16/9"));
	//SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 13"), TEXT("iPhone13"), TEXT("iPhone13"), FVector2D(2532.0f, 1170.0f),FVector2D(844.0f, 390.0f),TEXT("16/9"));
	SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 13 Pro (2021)"), TEXT("iPhone13Pro"), TEXT("iPhone13.Pro"), FVector2D(2532.0f, 1170.0f),FVector2D(844.0f, 390.0f),TEXT("19.5/9"));
	//SetAppleProfile(TEXT("iPhone"),TEXT("iPhone 13 Pro Max"), TEXT("iPhone13ProMax"), TEXT("iPhone13.ProMax"), FVector2D(2778.0f, 1284.0f),FVector2D(926.0f, 428.0f),TEXT("16/9"));

	//IPAD
	
	//SetAppleProfile(TEXT("iPad"),TEXT("iPad Pro 3nd gen (2018) 11"), TEXT("iPadPro11"), TEXT("iPadPro11"), FVector2D(2388.0f, 1668.0f),FVector2D(1194.0f, 834.0f),TEXT("4/3"));

	//SetAppleProfile(TEXT("iPad"),TEXT("iPad Air 4th gen (2020)"), TEXT("iPadAir4"), TEXT("iPadAir4"), FVector2D(2360.0f, 1640.0f),FVector2D(1180.0f, 820.0f),TEXT("4/3"));

	SetAppleProfile(TEXT("iPad"),TEXT("iPad 8th gen (2020) 10.2"), TEXT("iPad8"), TEXT("iPad.2020"), FVector2D(2160.0f, 1620.0f),FVector2D(1080.0f, 810.0f),TEXT("4/3"));
	SetAppleProfile(TEXT("iPad"),TEXT("iPad Pro 4th gen (2020) 11"), TEXT("iPadPro2_11"), TEXT("iPadPro.2020"), FVector2D(2388.0f, 1668.0f),FVector2D(1194.0f, 834.0f),TEXT("4/3"));
}

void FTouchInterfaceDesignerDeviceProfile::Deinitialize()
{
	FDesignerDeviceStyle::UnregisterStyle();
}

void FTouchInterfaceDesignerDeviceProfile::SetAndroidProfile(const FName Category, const FString DeviceName, const FString DeviceProfileName, const FString StyleName, const FVector2D ScreenDefinition, const FString ScreenFormat)
{
	DesignerDeviceProfiles.Add(DeviceName, FDesignerDeviceProfile(Category, DeviceProfileName, StyleName, FVector2D::ZeroVector, ScreenDefinition, ScreenFormat, false));
}

void FTouchInterfaceDesignerDeviceProfile::SetAppleProfile(const FName Category, const FString DeviceName, const FString DeviceProfileName, const FString StyleName, const FVector2D ScreenDefinition, const FVector2D LogicalResolution, const FString ScreenFormat)
{
	DesignerDeviceProfiles.Add(DeviceName, FDesignerDeviceProfile(Category, DeviceProfileName, StyleName, LogicalResolution, ScreenDefinition, ScreenFormat, false));
}

void FTouchInterfaceDesignerDeviceProfile::SetGenericProfile(const FName Category, const FString DeviceName, const FString DeviceProfileName, const FString StyleName, const FVector2D ScreenDefinition, const FString ScreenFormat)
{
	DesignerDeviceProfiles.Add(DeviceName, FDesignerDeviceProfile(Category, DeviceProfileName, StyleName, FVector2D::ZeroVector, ScreenDefinition, ScreenFormat, true));
}

bool FTouchInterfaceDesignerDeviceProfile::GetProfile(const FString DeviceName, FDesignerDeviceProfile& Profile)
{
	if (DesignerDeviceProfiles.Contains(DeviceName))
	{
		Profile = DesignerDeviceProfiles.FindRef(DeviceName);
		return true;
	}

	return false;
}

TArray<FString> FTouchInterfaceDesignerDeviceProfile::GetAllProfileNameByCategory(const FName Category)
{
	TArray<FString> ProfileNames;
	DesignerDeviceProfiles.GenerateKeyArray(ProfileNames);

	TArray<FString> ProfileNamesByCategory;
	for (const FString& ProfileName : ProfileNames)
	{
		if (DesignerDeviceProfiles.FindRef(ProfileName).Category == Category)
		{
			ProfileNamesByCategory.Add(ProfileName);
		}
	}
	
	return ProfileNamesByCategory;
}

bool FTouchInterfaceDesignerDeviceProfile::GetAllCategories(TArray<FName>& Categories)
{
	TArray<FDesignerDeviceProfile> DeviceProfiles;
	DesignerDeviceProfiles.GenerateValueArray(DeviceProfiles);

	FName CurrentCategory = NAME_None;
	for (FDesignerDeviceProfile& Profile : DeviceProfiles)
	{
		if (Profile.Category != CurrentCategory)
		{
			CurrentCategory = Profile.Category;
			Categories.Add(Profile.Category);
		}
	}

#if ENGINE_MAJOR_VERSION > 4
	return !Categories.IsEmpty();
#else
	return Categories.Num() > 0;
#endif
	
}

bool FTouchInterfaceDesignerDeviceProfile::GetAllProfileByCategory(const FName Category, TArray<FDesignerDeviceProfile>& Profiles)
{
	TArray<FDesignerDeviceProfile> DeviceProfiles;
	DesignerDeviceProfiles.GenerateValueArray(DeviceProfiles);
	
	for (FDesignerDeviceProfile& Profile : DeviceProfiles)
	{
		if (Profile.Category == Category)
		{
			Profiles.Add(Profile);
		}
	}

#if ENGINE_MAJOR_VERSION > 4
	return !Profiles.IsEmpty();
#else
	return Profiles.Num() > 0;
#endif
	
}

FVector2D FTouchInterfaceDesignerDeviceProfile::GetScreenDefinition(const FString DeviceName)
{
	FDesignerDeviceProfile Profile;

	if (GetProfile(DeviceName, Profile))
	{
		return Profile.ScreenDefinition;
	}

	return FVector2D::ZeroVector;
}

FString FTouchInterfaceDesignerDeviceProfile::GetDeviceProfileName(const FString DeviceName)
{
	FDesignerDeviceProfile Profile;

	if (GetProfile(DeviceName, Profile))
	{
		return Profile.DeviceProfileName;
	}

	return FString();
}

FString FTouchInterfaceDesignerDeviceProfile::GetDeviceStyleName(const FString DeviceName)
{
	FDesignerDeviceProfile Profile;

	if (GetProfile(DeviceName, Profile))
	{
		return Profile.StyleName;
	}
	
	return FString();
}

const FSlateBrush* FTouchInterfaceDesignerDeviceProfile::GetSlateBrush(const FString DeviceName)
{
	return FDesignerDeviceStyle::Get().GetBrush(FName(GetDeviceStyleName(DeviceName)));
}


// Designer Device Style

#define RootToContentDir Style->RootToContentDir

TSharedPtr<ISlateStyle> FDesignerDeviceStyle::StyleSet = nullptr;

void FDesignerDeviceStyle::RegisterStyle()
{
	if (StyleSet.IsValid()) return;

	StyleSet = Create();
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FDesignerDeviceStyle::UnregisterStyle()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

void FDesignerDeviceStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<ISlateStyle> FDesignerDeviceStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("DesignerDeviceStyle"));

/*#if ENGINE_MAJOR_VERSION > 4

	Style->SetContentRoot(IPluginManager::Get().FindPlugin("TouchInterfaceDesigner")->GetBaseDir()/TEXT("Resources/SVG/Mockup"));
	
	// Generic
	Style->Set("Generic.16/9", new IMAGE_BRUSH_SVG("Generic_16_9", FVector2D(1536, 4096)));
	Style->Set("Generic.18/9", new IMAGE_BRUSH_SVG("Generic_18_9", FVector2D(1536, 4096)));
	Style->Set("Generic.18.5/9", new IMAGE_BRUSH_SVG("Generic_18.5_9", FVector2D(1536, 4096)));
	Style->Set("Generic.19/9", new IMAGE_BRUSH_SVG("Generic_19_9", FVector2D(1536, 4096)));
	Style->Set("Generic.19.5/9", new IMAGE_BRUSH_SVG("Generic_19.5_9", FVector2D(1536, 4096)));
	Style->Set("Generic.20/9", new IMAGE_BRUSH_SVG("Generic_20_9", FVector2D(1536, 4096)));
	Style->Set("Generic.21/9", new IMAGE_BRUSH_SVG("Generic_21_9", FVector2D(1536, 4096)));

	// Sony Xperia 1 II
	Style->Set("Sony.Xperia1.II", new IMAGE_BRUSH_SVG("SonyXperia1_Mark2", FVector2D(1536, 4096)));

	// Samsung
	Style->Set("Samsung.S20", new IMAGE_BRUSH_SVG("SamsungS20", FVector2D(1536, 4096)));
	Style->Set("Samsung.Note20", new IMAGE_BRUSH_SVG("SamsungNote20", FVector2D(1536, 4096)));
	Style->Set("Samsung.Note20.Ultra", new IMAGE_BRUSH_SVG("SamsungNote20Ultra", FVector2D(1536, 4096)));
	Style->Set("Samsung.A50", new IMAGE_BRUSH_SVG("SamsungA50_2019", FVector2D(1536, 4096)));

	// Google
	Style->Set("Google.Pixel.5", new IMAGE_BRUSH_SVG("GooglePixel5", FVector2D(1536, 4096)));

	//OnePlus
	Style->Set("OnePlus.8.Pro", new IMAGE_BRUSH_SVG("OnePlus8Pro", FVector2D(1536, 4096)));
	
	// iPhone
	Style->Set("iPhone11.Pro", new IMAGE_BRUSH_SVG("iPhone11Pro", FVector2D(1536, 4096)));
	Style->Set("iPhoneSE2", new IMAGE_BRUSH_SVG("iPhoneSE2", FVector2D(1536, 4096)));
	Style->Set("iPhone12.Pro", new IMAGE_BRUSH_SVG("iPhone12Pro", FVector2D(1536, 4096)));
	Style->Set("iPhone13.Pro", new IMAGE_BRUSH_SVG("iPhone13Pro",FVector2D(1536, 4096)));

	// iPad
	Style->Set("iPad.2020", new IMAGE_BRUSH_SVG("iPad_2020", FVector2D(1536, 4096)));
	Style->Set("iPadPro.2020", new IMAGE_BRUSH_SVG("iPadPro_2020", FVector2D(1536, 4096)));
	
#else*/

	Style->SetContentRoot(IPluginManager::Get().FindPlugin("TouchInterfaceDesigner")->GetBaseDir()/TEXT("Resources/Mockup"));
	
	// Generic
	Style->Set("Generic.16/9", new IMAGE_BRUSH("Generic_16_9", FVector2D(1536, 4096)));
	Style->Set("Generic.18/9", new IMAGE_BRUSH("Generic_18_9", FVector2D(1536, 4096)));
	Style->Set("Generic.18.5/9", new IMAGE_BRUSH("Generic_18.5_9", FVector2D(1536, 4096)));
	Style->Set("Generic.19/9", new IMAGE_BRUSH("Generic_19_9", FVector2D(1536, 4096)));
	Style->Set("Generic.19.5/9", new IMAGE_BRUSH("Generic_19.5_9", FVector2D(1536, 4096)));
	Style->Set("Generic.20/9", new IMAGE_BRUSH("Generic_20_9", FVector2D(1536, 4096)));
	Style->Set("Generic.21/9", new IMAGE_BRUSH("Generic_21_9", FVector2D(1536, 4096)));

	// Sony Xperia 1 II
	Style->Set("Sony.Xperia1.II", new IMAGE_BRUSH("SonyXperia1_Mark2", FVector2D(1536, 4096)));

	// Samsung
	Style->Set("Samsung.S20", new IMAGE_BRUSH("SamsungS20", FVector2D(1536, 4096)));
	Style->Set("Samsung.Note20", new IMAGE_BRUSH("SamsungNote20", FVector2D(1536, 4096)));
	Style->Set("Samsung.Note20.Ultra", new IMAGE_BRUSH("SamsungNote20Ultra", FVector2D(1536, 4096)));
	Style->Set("Samsung.A50", new IMAGE_BRUSH("SamsungA50_2019", FVector2D(1536, 4096)));

	// Google
	Style->Set("Google.Pixel.5", new IMAGE_BRUSH("GooglePixel5", FVector2D(1536, 4096)));

	//OnePlus
	Style->Set("OnePlus.8.Pro", new IMAGE_BRUSH("OnePlus8Pro", FVector2D(1536, 4096)));
	
	// iPhone
	Style->Set("iPhone11.Pro", new IMAGE_BRUSH("iPhone11Pro", FVector2D(1536, 4096)));
	Style->Set("iPhoneSE2", new IMAGE_BRUSH("iPhoneSE2", FVector2D(1536, 4096)));
	Style->Set("iPhone12.Pro", new IMAGE_BRUSH("iPhone12Pro", FVector2D(1536, 4096)));
	Style->Set("iPhone13.Pro", new IMAGE_BRUSH("iPhone13Pro",FVector2D(1536, 4096)));

	// iPad
	Style->Set("iPad.2020", new IMAGE_BRUSH("iPad_2020", FVector2D(1536, 4096)));
	Style->Set("iPadPro.2020", new IMAGE_BRUSH("iPadPro_2020", FVector2D(1536, 4096)));
	
//#endif
	
	return Style;
}
