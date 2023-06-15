// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/DataTable.h"
#include "LandscapingPaintLayerInterface.h"
#include "LandscapingSplineActor.generated.h"

USTRUCT(BlueprintType)
struct LANDSCAPING_API FSplineMeshDetails : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Landscaping)
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Landscaping)
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;

	FSplineMeshDetails() : ForwardAxis(ESplineMeshAxis::Type::X)
	{
	}
};

UCLASS(ClassGroup=(Landscaping))
class LANDSCAPING_API ALandscapingSplineActor : public AActor, public ILandscapingPaintLayerInterface
{
	GENERATED_BODY()
	
public:
	ALandscapingSplineActor();
	void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	FSplineMeshDetails SplineMeshDetails;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	USplineComponent* SplineComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	float StartScale = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	float EndScale = 1;
	
	UFUNCTION(BlueprintCallable, Category=Landscaping)
	void ConstructSpline();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category=Landscaping)
	void OnSplineCreated(USplineComponent* InSplineComponent);
	UFUNCTION(BlueprintCallable, Category=Landscaping)
	FVector SnapToFloor(FVector Location, float OffsetFromFloor, AActor* ActorWithSplineComp = nullptr);
	UFUNCTION(BlueprintCallable, Category=Landscaping)
	bool ToggleCollisionEnabled(AActor* ActorWithSplineComp);
	UFUNCTION(BlueprintCallable, Category=Landscaping)
	void SetNewActorLabel(AActor* InActor, FString NewName);
	UFUNCTION(BlueprintCallable, Category=Landscaping)
	void SetEditorOnly(AActor* InActor, bool bIsEditorOnly);
	UFUNCTION(BlueprintCallable, Category=Landscaping)
	void PostSplineUpdate(AActor* ActorWithSplineComp);

protected:
	virtual void BeginPlay() override;
};