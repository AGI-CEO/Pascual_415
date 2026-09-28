// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcPlane.h"
#include "ProceduralMeshComponent.h"

// Sets default values
AProcPlane::AProcPlane()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	procMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("procMesh"));
	RootComponent = procMesh;
}

void AProcPlane::PostActorCreated()
{
	Super::PostActorCreated();
	CreateMesh();
}

void AProcPlane::PostLoad()
{
	Super::PostLoad();
	CreateMesh();
}

void AProcPlane::CreateMesh()
{
	if (procMesh)
	{
		procMesh->CreateMeshSection(
			0,
			Vertices,
			Triangles,
			TArray<FVector>(),
			TArray<FVector2D>(),
			TArray<FColor>(),
			TArray<FProcMeshTangent>(),
			true
		);
	}
}
