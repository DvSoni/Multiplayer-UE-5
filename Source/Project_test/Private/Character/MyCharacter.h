// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include"Project_test/Private/CharacterTypes/TurningInPlace.h"
#include "Project_test/Private/Interfaces/InteractwithCrosshairs_Interface.h"
#include "Components/TimelineComponent.h"
#include "CharacterTypes/CombatState.h"
#include "MyCharacter.generated.h"


UCLASS()
class AMyCharacter : public ACharacter , public IInteractwithCrosshairs_Interface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);

	void PlayHitReactMontage(); // function when player hit by bullet

	void PlayElimMontage();

	void PlayReloadMontage();

	void PlayThrowGrenadeMontage();

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)   // client side replicating the variable
		class AWeapon* OverLappingWeapon;

	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	virtual void OnRep_ReplicatedMovement() override;

	void Elim();

	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim(); // when player gets eliminated

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Character Movement Function ...
	void MoveForward(float Value); // binding with the funciton in unreal engine 5 ->project Settings ->Input 
	void MoveRight(float Value);
	void Turn(float Value);		//			||
	void LookUp(float Value);
	void EquipButtonPressed();// binding with the funciton in unreal engine 5 ->project Settings ->Input 
	void CrouchButtonPressed();
	void AimButtonPressed();	//			||
	void AimButtonReleased();
	void AimOffset(float DeltaTime);// binding  with the function in unreal engine 5 ->project Settings ->Input 
	void ReloadButtonPressed(); //			||
	void GrenadeButtonPressed();//			||


	void SimProxiesTurn();	//simulated poxy function
	void CalculateAO_Pitch();

	virtual void Jump()override;

	void FireButtonPressed(); // binding with the funciton  
	void FireButtonReleased();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();  // update HUd health 

	// Poll for any relelvant classes and initialize our HUD
	void PollInit();

	void RotateInPlace(float DeltaTime);

	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();

private:
	UPROPERTY(VisibleAnywhere,Category=Camera)
		class USpringArmComponent* CameraBoom;



	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess="true"))
		class UWidgetComponent* OverheadWidget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCombatComponent* Combat;

	UFUNCTION(Server,Reliable)
	void ServerEquippedButtonPressed(); //RPC (Remote Procedure Call) for player equipping weapon 

	float AO_Yaw;
	float InterpAo_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);



	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
		 UAnimMontage* HitReactMontage;


	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* ThrowGrenadeMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
		float CameraThreshold = 200.f;


	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();


	/*
		Player Health 
	*/

	UPROPERTY(EditAnywhere,Category="Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats" );
	float Health = 100.f;			//register the variable for replication 

	UFUNCTION()
	void OnRep_Health();


	UPROPERTY()
	class AMyCharacter_PlayerController* MyCharacter_PlayerController;

	bool bElimmed = false;


	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly) //we can edit ElimDelay only on the character
	float ElimDelay = 3.f;		//time delay of 5 sce after elimination

	void ElimTimerFinished();


	/**
	* Dissolve effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue); // callback function which caleed every rame as we are updating timeline
	void StartDissolve( );

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance1;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance2;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance3;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance4;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance5;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance6;


	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance1;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance2;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance3;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance4;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance5;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance6;

	void DissolveEffectFun();

	UPROPERTY()
	class AMyCharacterPlayerState* MyCharacterPlayerState;
	

	/*
		Grenade
	*/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/*
		Default weapon
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;	// for default weapon calss to spawn

	
public:	

	void UpdateHUDAmmo();

	void SpawnDefaultWeapon();


	UPROPERTY(BlueprintReadOnly, Category = Camera)
		class UCameraComponent* FollowCamera;

	UFUNCTION(BlueprintImplementableEvent) // we can imlement this in blueprint character
	void ShowSniperScopeWidget(bool bShowScope);// to show or hide the scope 

	void SetOverlappingWeapon(AWeapon* Weapon);  //when player overlpp weapon
	bool IsWeaponEquipped(); // if weapon equipped ,it use for the animation pose

	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; } // Getter function
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; } // Getter function

	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; } // Getter function

	FORCEINLINE UCameraComponent* GetFollowCamera()const { return FollowCamera; } //Getter Function

	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	FORCEINLINE bool IsElimmed() const { return bElimmed; }//Getter Function

	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }


	ECombatState GetCombatState() const;
	
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }

	AMyCharacter_PlayerController* MyController;

	FORCEINLINE AMyCharacter_PlayerController* GetMyCharacter_PlayerController() const { return MyController; }
};
