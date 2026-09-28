// fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "PerlinProcTerrain.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class PASCUAL_415_API APerlinProcTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	APerlinProcTerrain();

	// customizable terrain settings for grid size, noise height, spacing, and digging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0"))
	int32 XSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0"))
	int32 YSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float ZMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float NoiseScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float UVScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Depth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 sectionID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProceduralMeshComponent* procMesh;

protected:
	// lifecycle functions to generate the mesh in-game and live in the editor
	virtual void BeginPlay() override;
	virtual void PostActorCreated() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	UMaterialInterface* Material;

public:	
	// vertex and triangle data buffers used to render the procedural mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
	TArray<FVector> Vertices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
	TArray<int32> Triangles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
	TArray<FVector> Normals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
	TArray<FVector2D> UVs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
	TArray<FProcMeshTangent> Tangents;

	// functions for generating the terrain geometry and digging into it on impact
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void AlterMesh(FVector ImpactPoint);

	void CreateVertices();
	void CreateTriangles();
	void GenerateMesh();
};
