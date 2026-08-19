// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyCh_HUD.generated.h"


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:

	UPROPERTY() // Added this 
	UTexture2D* CrosshairsCenter;

	UPROPERTY()// Added this 
	UTexture2D* CrosshairsLeft;

	UPROPERTY()// Added this 
	UTexture2D* CrosshairsRight;

	UPROPERTY()// Added this 
	UTexture2D* CrosshairsTop;

	UPROPERTY()// Added this 
	UTexture2D* CrosshairsBottom;


	float CrosshairSpread;  // how much when we spread when we draw them
	FLinearColor CrosshairsColor;
};
/**
 * 
 */
UCLASS()
class AMyCh_HUD : public AHUD
{
	GENERATED_BODY()
	
public:


	virtual void DrawHUD() override;


	UPROPERTY(EditAnywhere,Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass; // set this from our blueprint	 //   < > == forward cast means 

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay; // then we can create this widget


	void AddCharacterOverlay();


	UPROPERTY(EditAnywhere, Category = "Announcements")
		TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
		class UAnnouncement* Announcement;

	void AddAnnouncement();

	void AddElimAnnouncement(FString Attacker, FString Victim);

protected:

	virtual void BeginPlay() override; //beginplay is protected fnction and we have to override it.



private:

	UPROPERTY()
	class APlayerController* OwningPlayer;

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread,FLinearColor Crosshaircolor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 3.7f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

};
