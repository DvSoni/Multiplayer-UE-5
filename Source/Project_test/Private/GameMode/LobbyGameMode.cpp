// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{

	Super::PostLogin(NewPlayer);

	//Getstate == gamestate has array of player state
	int32 NumberofPlayers = GameState.Get()->PlayerArray.Num();
	if (NumberofPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/Deathlevel?listen"));
		}
	}
}