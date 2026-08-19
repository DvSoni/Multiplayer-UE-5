// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()



public:
	UPROPERTY(meta = (BindWidget))		//binding the class HealthBar to the widget 
		class UProgressBar* HealthBar;	

	UPROPERTY(meta = (BindWidget))		//binding the class HealthText to the widget 
		class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* ScoreAmount;		//showing the ScoreAmount to widget

	UPROPERTY(meta = (BindWidget))
		UTextBlock* DefeatsAmount; 

	UPROPERTY(meta = (BindWidget))	//showing the WeaponAmmoAmount	 to widget
		UTextBlock* WeaponAmmoAmount;
	
	UPROPERTY(meta = (BindWidget))	//showing the CarriedAmmoAmount	 to widget
		UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* MatchCountDownText;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* GrenadesText;
};
