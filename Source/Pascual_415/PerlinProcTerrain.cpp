// fill out your copyright notice in the Description page of Project Settings.

#include "PerlinProcTerrain.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"

// initialize default subobjects and starting settings for the terrain
APerlinProcTerrain::APerlinProcTerrain()
{
	PrimaryActorTick.bCanEverTick = false;

	procMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("procMesh"));
	RootComponent = procMesh;

	XSize = 10;
	YSize = 10;
	ZMultiplier = 100.0f;
	NoiseScale = 0.1f;
	Scale = 100.0f;
	UVScale = 1.0f;
	Radius = 100.0f;
	Depth = 50.0f;
	sectionID = 0;
}

// build the terrain when playing the game or loading into the editor
void APerlinProcTerrain::BeginPlay()
{
	Super::BeginPlay();
	GenerateMesh();
}

void APerlinProcTerrain::PostActorCreated()
{
	Super::PostActorCreated();
	GenerateMesh();
}

void APerlinProcTerrain::PostLoad()
{
	Super::PostLoad();
	GenerateMesh();
}

#if WITH_EDITOR
// rebuild the mesh live when tweaking properties in the editor details panel
void APerlinProcTerrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	GenerateMesh();
}
#endif

// rebuilds the entire 3D terrain mesh from scratch
void APerlinProcTerrain::GenerateMesh()
{
	Vertices.Reset();
	Triangles.Reset();
	UVs.Reset();
	Normals.Reset();
	Tangents.Reset();

	CreateVertices();
	CreateTriangles();

	// calculate smooth normals and tangents for proper lighting
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);

	if (procMesh)
	{
		procMesh->CreateMeshSection(
			sectionID,
			Vertices,
			Triangles,
			Normals,
			UVs,
			TArray<FColor>(),
			Tangents,
			true
		);

		if (Material)
		{
			procMesh->SetMaterial(sectionID, Material);
		}
	}
}

// generate grid vertices using 2D perlin noise to set the height of each point
void APerlinProcTerrain::CreateVertices()
{
	for (int32 X = 0; X <= XSize; X++)
	{
		for (int32 Y = 0; Y <= YSize; Y++)
		{
			float Z = FMath::PerlinNoise2D(FVector2D(X * NoiseScale, Y * NoiseScale)) * ZMultiplier;
			Vertices.Add(FVector(X * Scale, Y * Scale, Z));
			UVs.Add(FVector2D(X * UVScale, Y * UVScale));
		}
	}
}

// connects grid vertices into pairs of clockwise triangles to create 3D quads
void APerlinProcTerrain::CreateTriangles()
{
	int32 Vertex = 0;
	for (int32 X = 0; X < XSize; X++)
	{
		for (int32 Y = 0; Y < YSize; Y++)
		{
			Triangles.Add(Vertex);
			Triangles.Add(Vertex + 1);
			Triangles.Add(Vertex + YSize + 1);

			Triangles.Add(Vertex + 1);
			Triangles.Add(Vertex + YSize + 2);
			Triangles.Add(Vertex + YSize + 1);

			Vertex++;
		}
		Vertex++;
	}
}

// lowers all vertices within the blast radius to dig a hole at the impact point
void APerlinProcTerrain::AlterMesh(FVector ImpactPoint)
{
	FVector tempVector = ImpactPoint - GetActorLocation();

	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		if ((Vertices[i] - tempVector).Size() < Radius)
		{
			Vertices[i].Z -= Depth;
		}
	}

	if (procMesh)
	{
		procMesh->UpdateMeshSection(
			sectionID,
			Vertices,
			Normals,
			UVs,
			TArray<FColor>(),
			Tangents
		);
	}
}
