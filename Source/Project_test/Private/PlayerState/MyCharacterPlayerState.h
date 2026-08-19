// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyCharacterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class AMyCharacterPlayerState : public APlayerState
{
	GENERATED_BODY()

	
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/**
	* Replication notifies
	*/
	virtual void OnRep_Score() override;

	UFUNCTION()
		virtual void OnRep_Defeats();

	void AddToScore(float ScoreAmount);

	void AddToDefeats(int32 DefeatsAmount);

private:

	UPROPERTY()
	class AMyCharacter* Character;
		
	UPROPERTY()
	class AMyCharacter_PlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
		int32 Defeats;

};
