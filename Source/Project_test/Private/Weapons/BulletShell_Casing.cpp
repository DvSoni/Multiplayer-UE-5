// Fill out your copyright notice in the Description page of Project Settings.
#include "BulletShell_Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
// Sets default values
ABulletShell_Casing::ABulletShell_Casing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // camera does not collide with the shell 

	CasingMesh->SetSimulatePhysics(true); // apply physics to the casing mesh
	CasingMesh->SetEnableGravity(true); // apply the gravity to the casing mesh
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse=10.f;




}

// Called when the game starts or when spawned
void ABulletShell_Casing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMesh->OnComponentHit.AddDynamic(this, &ABulletShell_Casing::OnHit); // binding component to OnHit()
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);//Add Impulse to our mesh sot it's flys out 


}

void ABulletShell_Casing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	Destroy();
}


