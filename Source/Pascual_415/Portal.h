// fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ArrowComponent.h"
#include "Portal.generated.h"

class APascual_415Character;
class UTextureRenderTarget2D;
class UMaterialInterface;

UCLASS()
class PASCUAL_415_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// sets default values for this actor's properties
	APortal();

	// updates materials and render targets live in the editor
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// sets up overlap events and hides portal meshes from the camera
	virtual void BeginPlay() override;

public:	
	// updates the portal camera every frame
	virtual void Tick(float DeltaTime) override;

	// components for the trigger box, the portal doorway mesh, the scene capture camera, and the exit spawn arrow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneCaptureComponent2D* sceneCapture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UArrowComponent* rootArrow;

	// portal settings to link the other portal, its render target texture, and its material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UTextureRenderTarget2D* renderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	APortal* OtherPortal;

	// teleports the player to the other portal when they walk into the trigger
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// resets the teleport check after a short delay so we can use portals again
	UFUNCTION()
	void SetBool(APascual_415Character* PlayerCharacter);

	// moves and rotates the scene capture camera based on where the player is looking
	void UpdatePortals();
};
