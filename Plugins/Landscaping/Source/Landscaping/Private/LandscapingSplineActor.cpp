// Copyright (c) 2021 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "LandscapingSplineActor.h"
#include "Components/SplineComponent.h"
#include "LandscapingUtils.h"

ALandscapingSplineActor::ALandscapingSplineActor()
{
#if WITH_EDITOR
	PrimaryActorTick.bCanEverTick = true;
#endif

	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
	if(SplineComponent)
	{
		SetRootComponent(SplineComponent); 
	} 
}

void ALandscapingSplineActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALandscapingSplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALandscapingSplineActor::OnConstruction(const FTransform& Transform)
{	
	Super::OnConstruction(Transform);

	ConstructSpline();
}

void ALandscapingSplineActor::ConstructSpline()
{
	if(SplineComponent && SplineMeshDetails.Mesh != nullptr)
	{
		double SplineLength = SplineComponent->GetSplineLength();
		FVector MeshSize = SplineMeshDetails.Mesh->GetBounds().GetBox().GetSize();
		double MeshLength = 0;
		if(SplineMeshDetails.ForwardAxis == ESplineMeshAxis::X)
		{
			MeshLength = MeshSize.X;
		}
		else if(SplineMeshDetails.ForwardAxis == ESplineMeshAxis::Y)
		{
			MeshLength = MeshSize.Y;
		} 
		else if(SplineMeshDetails.ForwardAxis == ESplineMeshAxis::Z)
		{
			MeshLength = MeshSize.Z;
		}
		double SegmentsCount = SplineLength / MeshLength;
		for(int SplineCount = 0; SplineCount < (SegmentsCount - 1); SplineCount++)
		{
			USplineMeshComponent *SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());

			UStaticMesh* StaticMesh = SplineMeshDetails.Mesh;
			UMaterialInterface* Material = nullptr;
			ESplineMeshAxis::Type ForwardAxis = SplineMeshDetails.ForwardAxis;

			// update mesh details
			SplineMesh->SetStaticMesh(StaticMesh);
			SplineMesh->SetForwardAxis(ForwardAxis, true);
	
			// initialize the object
			SplineMesh->RegisterComponentWithWorld(GetWorld());
	
			SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			SplineMesh->SetMobility(EComponentMobility::Movable);
	
			SplineMesh->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);
	
			// define the positions of the points and tangents
			const FVector StartPoint = SplineComponent->GetLocationAtDistanceAlongSpline(MeshLength * SplineCount, ESplineCoordinateSpace::Type::Local);
			const FVector StartTangent = SplineComponent->GetTangentAtDistanceAlongSpline(MeshLength * SplineCount, ESplineCoordinateSpace::Type::Local).GetClampedToSize(0, MeshLength);
			const FVector EndPoint = SplineComponent->GetLocationAtDistanceAlongSpline(MeshLength * (SplineCount + 1), ESplineCoordinateSpace::Type::Local);
			const FVector EndTangent = SplineComponent->GetTangentAtDistanceAlongSpline(MeshLength * (SplineCount + 1), ESplineCoordinateSpace::Type::Local).GetClampedToSize(0, MeshLength);
			SplineMesh->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent, true);
			SplineMesh->SetStartScale(FVector2D(StartScale));
			SplineMesh->SetEndScale(FVector2D(EndScale));
	
			// query physics
			SplineMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		}
	}
}

FVector ALandscapingSplineActor::SnapToFloor(FVector Location, float OffsetFromFloor, AActor* ActorWithSplineComp)
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
	PostSplineUpdate(ActorWithSplineComp);
	return Result;
}

bool ALandscapingSplineActor::ToggleCollisionEnabled(AActor* ActorWithSplineComp)
{
	if(ActorWithSplineComp)
	{
		bool bCollisionEnabled = ActorWithSplineComp->GetActorEnableCollision();
		ActorWithSplineComp->SetActorEnableCollision(!bCollisionEnabled);
		return ActorWithSplineComp->GetActorEnableCollision();
	}
	UE_LOG(LogTemp, Error, TEXT("Landscaping: No Actor to enable or disable Collision on"));
	return false;
}

void ALandscapingSplineActor::SetNewActorLabel(AActor* InActor, FString NewName)
{
#if WITH_EDITOR
	if(InActor)
	{
		InActor->SetActorLabel(NewName);
	}
#endif
}

void ALandscapingSplineActor::SetEditorOnly(AActor* InActor, bool bIsEditorOnly)
{
#if WITH_EDITOR
	if(InActor)
	{
		InActor->bIsEditorOnlyActor = true;
	}
#endif
}

void ALandscapingSplineActor::PostSplineUpdate(AActor* ActorWithSplineComp)
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