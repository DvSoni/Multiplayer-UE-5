// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterPlayerState.h"
#include "Character/MyCharacter.h"
#include "PlayerController/MyCharacter_PlayerController.h"
#include "Net/UnrealNetwork.h"



void AMyCharacterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyCharacterPlayerState, Defeats);
}



void AMyCharacterPlayerState::AddToScore(float ScoreAmount)
{

	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AMyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMyCharacter_PlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMyCharacterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AMyCharacter>(GetPawn()) : Character;
	if (Character )
	{
		Controller = Controller == nullptr ? Cast<AMyCharacter_PlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
void AMyCharacterPlayerState::OnRep_Score()
{

	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AMyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMyCharacter_PlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}


	

}

void AMyCharacterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AMyCharacter>(GetPawn()) : Character;
	if (Character ) //&& Character->Controller
	{
		Controller = Controller == nullptr ? Cast<AMyCharacter_PlayerController>(Character->Controller) : Controller;
	
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
		if (Controller == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Game Crash reason ............"))
		}
	}
}
