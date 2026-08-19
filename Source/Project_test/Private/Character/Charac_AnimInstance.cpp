// Fill out your copyright notice in the Description page of Project Settings.


#include "Charac_AnimInstance.h"
#include"MyCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Project_test/Private/Weapons/Weapon.h"
#include "CharacterTypes/CombatState.h"

void UCharac_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner()); //getting the character from pawn owner 
}

void UCharac_AnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MyCharacter == nullptr)
	{
		MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());//getting the character from pawn owner
	}
	if (MyCharacter == nullptr)
	{
		return;
	}
	FVector Velocity = MyCharacter->GetVelocity(); //character velocity
	Velocity.Z = 0.f;
	speed = Velocity.Size();


	bIsInAIR = MyCharacter->GetCharacterMovement()->IsFalling();


	bIsAccelerating = MyCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false; //if current acceleration size is graeter than 0 then return true else false  

	bWeaponEquipped = MyCharacter->IsWeaponEquipped();

	EquippedWeapon = MyCharacter->GetEquippedWeapon();

	bIsCrouched = MyCharacter->bIsCrouched;

	bAiming = MyCharacter->IsAiming();

	TurningInPlace = MyCharacter->GetTurningInPlace();

	bRotateRootBone = MyCharacter->ShouldRotateRootBone();

	bElimmed = MyCharacter->IsElimmed();


	// Offset Yaw for Strafing
	FRotator AimRotation = MyCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MyCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MyCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);


	AO_Yaw = MyCharacter->GetAO_Yaw();
	AO_Pitch = MyCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MyCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MyCharacter->GetMesh()->TransformToBoneSpace(FName("Hand_R"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		/*
		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector Muzzlex(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + Muzzlex * 1000.f, FColor::Red);

		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MyCharacter->GetHitTarget(), FColor::Blue);
		*/

		if (MyCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			
			FTransform RightHandTransform = MyCharacter->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			//RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyCharacter->GetHitTarget()));
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
		
	}
	bUseFABRIK = MyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;

	bUseAimOffsets = MyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !MyCharacter->GetDisableGameplay();

	bTransformRightHand = MyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !MyCharacter->GetDisableGameplay();
}
