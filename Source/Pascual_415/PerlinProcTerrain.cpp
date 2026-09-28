// Fill out your copyright notice in the Description page of Project Settings.

#include "PerlinProcTerrain.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"

// Sets default values
APerlinProcTerrain::APerlinProcTerrain()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
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
void APerlinProcTerrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	GenerateMesh();
}
#endif

void APerlinProcTerrain::GenerateMesh()
{
	Vertices.Reset();
	Triangles.Reset();
	UVs.Reset();
	Normals.Reset();
	Tangents.Reset();

	CreateVertices();
	CreateTriangles();

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
