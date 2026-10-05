// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	APortal();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneCaptureComponent2D* sceneCapture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UArrowComponent* rootArrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UTextureRenderTarget2D* renderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	APortal* OtherPortal;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void SetBool(APascual_415Character* PlayerCharacter);

	void UpdatePortals();
};
