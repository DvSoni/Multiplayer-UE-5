// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void MenuSetup();
	void MenuTearDown();

protected:

	virtual bool Initialize() override;// similar to begin play 

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

private:

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_ReturnToMainMenu; // we need callback to bind this button on click delegate 

	UFUNCTION()
	void ReturnButtonClicked(); // binding with this function  Button_ReturnToMainMenu

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY()
	class APlayerController* PlayerController;
};
