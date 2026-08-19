// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "GameFramework/SpringArmComponent.h" //see in ue document site..
#include "Camera/CameraComponent.h" //import camera libraries
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Project_test/Private/Weapons/Weapon.h"
#include"Project_test/Private/MyCharacter_Components/CombatComponent.h"
#include "Components/CapsuleComponent.h "
#include "Kismet/KismetMathLibrary.h"
#include "Charac_AnimInstance.h"
#include "Project_test/Project_test.h" // for skeletalmesh not in the collsion channel 
#include "PlayerController/MyCharacter_PlayerController.h"
#include "GameMode/DeathLevelGameMode.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"
#include "PlayerState/MyCharacterPlayerState.h"
#include "Weapons/WeaponTypes.h"
#include"MyCharacter_Components/CombatComponent.h"	
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;	// sometime the projetile from weapon are not firing from muzzle socket so to prevent that we refresh every bones at tick 

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // it is also available in character blueprint

	//Camera Code
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh()); //camera will attach to the roots mesh (capsule)
	CameraBoom->TargetArmLength = 600.f; 
	CameraBoom->bUsePawnControlRotation = true; // rotate camera along with controller
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

    //Controller
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	//Overhead Widget
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	//Combat Mechanism
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	//Crouch movemtn
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 800.f);

	//capsule Component
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}
void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMyCharacter, OverLappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AMyCharacter, Health); // health will be replicated 
	DOREPLIFETIME(AMyCharacter, bDisableGameplay); 

}
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		//MyCharacter_PlayerController->SetHUDHealth(Health, MaxHealth);
		OnTakeAnyDamage.AddDynamic(this, &AMyCharacter::ReceiveDamage); // taking damage 
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
	
}
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 
	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();

	PollInit();
	
}

void AMyCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled()) //if role is greater than simulated proxy (by peking the ENetRole , the role represent as a int value )
	{
		//use aimoffset function if we are not a simulated proxy
		AimOffset(DeltaTime); // call this function Every frame
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}

}

void AMyCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void AMyCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//for jump which is in ACharacter file 
	PlayerInputComponent->BindAction(TEXT("Jump"),IE_Pressed, this, &AMyCharacter::Jump);

	//binding with player input component ..
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AMyCharacter::MoveForward); //binding with the button pressed from the player in unreal engine 5 ->project Settings ->Input
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMyCharacter::MoveRight);			//					||
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AMyCharacter::Turn);		
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AMyCharacter::LookUp);  //binding with the button pressed from the player in unreal engine 5 ->project Settings ->Input
	
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMyCharacter::EquipButtonPressed);//binding with the button pressed from the player in unreal engine 5 ->project Settings ->Input
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyCharacter::CrouchButtonPressed); //				||


	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMyCharacter::AimButtonPressed); // Aim button pressed   binding with the button pressed from the player in unreal engine 5 ->project Settings ->Input
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMyCharacter::AimButtonReleased);// Aim button released   			||

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyCharacter::FireButtonPressed); // fire button pressed    binding with the button pressed from the player in unreal engine 5 ->project Settings ->Input
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMyCharacter::FireButtonReleased); // fire button released  /		||

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMyCharacter::ReloadButtonPressed); // Reload button pressed    binding with the button pressed from the player in unreal engine 5 ->project Settings ->Input
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &AMyCharacter::GrenadeButtonPressed); // ThrowGrenade (T) button pressed    binding with the button pressed from the player in unreal engine 5 ->project Settings ->Input




}

void AMyCharacter::PostInitializeComponents()
{
	
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AMyCharacter::PlayFireMontage(bool bAiming)
{

	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyCharacter::PlayElimMontage()
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	char* Death[5] = {"Death1","Death2","Death3" ,"Death4" ,"Death5" }; //character pointer for storing name of multiple death animation
	
	int ra = FMath::RandRange(0, 4); // selecting random animation from the above


	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
		FName SectionName(Death[ra]);	
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


void AMyCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)  // if both the player equip weapon then only the montage will play 
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
		//UE_LOG(LogTemp, Warning, TEXT("HiTReactmontage....."));
	}

}

void AMyCharacter::PlayReloadMontage()	// reload animation function 
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubMachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;

		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);

	}
}


void AMyCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f) //if player(controller) is not null
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f) //if player(controller) is not null
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMyCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMyCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;
		if (Combat)
		{
			ServerEquippedButtonPressed();
			
		}

	
}
void AMyCharacter::ServerEquippedButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverLappingWeapon)
		{
			Combat->EquipWeapon(OverLappingWeapon);
		}
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
		
	}
}

void  AMyCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}

}

void AMyCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}

}

void AMyCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}


}
float AMyCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}
void AMyCharacter::AimOffset(float DeltaTime)
{

	if (Combat && Combat->EquippedWeapon == nullptr) return; // not equipped weapon then return
	float Speed = CalculateSpeed();
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;

	//float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;

		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAo_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		bRotateRootBone = false;

		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pitch();
}

void AMyCharacter::ReloadButtonPressed()
{

	if (Combat)
	{
		Combat->Reload();
	}

}

void AMyCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}

}

void AMyCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}


void AMyCharacter::Jump()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AMyCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)  // if in combat fire button is pressed then true
	{
		Combat->FireButtonPressed(true);
	}

}

void AMyCharacter::FireButtonReleased()
{

	if (bDisableGameplay) return;
	if (Combat)		// if in combat fire button is released then false
	{
		Combat->FireButtonPressed(false);
	}

}

void AMyCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimmed) return;
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f) // if health gets 0  
	{
		ADeathLevelGameMode* DeathLevelGameMode = GetWorld()->GetAuthGameMode<ADeathLevelGameMode>();	//get the game mode 
		if (DeathLevelGameMode) // is valid
		{
			MyCharacter_PlayerController = MyCharacter_PlayerController == nullptr ? Cast<AMyCharacter_PlayerController>(Controller) : MyCharacter_PlayerController;
			AMyCharacter_PlayerController* AttackerController = Cast<AMyCharacter_PlayerController>(InstigatorController);
			DeathLevelGameMode->PlayerEliminated(this, MyCharacter_PlayerController, AttackerController);
		}
	}


}

void AMyCharacter::UpdateHUDHealth()
{
	MyCharacter_PlayerController = MyCharacter_PlayerController == nullptr ? Cast<AMyCharacter_PlayerController>(Controller) : MyCharacter_PlayerController;
	if (MyCharacter_PlayerController)
	{
		MyCharacter_PlayerController->SetHUDHealth(Health, MaxHealth);
	}

}

void AMyCharacter::PollInit()
{
	if (MyCharacterPlayerState == nullptr)
	{
		MyCharacterPlayerState = GetPlayerState<AMyCharacterPlayerState>();
		if (MyCharacterPlayerState)
		{
			MyCharacterPlayerState->AddToScore(0.f);
			MyCharacterPlayerState->AddToDefeats(0);
		}
	}
	if (MyController == nullptr)
	{
		MyController = MyController == nullptr ? Cast<AMyCharacter_PlayerController>(Controller) : MyController;

		if (MyController)
		{
			SpawnDefaultWeapon();
			UpdateHUDAmmo();
			UpdateHUDHealth();
		}
	}
}


void AMyCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAo_Yaw = FMath::FInterpTo(InterpAo_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAo_Yaw;
		if (FMath::Abs(AO_Yaw)<15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}

}
void AMyCharacter::HideCameraIfCharacterClose() // hide camera when character is near to wall or something blocking it	
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}

}

void AMyCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();

}

 void AMyCharacter::UpdateHUDAmmo()
{

	MyCharacter_PlayerController = MyCharacter_PlayerController == nullptr ? Cast<AMyCharacter_PlayerController>(Controller) : MyCharacter_PlayerController;
	if (MyCharacter_PlayerController && Combat && Combat->EquippedWeapon)
	{
		MyCharacter_PlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		MyCharacter_PlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}

}

void AMyCharacter::SpawnDefaultWeapon()
{
	ADeathLevelGameMode* DeathLevelGameMode = Cast<ADeathLevelGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (DeathLevelGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);	// spawn an actor based of default weapon class
		StartingWeapon->bDestroyWeapon = true;

		//Equip Weapon is function(public function) in CombatComponent and we can access because the MyCharacter is a freind class of CombatCopnent
		if (Combat) 
		{
			Combat->EquipWeapon(StartingWeapon);	
		}
	}


}

void AMyCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	
		if (OverLappingWeapon)
		{
			OverLappingWeapon->ShowPickupWidget(false);
		}
		OverLappingWeapon = Weapon;
		if (IsLocallyControlled())
		{
			if (OverLappingWeapon)
			{
				OverLappingWeapon->ShowPickupWidget(true);
			}
		}
	
}


void AMyCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{

	if (OverLappingWeapon)
	{
		OverLappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AMyCharacter::OnRep_ReplicatedMovement()
{

	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AMyCharacter::Elim()
{
	DropOrDestroyWeapons();

	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer, //timer handle
		this,				// user object
		&AMyCharacter::ElimTimerFinished,
		ElimDelay // delay time 
	);

}
void AMyCharacter::ElimTimerFinished()
{
	ADeathLevelGameMode* DeathLevelGameMode = GetWorld()->GetAuthGameMode<ADeathLevelGameMode>();	//get the game mode 

	if (DeathLevelGameMode)
	{
		DeathLevelGameMode->RequestRespawn(this, Controller);
	}

}

void AMyCharacter::MulticastElim_Implementation() // on the server
{

	if (MyCharacter_PlayerController)
	{
		MyCharacter_PlayerController->SetHUDWeaponAmmo(0);
	}

	bElimmed = true;
	PlayElimMontage();

	bool DissolveMaterialInst = DissolveMaterialInstance1 && 
								DissolveMaterialInstance2 && 
								DissolveMaterialInstance3 && 
								DissolveMaterialInstance4 && 
								DissolveMaterialInstance5 && 
								DissolveMaterialInstance6;
	if (DissolveMaterialInst)
	{
		// Start Dissolve effect 
		DissolveEffectFun();
	}

	StartDissolve();

	/*

		// Disable character movement
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();

		if (MyCharacter_PlayerController)
		{
			DisableInput(MyCharacter_PlayerController);
		}
	*/

	// Disable Character Movement
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();//prevent us from flling through the floor 
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
}

void AMyCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	bool DynamicDissMaterialInst = DynamicDissolveMaterialInstance1 && 
									DynamicDissolveMaterialInstance2 && 
									DynamicDissolveMaterialInstance3 && 
									DynamicDissolveMaterialInstance4 && 
									DynamicDissolveMaterialInstance5 && 
									DynamicDissolveMaterialInstance6;
	if (DynamicDissMaterialInst)
	{
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance4->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance5->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance6->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	
	}
}

void AMyCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AMyCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}

}



void AMyCharacter::DissolveEffectFun()
{
	DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance1, this); // create the dyanmic instance of material 
	GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance1);	// set the dyanmic instance of material to skeletal mesh index 0 (where the normal material index )
	DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), -0.7f);	// set the dyanmic instance of material parameter Dissolve to -0.78 (not dissolve value)
	DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Glow"), 500.f);			// set the dyanmic instance of material parameter Glow to 500.f(brightness of glow )


	// UE_LOG(LogTemp,Warning,TEXT("DissolveMaterialInstance2 ..........."));
	DynamicDissolveMaterialInstance2 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance2, this); // create the dyanmic instance of material 
	GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance2);	// set the dyanmic instance of material to skeletal mesh index  (where the normal material index )
	DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"), -0.7f);	// set the dyanmic instance of material parameter Dissolve to -0.78 (not dissolve value)
	DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Glow"), 500.f);			// set the dyanmic instance of material parameter Glow to 500.f(brightness of glow )

	DynamicDissolveMaterialInstance3 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance3, this); // create the dyanmic instance of material 
	GetMesh()->SetMaterial(2, DynamicDissolveMaterialInstance3);	// set the dyanmic instance of material to skeletal mesh index  (where the normal material index )
	DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Dissolve"), -0.7f);	// set the dyanmic instance of material parameter Dissolve to -0.78 (not dissolve value)
	DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Glow"), 500.f);			// set the dyanmic instance of material parameter Glow to 500.f(brightness of glow )


	DynamicDissolveMaterialInstance4 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance4, this); // create the dyanmic instance of material 
	GetMesh()->SetMaterial(3, DynamicDissolveMaterialInstance4);	// set the dyanmic instance of material to skeletal mesh index  (where the normal material index )
	DynamicDissolveMaterialInstance4->SetScalarParameterValue(TEXT("Dissolve"), -0.7f);	// set the dyanmic instance of material parameter Dissolve to -0.78 (not dissolve value)
	DynamicDissolveMaterialInstance4->SetScalarParameterValue(TEXT("Glow"), 500.f);			// set the dyanmic instance of material parameter Glow to 500.f(brightness of glow )


	DynamicDissolveMaterialInstance5 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance5, this); // create the dyanmic instance of material 
	GetMesh()->SetMaterial(4, DynamicDissolveMaterialInstance5);	// set the dyanmic instance of material to skeletal mesh index  (where the normal material index )
	DynamicDissolveMaterialInstance5->SetScalarParameterValue(TEXT("Dissolve"), -0.7f);	// set the dyanmic instance of material parameter Dissolve to -0.78 (not dissolve value)
	DynamicDissolveMaterialInstance5->SetScalarParameterValue(TEXT("Glow"), 500.f);			// set the dyanmic instance of material parameter Glow to 500.f(brightness of glow )


	DynamicDissolveMaterialInstance6 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance6, this); // create the dyanmic instance of material 
	GetMesh()->SetMaterial(5, DynamicDissolveMaterialInstance6);	// set the dyanmic instance of material to skeletal mesh index  (where the normal material index )
	DynamicDissolveMaterialInstance6->SetScalarParameterValue(TEXT("Dissolve"), -0.7f);	// set the dyanmic instance of material parameter Dissolve to -0.78 (not dissolve value)
	DynamicDissolveMaterialInstance6->SetScalarParameterValue(TEXT("Glow"), 500.f);			// set the dyanmic instance of material parameter Glow to 500.f(brightness of glow )

}


bool AMyCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon); //if combat is valid & combat equipped weapon return true 
}

bool AMyCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}


AWeapon* AMyCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AMyCharacter::GetHitTarget() const
{
	
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState AMyCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;

}

void AMyCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false; //for sim proxy turn we set bRotateRootBone to false
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

//	UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

}

