// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal.h"
#include "Pascual_415Character.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/TextureRenderTarget2D.h"

// Sets default values
APortal::APortal()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetCollisionProfileName(TEXT("Trigger"));
	BoxComponent->SetGenerateOverlapEvents(true);
	RootComponent = BoxComponent;

	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	mesh->SetupAttachment(RootComponent);
	mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	mesh->SetCastShadow(false);
	mesh->SetHiddenInSceneCapture(true);

	sceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	sceneCapture->SetupAttachment(mesh);

	rootArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RootArrow"));
	rootArrow->SetupAttachment(RootComponent);
}

void APortal::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Material && mesh)
	{
		mesh->SetMaterial(0, Material);
	}

	if (renderTarget && sceneCapture)
	{
		sceneCapture->TextureTarget = renderTarget;
	}
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnOverlapBegin);

	if (Material && mesh)
	{
		mesh->SetMaterial(0, Material);
	}

	if (renderTarget && sceneCapture)
	{
		sceneCapture->TextureTarget = renderTarget;
	}

	if (mesh)
	{
		mesh->SetHiddenInSceneCapture(true);
		mesh->SetCastShadow(false);
	}

	if (sceneCapture)
	{
		sceneCapture->HiddenActors.AddUnique(this);
		if (mesh)
		{
			sceneCapture->HideComponent(mesh);
		}
		if (OtherPortal)
		{
			sceneCapture->HiddenActors.AddUnique(OtherPortal);
			if (OtherPortal->mesh)
			{
				sceneCapture->HideComponent(OtherPortal->mesh);
			}
		}
	}
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePortals();
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APascual_415Character* PlayerCharacter = Cast<APascual_415Character>(OtherActor);
	if (PlayerCharacter)
	{
		if (OtherPortal)
		{
			if (!PlayerCharacter->isTeleporting)
			{
				PlayerCharacter->isTeleporting = true;

				FVector TargetLocation = (OtherPortal->rootArrow) ? OtherPortal->rootArrow->GetComponentLocation() : OtherPortal->GetActorLocation();
				PlayerCharacter->SetActorLocation(TargetLocation);

				FTimerHandle TimerHandle;
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindUFunction(this, FName("SetBool"), PlayerCharacter);
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.0f, false);
			}
		}
	}
}

void APortal::SetBool(APascual_415Character* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		PlayerCharacter->isTeleporting = false;
	}
}

void APortal::UpdatePortals()
{
	if (!OtherPortal || !sceneCapture)
	{
		return;
	}

	if (sceneCapture->HiddenActors.Num() < 2)
	{
		sceneCapture->HiddenActors.AddUnique(this);
		sceneCapture->HiddenActors.AddUnique(OtherPortal);
		if (mesh)
		{
			sceneCapture->HideComponent(mesh);
		}
		if (OtherPortal->mesh)
		{
			sceneCapture->HideComponent(OtherPortal->mesh);
		}
	}

	FVector Location = this->GetActorLocation() - OtherPortal->GetActorLocation();

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (CameraManager && CameraManager->GetTransformComponent())
	{
		FVector camLocation = CameraManager->GetTransformComponent()->GetComponentLocation();
		FRotator camRotation = CameraManager->GetTransformComponent()->GetComponentRotation();

		FVector CombinedLocation = camLocation + Location;

		sceneCapture->SetWorldLocationAndRotation(CombinedLocation, camRotation);
	}
}
