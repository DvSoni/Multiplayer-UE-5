// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Project_test/Private/HUD/MyCh_HUD.h"
#include "Weapons/WeaponTypes.h"
#include "CharacterTypes/CombatState.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

	friend class AMyCharacter;

	void EquipWeapon(class AWeapon* WeaponToEquip);

	void SwapWeapons();

	void Reload(); //for Reload 

	UFUNCTION(BlueprintCallable)
		void FinishReloading();

	void FireButtonPressed(bool bPressed); // function when we pressed fire button

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bIsAiming); //Remote procedure call (RPC) getting info from clent to server

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void Fire();

	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);				//Server RPC(Remote Procedure Calls)

	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);		//Server RPC(Remote Procedure Calls)

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);		//hit results(information) and trace of the gun fire 

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;


	void DropEquippedWeapon();

	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);

	void AttachActorToSecondarySocket(AActor* ActorToAttach);	// attach the secondary weapon on the player 

	void UpdateCarriedAmmo();

	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);

	void ReloadEmptyWeapon();

	void ShowAttachedGrenade(bool bShowGrenade);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);	// for primary weapon 
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);	// for secondary weapon when equip

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;	

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	
private:
	UPROPERTY()
	class AMyCharacter_PlayerController* Controller;
	
	UPROPERTY()
	class AMyCharacter* Character;

	UPROPERTY()
	class AMyCh_HUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)	
	AWeapon* EquippedWeapon;	// replicated 

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;	// Secondary weapon replicated 

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
		float BaseWalkSpeed;  // basic walkspeed when no aiming

	UPROPERTY(EditAnywhere)
		float AimWalkSpeed; // less walkspeed when the player is aiming


	bool bFireButtonPressed;


	FHUDPackage HUDPackage;
	/**
	* HUD and crosshairs
	*/

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;	// for shrink when aiming
	float CrosshairShootingFactor; 
		
	FVector HitTarget; 

	/*
	
		Aiming And FOV (Field of View)
	*/


	//FOV (field of view) when not aiming;set to the camera's base FOV in begin play  
	float DefaultFOV;


	UPROPERTY(EditAnywhere,Category=Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	float CurrentFOV;

	void InterpFOV(float DeltaTime); // interpolation function for interpolating the field of View (FOV)


	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire(); //if we have more than 0 ammo then only fire 

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
		int32 CarriedAmmo;

	UFUNCTION()
		void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
		int32 MaxCarriedAmmo = 300;

	UPROPERTY(EditAnywhere)
		int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
		int32 StartingRocketAmmo = 2;

	UPROPERTY(EditAnywhere)
		int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
		int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere)
		int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditAnywhere)
		int32 StartingSniperRifleAmmo = 0;

	UPROPERTY(EditAnywhere)
		int32 StartingGrenadeLauncherAmmo = 0;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
		void OnRep_CombatState();

	void UpdateAmmoValues();

	void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 5;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 5;

	void UpdateHUDGrenades();


public:
		FORCEINLINE int32 GetGrenades() const { return Grenades; }
		bool ShouldSwapWeapons();
};
