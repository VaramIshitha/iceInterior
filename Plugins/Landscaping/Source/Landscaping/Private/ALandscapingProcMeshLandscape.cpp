// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "ALandscapingProcMeshLandscape.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "LandscapingUtils.h"

ALandscapingProcMeshLandscape::ALandscapingProcMeshLandscape(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
#endif

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	RootComponent = ProceduralMesh;
	if(ProcMeshId.IsEmpty())
	{
		ProcMeshId = LandscapingUtils::GetUniqueId();
	}

}

bool ALandscapingProcMeshLandscape::ShouldTickIfViewportsOnly() const
{
    return true;
}

void ALandscapingProcMeshLandscape::CreateMesh(FIntVector ImportResolution, double MeterPerPixelX, double MeterPerPixelY, TArray<double> InHeightData, bool bMeshCollision)
{
    if(InHeightData.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Cannot create Mesh - height data is empty"));
    }
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Create Mesh - Import Resolution: %i,%i - Meter Per Pixel: %f,%f - Heightdata: %i"), ImportResolution.X, ImportResolution.Y, MeterPerPixelX, MeterPerPixelY, InHeightData.Num());
    XSize = ImportResolution.X;
    YSize = ImportResolution.Y;
	HeightData = InHeightData;
    Scale.X = FMath::Abs(MeterPerPixelX) * 100;
    Scale.Y = FMath::Abs(MeterPerPixelY) * 100;
	CreateVertices();
	CreateTriangles();
	CalcNormalsAndTangents();
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, bMeshCollision);
	ProceduralMesh->MarkPackageDirty();
}

void ALandscapingProcMeshLandscape::AddVertexColor(TArray<FColor> InColor, int32 InWidth, int32 InHeight)
{
    TArray<FColor> ColorData;
    if(InColor.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Cannot add vertex color: Colors: %i - Vertices: %i"), InColor.Num(), Vertices.Num());
        return;
    }
    if(InColor.Num() != Vertices.Num())
    {
        ColorData = LandscapingUtils::ResampleFColor(InColor, InWidth, InHeight, XSize, YSize);
    }
    ProceduralMesh->UpdateMeshSection(0, Vertices, Normals, UV0, ColorData, Tangents);
	ProceduralMesh->MarkPackageDirty();
}

void ALandscapingProcMeshLandscape::SetMaterial(UMaterialInterface* InMaterial)
{
    Material = InMaterial;
    MaterialCached = Material;
	ProceduralMesh->SetMaterial(0, Material);
	ProceduralMesh->MarkPackageDirty();
}

void ALandscapingProcMeshLandscape::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if(Material != MaterialCached)
    {
        SetMaterial(Material);
    }
}

void ALandscapingProcMeshLandscape::CalcNormalsAndTangents()
{
	Normals.AddDefaulted(Vertices.Num());
	Tangents.AddDefaulted(Vertices.Num());
	ParallelFor (YSize, [&](int32 Y)
	{
		for (int32 X = 0; X < XSize; X++)
		{
			int32 XIndex = FMath::Clamp(X, 1, XSize - 2);
			int32 YIndex = FMath::Clamp(Y, 1, YSize - 2);
			double s01 = HeightData[XIndex - 1 + YIndex * XSize];
			double s21 = HeightData[XIndex + 1 + YIndex * XSize];
			double s10 = HeightData[XIndex + (YIndex - 1) * XSize];
			double s12 = HeightData[XIndex + (YIndex + 1) * XSize];

			// Get tangents in the x and y directions
			FVector vx(2.0, 0.0, s21 - s01);
			FVector vy(0.0, 2.0, s12 - s10);

			// Calculate the cross product of the two tangents
			vx.Normalize();
			vy.Normalize();

			FVector Normal = FVector::CrossProduct(vx, vy);
			FProcMeshTangent Tangent(vx, false);
			Normals[Y * XSize + X] = Normal;
			Tangents[Y * XSize + X] = Tangent;
		}
	});
}

void ALandscapingProcMeshLandscape::CreateVertices()
{
    for (int i = 0; i < YSize; i++)
	{
		for (int j = 0; j < XSize; j++)
		{
			Vertices.Add(FVector((double)j * Scale.X, (double)i * Scale.Y, HeightData[i * XSize + j] * 100));
			UV0.Add(FVector2D((double)j / ((double)XSize - 1), (double)i / ((double)YSize - 1)));
		}
	}
}

void ALandscapingProcMeshLandscape::CreateTriangles()
{
	UE_LOG(LogTemp, Log, TEXT("Landscaping: CreateTriangles - YSize: %i - XSize: %i - Triangles: %i"), YSize, XSize, Triangles.Num());
	for (int i = 0; i < YSize - 1; i++)
	{
		for (int j = 0; j < XSize - 1; j++)
		{
			int idx = j + (i * XSize);
			Triangles.Add(idx);
			Triangles.Add(idx + XSize);
			Triangles.Add(idx + 1);

			Triangles.Add(idx + 1);
			Triangles.Add(idx + XSize);
			Triangles.Add(idx + XSize + 1);
		}
	}
}
