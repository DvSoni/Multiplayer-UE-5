// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Project_test/Private/Character/MyCharacter.h"
#include "Project_test/Private/Character/MyCharacter.h"
#include "Project_test/Project_test.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	//collision settings for our box component
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);//weapon fire will flying through the air
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //hit events
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//does not overlap or hit everything 
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); //block from the wall, surface etc
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block); 
	//CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block); 
	
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block); // for skeletal mesh which is defined in project_test.h file


}


// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (Tracer) // if tracer is attached to our root component our collision box
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached
		(
			Tracer,
			CollisionBox,
		FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HasAuthority()) // check hasAuthority then call the function Multicast_OnHit()
	{
		Multicast_OnHit();
	}
	Destroy(); // destroying the projectile action
}

void AProjectile::Multicast_OnHit_Implementation()
{
	if (ImpactParticles) // check is valid
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform()); // spawning emiter in the world 
	}
	if (ImpactSound) // check is valid
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation()); // spawning sound on impact 
	}
}


void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)//spawning the niagara system 
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);

	}

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}



void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());	
	}
}

void AProjectile::ExplodeDamage()
{
	APawn* Firingpawn = GetInstigator();	//	GetInstigator()	returns the pawn that owns the weapon and fire this rocket

	if (Firingpawn && HasAuthority()) // is valid 
	{
		AController* FiringController = Firingpawn->GetController();  //Get Controller and store in variable 

		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,	//World Context Object
				Damage,	//base Damage
				10.f,//minimum damage outside the radius of rocket hit on other payer 
				GetActorLocation(), // origin of the radius (the damage of inner and outer radii will center around this )
				InnerRadius,
				OuterRadius,
				1.f, // means damage will decrease steadily for actor the farther away they are from the inner radius and that damage decrease will be linear 
				UDamageType::StaticClass(),	//Damage Type Class
				TArray<AActor*>(),	//empty array because the character who fire rocket can also get damage if it is near (we can include in array so , this rocket does not damage the character that fired it  )
				this,//Damage Causer.
				FiringController // InstigatorController 	
			);
		}
	}
}


