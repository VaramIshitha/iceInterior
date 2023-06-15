// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "StaticMeshDescription.h"
#include "StaticMeshAttributes.h"
#include "ProceduralMeshComponent.h"
#include "ALandscapingProcMeshLandscape.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class LANDSCAPING_API ALandscapingProcMeshLandscape : public AActor
{
	GENERATED_BODY()

public:
	ALandscapingProcMeshLandscape(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, Category=Landscaping)
	FString ProcMeshId = FString();

	UPROPERTY(VisibleAnywhere, Category=Landscaping)
    int XSize = 0;
	UPROPERTY(VisibleAnywhere, Category=Landscaping)
    int YSize = 0;

	UPROPERTY(VisibleAnywhere, Category=Landscaping)
    FVector Scale = FVector(100);
	UPROPERTY(VisibleAnywhere, Category=Landscaping)
    float UVScale = 1.0f;

	UPROPERTY(EditAnyWhere, Category=Landscaping)
    UMaterialInterface* Material = nullptr;

	UProceduralMeshComponent* ProceduralMesh;
	
public:
	virtual void Tick(float DeltaTime) override;

    virtual bool ShouldTickIfViewportsOnly() const override;

	UFUNCTION(Category=Landscaping)
    void CreateMesh(FIntVector ImportResolution, double MeterPerPixelX, double MeterPerPixelY, TArray<double> InHeightData, bool bMeshCollision);
	
	UFUNCTION(Category=Landscaping)
    void AddVertexColor(TArray<FColor> InColor, int32 InWidth, int32 InHeight);
	
	UFUNCTION(Category=Landscaping)
    void SetMaterial(UMaterialInterface* InMaterial);

private:
	TArray<FVector> Vertices = TArray<FVector>();
	TArray<int> Triangles = TArray<int>();
	TArray<FVector2D> UV0 = TArray<FVector2D>();
	TArray<FVector2D> UV1 = TArray<FVector2D>();
	TArray<FVector> Normals = TArray<FVector>();
	TArray<FProcMeshTangent> Tangents = TArray<FProcMeshTangent>();
    TArray<double> HeightData = TArray<double>();
    UMaterialInterface* MaterialCached = nullptr;
	void CalcNormalsAndTangents();
	void CreateVertices();
	void CreateTriangles();
};