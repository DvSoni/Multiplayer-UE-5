// Fill out your copyright notice in the Description page of Project Settings.
#include "Weapons/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Project_test/Private/Character/MyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "BulletShell_Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PlayerController/MyCharacter_PlayerController.h"
#include "MyCharacter_Components/CombatComponent.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true); // replicate the movement

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//when player drop collide with surface and little bit bounce
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Ignore); // when player walk on dropped weapon it does not collide with weapon
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//when we walk up and pick the weapon there will no collision

	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_Yellow);		// weapon outline color 
	WeaponMesh->MarkRenderStateDirty();

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere")); //areashpere is the sphere from which player can picup weapon
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//ignore when player is near to the area sphere
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //ignore when player collide to the area sphere

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	}
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}


// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}

}
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);

}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject")); // check all the weapon consist AmmoEject socket 
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ABulletShell_Casing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
					);
			}
		}
	}
	SpendRound();
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromsweep, const FHitResult& SweepResult)
{

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(OtherActor);
	if (MyCharacter)
	{
		MyCharacter->SetOverlappingWeapon(this);

	}

}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(OtherActor);
	if (MyCharacter)
	{
		MyCharacter->SetOverlappingWeapon(nullptr);
	}

}


void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::SetWeaponState(EWeaponState State)
{

	WeaponState = State;
	OnWeaponStateSet();

}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped: // weapon equipped
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}
void AWeapon::OnRep_WeaponState() // to replicate the properties of weapon equipped
{
	OnWeaponStateSet();
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);// does not the widget(E-Equip)
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);//does not collide wihe AreaSphere
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);

}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//	1		something bad happens comment this 3 lines
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);	//	2
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);	//	3

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_Yellow);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

}
void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);// does not the widget(E-Equip)
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);//does not collide wihe AreaSphere
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	EnableCustomDepth(true);
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}

}
bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}
void AWeapon::OnRep_Ammo()
{

	MyOwnerCharacter = MyOwnerCharacter == nullptr ? Cast<AMyCharacter>(GetOwner()) : MyOwnerCharacter;
	if (MyOwnerCharacter && MyOwnerCharacter->GetCombat() && IsFull())
	{
		MyOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();

}

void AWeapon::SetHUDAmmo()
{
	MyOwnerCharacter = MyOwnerCharacter == nullptr ? Cast<AMyCharacter>(GetOwner()) : MyOwnerCharacter;
	if (MyOwnerCharacter)
	{
		MyCharacterOwnerController = MyCharacterOwnerController == nullptr ? Cast<AMyCharacter_PlayerController>(MyOwnerCharacter->Controller) : MyCharacterOwnerController;
		if (MyCharacterOwnerController)
		{
			MyCharacterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}


void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();

}
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		MyOwnerCharacter = nullptr;
		MyCharacterOwnerController = nullptr;
	}
	else
	{
		MyOwnerCharacter = MyOwnerCharacter == nullptr ? Cast<AMyCharacter>(Owner) : MyOwnerCharacter;
		if(MyOwnerCharacter && MyOwnerCharacter->GetEquippedWeapon() && MyOwnerCharacter->GetEquippedWeapon()== this)
		{
			SetHUDAmmo();
		}
		
	}
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);

	MyOwnerCharacter = nullptr;					// weapon dropped
	MyCharacterOwnerController = nullptr;		
}


void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}


