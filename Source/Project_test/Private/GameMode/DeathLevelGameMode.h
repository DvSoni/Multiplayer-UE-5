// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DeathLevelGameMode.generated.h"


namespace MatchState
{
	extern const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

/**
 * 
 */
UCLASS()
class ADeathLevelGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	ADeathLevelGameMode();

	virtual void Tick(float DeltaTime) override;

	virtual void PlayerEliminated(class AMyCharacter* ElimmedCharacter,class AMyCharacter_PlayerController* VictimController, AMyCharacter_PlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 5.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	
	float LevelStartingTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override; // already in gamemode internal file so we override

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
