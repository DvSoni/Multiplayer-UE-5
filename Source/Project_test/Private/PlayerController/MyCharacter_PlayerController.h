// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyCharacter_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AMyCharacter_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	void SetHUDHealth(float Health, float MaxHealth); // access the Hud and get the character overlay so we can change health percentage and health text
	
	void SetHUDScore(float Score);			//similar to SetHUDHealth
	void SetHUDDefeats(int32 Defeats);

	void SetHUDWeaponAmmo(int32 Ammo);
	
	void SetHUDCarriedAmmo(int32 Ammo);

	virtual void OnPossess(APawn* InPawn) override; // we have access to the awn that being possessed

	void SetHUDMatchCountdown(float CountdownTime);	 //	match countdown for starting 

	void SetHUDAnnouncementCountdown(float CountdownTime);	// announcement of time before the game mode starts 

	void SetHUDGrenades(int32 Grenades);
	
	virtual void Tick(float DeltaTime) override;


	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnMatchStateSet(FName State);

	void HandleMatchHasStarted();

	void HandleCooldown(); 

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

protected:

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	void SetHUDTime();


	/**
	* Sync time between client and server
	*/

	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);


	void PollInit();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown,float StartingTime); // joining he client on mid game 

	void ShowReturnToMainMenu();// here we create the widget and call its widget function 

	UFUNCTION(Client, Reliable)
		void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

private:

	/**
	* Return to main menu
	*/

	UPROPERTY(EditAnywhere, Category = HUD)
		TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
		class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;


	UPROPERTY()
	class AMyCh_HUD* MyCh_HUD;

	UPROPERTY()
	float MatchTime = 0.f;		// 120 secs , 2mins total

	uint32 CountdownInt = 0;

	float LevelStartingTime = 0.f;
	
	float WarmupTime = 0.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY()
	class ADeathLevelGameMode* DeathLevelGameMode;


	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;

	UPROPERTY()
	float CooldownTime = 0.f;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;


	
};
