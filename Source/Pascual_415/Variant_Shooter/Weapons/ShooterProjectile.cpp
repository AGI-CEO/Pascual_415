// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

AShooterProjectile::AShooterProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// create the collision component and assign it as the root
	RootComponent = CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));

	CollisionComponent->SetSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	// create the ball mesh component and attach it to collision component
	ballMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball Mesh"));
	ballMesh->SetupAttachment(CollisionComponent);
	ballMesh->SetRelativeScale3D(FVector(0.125f, 0.125f, 0.125f));
	ballMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ballMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// create the projectile movement component. No need to attach it because it's not a Scene Component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.6f;

	// set the default damage type
	HitDamageType = UDamageType::StaticClass();
}

void AShooterProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// ignore the pawn that shot this projectile
	CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);

	// Pick random RGB values between 0 and 1 to make a random color
	float ranNumX = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
	float ranNumY = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
	float ranNumZ = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
	randColor = FLinearColor(ranNumX, ranNumY, ranNumZ, 1.0f);

	// Create a dynamic material instance so we can change colors at runtime
	if (projectileMaterial)
	{
		dmiMat = UMaterialInstanceDynamic::Create(projectileMaterial, this);
	}
	else if (ballMesh && ballMesh->GetMaterial(0))
	{
		dmiMat = ballMesh->CreateDynamicMaterialInstance(0);
	}

	// Apply the random color to the projectile mesh
	if (ballMesh && dmiMat)
	{
		ballMesh->SetMaterial(0, dmiMat);
		dmiMat->SetVectorParameterValue(TEXT("ProjColor"), randColor);
		dmiMat->SetVectorParameterValue(TEXT("Projectile Color"), randColor);
		dmiMat->SetVectorParameterValue(TEXT("Color"), randColor);
	}
}

void AShooterProjectile::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the destruction timer
	GetWorld()->GetTimerManager().ClearTimer(DestructionTimer);
}

void AShooterProjectile::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	// ignore if we've already hit something else
	if (bHit)
	{
		return;
	}

	bHit = true;

	// make AI perception noise
	MakeNoise(NoiseLoudness, GetInstigator(), GetActorLocation(), NoiseRange, NoiseTag);

	// Spawn the splatter decal on the hit surface with the random color
	if (Other != nullptr)
	{
		float FrameNumber = UKismetMathLibrary::RandomFloatInRange(0.0f, 3.0f);

		float DecalSize = UKismetMathLibrary::RandomFloatInRange(20.0f, 40.0f);

		UMaterialInterface* DecalMat = baseMat ? baseMat : LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Splatter/Splat1_MAT.Splat1_MAT"));
		if (DecalMat)
		{
			auto Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), DecalMat, FVector(DecalSize, DecalSize, DecalSize), Hit.Location, Hit.Normal.Rotation(), 0.0f);
			if (Decal)
			{
				auto MatInstance = Decal->CreateDynamicMaterialInstance();
				if (MatInstance)
				{
					MatInstance->SetVectorParameterValue(TEXT("Color"), randColor);
					MatInstance->SetScalarParameterValue(TEXT("Frame"), FrameNumber);
				}
			}
		}
	}

	// Spawn the particle effect at the hit location and set its color
	UNiagaraSystem* ParticleSys = colorP ? colorP : LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Materials/Confetti_P.Confetti_P"));
	if (ParticleSys)
	{
		UNiagaraComponent* particleComp = nullptr;
		if (OtherComp)
		{
			particleComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				ParticleSys, 
				OtherComp, 
				NAME_None, 
				Hit.Location, 
				Hit.Normal.Rotation(), 
				EAttachLocation::KeepWorldPosition, 
				true
			);
		}
		if (!particleComp)
		{
			particleComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), 
				ParticleSys, 
				Hit.Location, 
				Hit.Normal.Rotation(), 
				FVector(1.f), 
				true
			);
		}

		if (particleComp)
		{
			particleComp->SetVariableLinearColor(TEXT("RandColor"), randColor);
			particleComp->SetVariableLinearColor(TEXT("RandomColor"), randColor);
		}
	}

	// Hide and remove the ball mesh so it disappears immediately on impact
	if (ballMesh)
	{
		ballMesh->SetVisibility(false, true);
		ballMesh->SetHiddenInGame(true, true);
		ballMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ballMesh->DestroyComponent();
	}

	// Turn off collision and stop movement so the projectile doesn't bounce
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComponent->SetCollisionProfileName(TEXT("NoCollision"));
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (bExplodeOnHit)
	{
		
		// apply explosion damage centered on the projectile
		ExplosionCheck(GetActorLocation());

	} else {

		// single hit projectile. Process the collided actor
		ProcessHit(Other, OtherComp, Hit.ImpactPoint, -Hit.ImpactNormal);

	}

	// pass control to BP for any extra effects
	BP_OnProjectileHit(Hit);

	// check if we should schedule deferred destruction of the projectile
	if (DeferredDestructionTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AShooterProjectile::OnDeferredDestruction, DeferredDestructionTime, false);

	} else {

		// destroy the projectile right away
		Destroy();
	}
}

void AShooterProjectile::ExplosionCheck(const FVector& ExplosionCenter)
{
	// do a sphere overlap check look for nearby actors to damage
	TArray<FOverlapResult> Overlaps;

	FCollisionShape OverlapShape;
	OverlapShape.SetSphere(ExplosionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (!bDamageOwner)
	{
		QueryParams.AddIgnoredActor(GetInstigator());
	}

	GetWorld()->OverlapMultiByObjectType(Overlaps, ExplosionCenter, FQuat::Identity, ObjectParams, OverlapShape, QueryParams);

	TArray<AActor*> DamagedActors;

	// process the overlap results
	for (const FOverlapResult& CurrentOverlap : Overlaps)
	{
		// overlaps may return the same actor multiple times per each component overlapped
		// ensure we only damage each actor once by adding it to a damaged list
		if (DamagedActors.Find(CurrentOverlap.GetActor()) == INDEX_NONE)
		{
			DamagedActors.Add(CurrentOverlap.GetActor());

			// apply physics force away from the explosion
			const FVector& ExplosionDir = CurrentOverlap.GetActor()->GetActorLocation() - GetActorLocation();

			// push and/or damage the overlapped actor
			ProcessHit(CurrentOverlap.GetActor(), CurrentOverlap.GetComponent(), GetActorLocation(), ExplosionDir.GetSafeNormal());
		}
			
	}
}

void AShooterProjectile::ProcessHit(AActor* HitActor, UPrimitiveComponent* HitComp, const FVector& HitLocation, const FVector& HitDirection)
{
	// have we hit a character?
	if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
	{
		// ignore the owner of this projectile
		if (HitCharacter != GetOwner() || bDamageOwner)
		{
			// apply damage to the character
			UGameplayStatics::ApplyDamage(HitCharacter, HitDamage, GetInstigator()->GetController(), this, HitDamageType);
		}
	}

	// have we hit a physics object?
	if (HitComp->IsSimulatingPhysics())
	{
		// give some physics impulse to the object
		HitComp->AddImpulseAtLocation(HitDirection * PhysicsForce, HitLocation);
	}
}

void AShooterProjectile::OnDeferredDestruction()
{
	// destroy this actor
	Destroy();
}
