// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	UPROPERTY(EditAnywhere)
		float InnerRadius = 250.f;

	UPROPERTY(EditAnywhere)
		float OuterRadius = 500.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);



	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;  // in protected because we can access in projectilebullet 

	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;		// trail of the rocket system  

	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	void StartDestroyTimer();

	void DestroyTimerFinished();

	void ExplodeDamage();

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* ProjectileMesh; // adding the mesh component to the game 

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;


	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;


private:

		UPROPERTY()
		class UParticleSystemComponent* TracerComponent;

		UFUNCTION(NetMulticast, Reliable)
			void Multicast_OnHit();		//multicast because we can see in the server if the client fires weapon to server or close to the server

		FTimerHandle DestroyTimer;		// destroy time of the trail of rocket 

		UPROPERTY(EditAnywhere)
			float DestroyTime = 3.f;	// after 3 sec it will destroy 
};
