// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathLevelGameMode.h"
#include "Character/MyCharacter.h"
#include "PlayerController/MyCharacter_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include"PlayerState/MyCharacterPlayerState.h"
#include "GameState/ProjectGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ADeathLevelGameMode::ADeathLevelGameMode()
{
	bDelayedStart = true;

}


void ADeathLevelGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ADeathLevelGameMode::OnMatchStateSet()
{

	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMyCharacter_PlayerController* MyCharacter_PlayerController = Cast<AMyCharacter_PlayerController>(*It);

		if (MyCharacter_PlayerController)
		{
			MyCharacter_PlayerController->OnMatchStateSet(MatchState);
		}
	}


}


void ADeathLevelGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
		/*	UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;
				World->ServerTravel(FString("/Game/Maps/Deathlevel?listen"));
			}
		*/
			RestartGame();
		}
	}
}

void ADeathLevelGameMode::PlayerEliminated(AMyCharacter* ElimmedCharacter, AMyCharacter_PlayerController* VictimController, AMyCharacter_PlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	AMyCharacterPlayerState* AttackerPlayerState = AttackerController ? Cast<AMyCharacterPlayerState>(AttackerController->PlayerState) : nullptr;
	AMyCharacterPlayerState* VictimPlayerState = VictimController ? Cast<AMyCharacterPlayerState>(VictimController->PlayerState) : nullptr;

	AProjectGameState* ProjectGameState = GetGameState<AProjectGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && ProjectGameState)
	{
		AttackerPlayerState->AddToScore(1.f);

		ProjectGameState->UpdateTopScore(AttackerPlayerState);

	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMyCharacter_PlayerController* MyCharacter_PlayerController = Cast<AMyCharacter_PlayerController>(*It);
		if (MyCharacter_PlayerController && AttackerPlayerState && VictimPlayerState)
		{
			MyCharacter_PlayerController->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ADeathLevelGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter) // if ElimmedCharacter is valid 
	{
		ElimmedCharacter->Reset();		// reset is inherited function and it is on pawn class // And also reset will unposes character and we can reposes the character 
		ElimmedCharacter->Destroy(); // destroy the character 
	}
	if (ElimmedController) // if ElimmedController is valid 
	{
		UE_LOG(LogTemp, Warning, TEXT("ElimmedController valid"));
		TArray<AActor*> PlayerStarts;	
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);	
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1); // get player Start in world at random 	
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]); // restart player at the player start 
	}
}

