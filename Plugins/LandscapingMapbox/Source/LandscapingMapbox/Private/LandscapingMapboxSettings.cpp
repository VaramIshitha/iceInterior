// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "LandscapingMapboxSettings.h"

ULandscapingMapboxSettings::ULandscapingMapboxSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ApiKey(TEXT(""))
	, Zoom(14)
	, ZoomVector(14)
	, ZoomSatellite(16)

{
	
}

#if WITH_EDITOR

void ULandscapingMapboxSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	//Get the name of the property that was changed  
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULandscapingMapboxSettings, Zoom))
	{
		if(!(Zoom > -1 && Zoom < 16))
		{
			Zoom = 14;
		}
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULandscapingMapboxSettings, ZoomVector))
	{
		if(!(ZoomVector > -1 && ZoomVector < 31))
		{
			Zoom = 14;
		}
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULandscapingMapboxSettings, ZoomSatellite))
	{
		if(!(ZoomSatellite > -1 && ZoomSatellite < 22))
		{
			ZoomSatellite = 16;
		}
	}
	
	// Call the base class version  
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif