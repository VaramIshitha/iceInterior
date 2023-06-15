// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "LandscapingSettings.h"

ULandscapingSettings::ULandscapingSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CacheDirectory(TEXT("C:/Temp/Landscaping"))
{
	
}

#if WITH_EDITOR

void ULandscapingSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	//Get the name of the property that was changed  
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULandscapingSettings, CacheDirectory))
	{
		CacheDirectory = CacheDirectory.Replace(TEXT("\\"), TEXT("/"));
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULandscapingSettings, Projection))
	{
		if(Projection < MIN_EPSG || Projection > MAX_EPSG)
		{
			Projection = 3857;
			UE_LOG(LogTemp, Error, TEXT("Landscaping: Please use a valid EPSG Code. You can look it up at https://epsg.io/"));
		}
	}
	
	// Call the base class version  
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif