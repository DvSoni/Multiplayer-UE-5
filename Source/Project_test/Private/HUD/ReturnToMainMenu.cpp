// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"

bool UReturnToMainMenu::Initialize()
{

	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}
void UReturnToMainMenu::MenuSetup()
{

	AddToViewport(); // to actually see thw widget
	SetVisibility(ESlateVisibility::Visible); //visible on screen
	bIsFocusable = true;	// focus on the widget 

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController; // acess player controller 
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;	
			InputModeData.SetWidgetToFocus(TakeWidget()); 
			PlayerController->SetInputMode(InputModeData);// we will able to focus in the game and UI so that way we cans still be able to do things like rotaee or controller 
			PlayerController->SetShowMouseCursor(true);	// showing mouse cursor 
		}
	}
	if (Button_ReturnToMainMenu && !Button_ReturnToMainMenu->OnClicked.IsBound()) // if the button is valid 
	{
		Button_ReturnToMainMenu->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked); // access the delegate on clicking the button 
	}
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem )//&& !MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound()
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);//accessing delegate from MultiplayerSessionsSubsystem file  and adding the dynamic 
		}
	}
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();	// after removing from or hiding from screen 
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);	// hide the mouse cursor 
		}
	}
	if (Button_ReturnToMainMenu && Button_ReturnToMainMenu->OnClicked.IsBound()) // if the button is valid 
	{
		Button_ReturnToMainMenu->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked); // access the delegate on clicking the button 
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}

}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		Button_ReturnToMainMenu->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>(); 
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	Button_ReturnToMainMenu->SetIsEnabled(false);

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}
