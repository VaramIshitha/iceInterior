// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "LandscapingLandscapeSplines.h"
#include "Components/SplineComponent.h"

FVector ALandscapingLandscapeSplines::SnapToFloor(FVector Location, float OffsetFromFloor, AActor* ActorWithSplineComp)
{
	UWorld* World = nullptr;
	if(ActorWithSplineComp)
	{
		World = ActorWithSplineComp->GetOuter()->GetWorld();
		if(World == nullptr)
		{
			World = ActorWithSplineComp->GetWorld();
		}
	}
	FVector Result = LandscapingUtils::SnapToGround(Location, OffsetFromFloor, World);
	return Result;
}

bool ALandscapingLandscapeSplines::ToggleCollisionEnabled(AActor* ActorWithSplineComp)
{
	bool bCollisionEnabled = ActorWithSplineComp->GetActorEnableCollision();
	ActorWithSplineComp->SetActorEnableCollision(!bCollisionEnabled);
	return ActorWithSplineComp->GetActorEnableCollision();
}


void ALandscapingLandscapeSplines::PostSplineUpdate(AActor* ActorWithSplineComp)
{
#if WITH_EDITOR
	if(ActorWithSplineComp)
	{
		USplineComponent* SplineComp = Cast<USplineComponent>(ActorWithSplineComp->GetComponentByClass(USplineComponent::StaticClass()));
		if(SplineComp == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: There is no `SplineComponent` on Actor %s"), *ActorWithSplineComp->GetActorLabel());
			return;
		}
		FProperty* PropCurves = FindFieldChecked<FProperty>(SplineComp->GetClass(), FName("SplineCurves"));
		if(PropCurves != nullptr)
		{
			FPropertyChangedEvent PropertyChangedEvent(PropCurves);
			SplineComp->PostEditChangeProperty(PropertyChangedEvent);
			SplineComp->MarkPackageDirty();
			SplineComp->PostEditChange();
		}
		if(!SplineComp->GetName().Equals("SplineComponent"))
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: There is no Component named `SplineComponent` on Actor %s"), *ActorWithSplineComp->GetActorLabel());
			return;
		}
		FProperty* Prop = FindFieldChecked<FProperty>(ActorWithSplineComp->GetClass(), FName("SplineComponent"));
		if(Prop == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not find `%s` on Actor %s"), *SplineComp->GetName(), *ActorWithSplineComp->GetActorLabel());
		}
		else
		{
			FPropertyChangedEvent NewObjPropertyChangedEvent(Prop);
			ActorWithSplineComp->PostEditChangeProperty(NewObjPropertyChangedEvent);
		}
	}
#endif
}