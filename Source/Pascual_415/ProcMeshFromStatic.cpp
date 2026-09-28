// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcMeshFromStatic.h"
#include "KismetProceduralMeshLibrary.h"

// Sets default values
AProcMeshFromStatic::AProcMeshFromStatic()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	baseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("baseMesh"));
	RootComponent = baseMesh;

	procMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("procMesh"));
	procMesh->SetupAttachment(baseMesh);
}

void AProcMeshFromStatic::PostActorCreated()
{
	Super::PostActorCreated();
	GetMeshData();
}

void AProcMeshFromStatic::PostLoad()
{
	Super::PostLoad();
	GetMeshData();
}

#if WITH_EDITOR
void AProcMeshFromStatic::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	GetMeshData();
}
#endif

void AProcMeshFromStatic::GetMeshData()
{
	if (baseMesh)
	{
		UStaticMesh* mesh = baseMesh->GetStaticMesh();
		if (mesh)
		{
			UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(mesh, 0, 0, Vertices, Triangles, Normals, UVs, Tangents);
			CreateMesh();
		}
	}
}

void AProcMeshFromStatic::CreateMesh()
{
	if (baseMesh && procMesh)
	{
		procMesh->CreateMeshSection_LinearColor(
			0,
			Vertices,
			Triangles,
			Normals,
			UVs,
			VertexColors,
			Tangents,
			true
		);

		if (baseMesh->GetMaterial(0))
		{
			procMesh->SetMaterial(0, baseMesh->GetMaterial(0));
		}
	}
}
