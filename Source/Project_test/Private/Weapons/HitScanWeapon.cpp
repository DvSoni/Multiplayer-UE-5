// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/MyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include"WeaponTypes.h"
#include "Kismet/KismetMathLibrary.h"

#include "DrawDebugHelpers.h"

// Hit Sccan Wepon for SMG & Pistol ,Close Range Wepons  (Determining a a HIT with a Line trace rather than projectile)

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());	// casting an Actor to Pawn and then we can get the controller 
 	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	// for perform LineTrace we have to get the muzzle flash socket from the weapon mesh 
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh()); // for socket 

		FVector Start = SocketTransform.GetLocation();		//	Starting point of Line trace
		FHitResult FireHit;			// Hit result 	

		WeaponTraceHit(Start, HitTarget, FireHit);

		AMyCharacter* MyCharacter = Cast<AMyCharacter>(FireHit.GetActor()); // getting the character

		

				if (MyCharacter && HasAuthority() && InstigatorController)
				{
					//	if (FireHit.bBlockingHit) //Check to see if we get the blocking hit 	//apply damage when it hit the character 

					UGameplayStatics::ApplyDamage(
						MyCharacter,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);

				}
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
					UE_LOG(LogTemp, Warning, TEXT("in impactparticles socket"));
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint
					);
				}
				if (MuzzleFlash)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						MuzzleFlash,
						SocketTransform
					);
				}
				if (FireSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						FireSound,
						GetActorLocation()
					);
				}
			
	

	}

}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{

	FHitResult	FireHit;
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f; // Ending point of Line trace 

		World->LineTraceSingleByChannel(
			OutHit,					// Hit Result
			TraceStart,							// Start location of Line Trace
			End,							//End  Location of Line Trace
			ECollisionChannel::ECC_Visibility		
		);

		FVector BeamEnd = End;

		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}

}


FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();	//vector pointing fromtrace start location to hittarget
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	/*
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true);
	*/
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

