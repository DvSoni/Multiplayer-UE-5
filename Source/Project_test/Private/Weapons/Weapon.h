// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX"),

};


UCLASS()
class AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override; //Overiding from MyCharacter.h class
	virtual void Fire(const FVector& HitTarget);
	
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();

	/**
	* Enable or disable custom depth
	*/
	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();

	virtual void OnEquipped();
	virtual void OnDropped();

	virtual void OnEquippedSecondary();

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverLappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromsweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
		void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);


private:
	UPROPERTY(VisibleAnywhere,Category="Weapon Properties" )
	USkeletalMeshComponent* WeaponMesh; 


	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere; //it is a sphere to pick up weapon when player is near.

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState ,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;


	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ABulletShell_Casing> CasingClass;
	

	/*
		Ammo 
	*/

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
		int32 Ammo;							

	UFUNCTION()
		void OnRep_Ammo();

	void SpendRound();		// ammo round 

	UPROPERTY(EditAnywhere)
		int32 MagCapacity;		//mag capacity different on different weapon

	UPROPERTY()
	class AMyCharacter* MyOwnerCharacter;


	UPROPERTY()
		class AMyCharacter_PlayerController* MyCharacterOwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:

	void Dropped();
	/*
	Texture for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = CrossHairs)
		class	UTexture2D* CrosshairsCenter;			//center dot of the crosshair

	UPROPERTY(EditAnywhere, Category = CrossHairs)
		class	UTexture2D* CrosshairsLeft;				//Left side part of crosshair from center

	UPROPERTY(EditAnywhere, Category = CrossHairs)
		class	UTexture2D* CrosshairsRight;			//Right side part of crosshair from center

	UPROPERTY(EditAnywhere, Category = CrossHairs)
		class	UTexture2D* CrosshairsTop;				//Top side part of crosshair from center

	UPROPERTY(EditAnywhere, Category = CrossHairs)
		class	UTexture2D* CrosshairsBottom;			//Bottom side part of crosshair from center


	/*
	
			Zoomed FOV (Field of View) while aiming
	
	*/
	UPROPERTY(EditAnywhere)
	float ZoomedFOV= 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;


	/**
	* Automatic fire
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .1f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	void AddAmmo(int32 AmmoToAdd);

public:
	void SetWeaponState(EWeaponState State);

	FORCEINLINE  USphereComponent* GetAreaSphere() const { return AreaSphere; }    //getter for the AreaSphere which is private

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; } // getter function

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }	// getter function

	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }	// getter function


	bool IsEmpty();	//	magazine empty when 0 bullet 
	bool IsFull(); // magazine is full 
	FORCEINLINE	EWeaponType GetWeaponType() const { return WeaponType; }	// getter function easily get wepon type from ur weapon 

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;
};

