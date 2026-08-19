// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter_PlayerController.h"

#include"HUD/CharacterOverlay.h"
#include "HUD/MyCh_HUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/MyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/DeathLevelGameMode.h"
#include "PlayerState/MyCharacterPlayerState.h"
#include "HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "MyCharacter_Components/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "GameState/ProjectGameState.h"
#include "HUD/ReturnToMainMenu.h"
void AMyCharacter_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	MyCh_HUD = Cast<AMyCh_HUD>(GetHUD()); // access the Hud , GetHud()= return a HUd. 

	ServerCheckMatchState();

}

void AMyCharacter_PlayerController::SetupInputComponent()
{

	Super::SetupInputComponent();

	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &AMyCharacter_PlayerController::ShowReturnToMainMenu); // binding



}

void AMyCharacter_PlayerController::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);

	PollInit();


}

void AMyCharacter_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyCharacter_PlayerController, MatchState);
}


void AMyCharacter_PlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
	//			   (      condition     )	if it is nullptr then cast , if not then send to CharacterHUD
	

	bool bHUDValid = MyCh_HUD &&	 //check CharacterHud
		MyCh_HUD->CharacterOverlay &&		//Get CharacterOverlay
		MyCh_HUD->CharacterOverlay->HealthBar &&		//Get HealthBar
		MyCh_HUD->CharacterOverlay->HealthText;			//Get HealthText
	if (bHUDValid) //if the above bool satisfy then ..
	{

	//	UE_LOG(LogTemp, Warning, TEXT("in bHUDValid "));
		const float HealthPercent = Health / MaxHealth;
		MyCh_HUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MyCh_HUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
	

}

void AMyCharacter_PlayerController::SetHUDScore(float Score)
{
	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
	bool bHUDValid = MyCh_HUD &&
		MyCh_HUD->CharacterOverlay &&
		MyCh_HUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		MyCh_HUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
	
}

void AMyCharacter_PlayerController::SetHUDDefeats(int32 Defeats)
{
	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
	bool bHUDValid = MyCh_HUD &&
		MyCh_HUD->CharacterOverlay &&
		MyCh_HUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		MyCh_HUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
	
}

void AMyCharacter_PlayerController::SetHUDWeaponAmmo(int32 Ammo)
{


	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
	bool bHUDValid = MyCh_HUD &&
		MyCh_HUD->CharacterOverlay &&
		MyCh_HUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		MyCh_HUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AMyCharacter_PlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
	bool bHUDValid = MyCh_HUD &&
		MyCh_HUD->CharacterOverlay &&
		MyCh_HUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		MyCh_HUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{

		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void AMyCharacter_PlayerController::OnPossess(APawn* InPawn)
{

	Super::OnPossess(InPawn);
	AMyCharacter* MyCharacter = Cast<AMyCharacter>(InPawn);
	if (MyCharacter)
	{
		SetHUDHealth(MyCharacter->GetHealth(), MyCharacter->GetMaxHealth());
	
	}
}

void AMyCharacter_PlayerController::SetHUDMatchCountdown(float CountdownTime)
{

	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
	bool bHUDValid = MyCh_HUD &&
		MyCh_HUD->CharacterOverlay &&
		MyCh_HUD->CharacterOverlay->MatchCountDownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			MyCh_HUD->CharacterOverlay->MatchCountDownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f); 
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds); //%02d means formatiing number in two digits for ex : if we have 1 then it will padd to 01
		MyCh_HUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountdownText));
	}

}

void AMyCharacter_PlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
	bool bHUDValid = MyCh_HUD &&
		MyCh_HUD->Announcement &&
		MyCh_HUD->Announcement->WarmupTime;
	if (bHUDValid)
	{

		if (CountdownTime < 0.f)
		{
			MyCh_HUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds); //%02d means formatiing number in two digits for ex : if we have 1 then it will padd to 01
		MyCh_HUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}

}

void AMyCharacter_PlayerController::SetHUDGrenades(int32 Grenades)
{
	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
	bool bHUDValid = MyCh_HUD &&
		MyCh_HUD->CharacterOverlay &&
		MyCh_HUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		MyCh_HUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		HUDGrenades = Grenades;
	}

}

void AMyCharacter_PlayerController::SetHUDTime()
{

	if (HasAuthority())
	{
	     DeathLevelGameMode = Cast<ADeathLevelGameMode>(UGameplayStatics::GetGameMode(this));
		if (DeathLevelGameMode)
		{
			LevelStartingTime = DeathLevelGameMode->LevelStartingTime;
			//LevelStartingTime = BlasterGameMode->GetLevelStartingTime();
		}
	}
	float TimeLeft = 0.f;

	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;

	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		DeathLevelGameMode = DeathLevelGameMode == nullptr ? Cast<ADeathLevelGameMode>(UGameplayStatics::GetGameMode(this)) : DeathLevelGameMode;
		if (DeathLevelGameMode)
		{
			SecondsLeft = FMath::CeilToInt(DeathLevelGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;



}

float AMyCharacter_PlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();		// get server time from the world in time seconds 
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;

}

void AMyCharacter_PlayerController::ReceivedPlayer() // eceived player means a player enters the game
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}



void AMyCharacter_PlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);

}

void AMyCharacter_PlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();

}

void AMyCharacter_PlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}

}

void AMyCharacter_PlayerController::PollInit()
{

	if (CharacterOverlay == nullptr)
	{
		if(MyCh_HUD && MyCh_HUD->CharacterOverlay)
		{
			CharacterOverlay = MyCh_HUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);

				AMyCharacter* MyCharacter = Cast<AMyCharacter>(GetPawn());
				if (MyCharacter && MyCharacter->GetCombat())
				{
					SetHUDGrenades(MyCharacter->GetCombat()->GetGrenades());
				}

				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
			}
		}
	}

}

void AMyCharacter_PlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}

}
void AMyCharacter_PlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{

	ClientElimAnnouncement(Attacker, Victim);
}

void AMyCharacter_PlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{


	APlayerState* Self = GetPlayerState<APlayerState>();

	if (Attacker && Victim && Self)
	{
		MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;
		if (MyCh_HUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				MyCh_HUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				MyCh_HUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				MyCh_HUD->AddElimAnnouncement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				MyCh_HUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			MyCh_HUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}



void AMyCharacter_PlayerController::ServerCheckMatchState_Implementation()
{

	ADeathLevelGameMode* GameMode = Cast<ADeathLevelGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
			WarmupTime = GameMode->WarmupTime;
			MatchTime = GameMode->MatchTime;
			CooldownTime = GameMode->CooldownTime;
			LevelStartingTime = GameMode->LevelStartingTime;
			MatchState = GameMode->GetMatchState();
			ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
		
	}


}

void AMyCharacter_PlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown,float StartingTime)
{

	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (MyCh_HUD && MatchState == MatchState::WaitingToStart)
	{
		MyCh_HUD->AddAnnouncement();
	}


}


void AMyCharacter_PlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();

	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMyCharacter_PlayerController::OnRep_MatchState()
{

	if (MatchState == MatchState::InProgress)
	{

		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMyCharacter_PlayerController::HandleMatchHasStarted()
{

	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;

	if (MyCh_HUD)
	{
		//if (MyCh_HUD->CharacterOverlay == nullptr)  // add this line if there is overlapping the HUD(CharacterOverlay)
		
		//	UE_LOG(LogTemp,Warning,TEXT("Add Character Overlay .........."))
			MyCh_HUD->AddCharacterOverlay();
		
		if (MyCh_HUD->Announcement)
		{
			MyCh_HUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}

}

void AMyCharacter_PlayerController::HandleCooldown() // cooldown when the game finishes or restarted
{

	MyCh_HUD = MyCh_HUD == nullptr ? Cast<AMyCh_HUD>(GetHUD()) : MyCh_HUD;

	if (MyCh_HUD)
	{
		MyCh_HUD->CharacterOverlay->RemoveFromParent();
		
		bool bHUDValid = MyCh_HUD->Announcement &&
			MyCh_HUD->Announcement->AnnouncementText &&
			MyCh_HUD->Announcement->InfoText;

		if (bHUDValid)
		{

			MyCh_HUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			MyCh_HUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			//	MyCh_HUD->Announcement->InfoText->SetText(FText());

			//From here the logic of displaying the player wins or tied 

			AProjectGameState* ProjectGameState = Cast<AProjectGameState>(UGameplayStatics::GetGameState(this));
			AMyCharacterPlayerState* MyCharacterPlayerState = GetPlayerState<AMyCharacterPlayerState>();

			if (ProjectGameState && MyCharacterPlayerState)
			{
				TArray<AMyCharacterPlayerState*>TopPlayers = ProjectGameState->TopScoringPlayers;

				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == MyCharacterPlayerState)
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}

				MyCh_HUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}

		}
	}

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter && MyCharacter->GetCombat())
	{
		MyCharacter->bDisableGameplay = true;
		MyCharacter->GetCombat()->FireButtonPressed(false);
	}
}



