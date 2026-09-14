// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeDMIMod.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "Pascual_415Character.h"

// Sets default values
ACubeDMIMod::ACubeDMIMod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	boxComp->InitBoxExtent(FVector(75.0f, 75.0f, 75.0f));
	boxComp->SetCollisionProfileName(TEXT("Trigger"));
	boxComp->SetGenerateOverlapEvents(true);
	RootComponent = boxComp;

	cubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube Mesh"));
	cubeMesh->SetupAttachment(boxComp);

}

// Called when the game starts or when spawned
void ACubeDMIMod::BeginPlay()
{
	Super::BeginPlay();
	boxComp->OnComponentBeginOverlap.AddDynamic(this, &ACubeDMIMod::OnOverlapBegin);
	if (baseMat)
	{
		dmiMat = UMaterialInstanceDynamic::Create(baseMat, this);
	}
	else if (cubeMesh && cubeMesh->GetMaterial(0))
	{
		dmiMat = cubeMesh->CreateDynamicMaterialInstance(0);
	}

	if (cubeMesh && dmiMat)
	{
		cubeMesh->SetMaterial(0, dmiMat);
	}
}

// Called every frame
void ACubeDMIMod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ACubeDMIMod::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APascual_415Character* overlappedActor = Cast<APascual_415Character>(OtherActor);
	if (overlappedActor || (OtherActor && OtherActor->ActorHasTag("Player")))
	{
		float ranNumX = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
		float ranNumY = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
		float ranNumZ = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
		
		FLinearColor randColor = FLinearColor(ranNumX, ranNumY, ranNumZ, 1.0f);
		if (dmiMat)
		{
			dmiMat->SetVectorParameterValue("Color", randColor);
			dmiMat->SetScalarParameterValue("Darkness", ranNumX);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CubeDMIMod: Overlapped, but dmiMat is null! Ensure baseMat is assigned in Class Defaults."));
		}
	}
}