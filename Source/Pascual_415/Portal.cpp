// fill out your copyright notice in the Description page of Project Settings.

#include "Portal.h"
#include "Pascual_415Character.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/TextureRenderTarget2D.h"

// set up all the components, attach them together, and make sure the portal doesn't cast shadows or block the player
APortal::APortal()
{
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

// apply our material and render target directly in the editor so we don't have to hit play to see it
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

// hook up the overlap trigger and hide both portals from the camera so they don't block the view
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

// update the portal camera position every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePortals();
}

// when the player walks into the portal, teleport them to the other portal's arrow and start a 1 second cooldown
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

// reset the teleport cooldown so the player can use portals again
void APortal::SetBool(APascual_415Character* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		PlayerCharacter->isTeleporting = false;
	}
}

// move and rotate the camera at the other portal to match our player's viewpoint so it looks like a real window
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
