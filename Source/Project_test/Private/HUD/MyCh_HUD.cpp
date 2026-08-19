// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCh_HUD.h"

#include "GameFramework/PlayerController.h"

#include "CharacterOverlay.h"

#include "Announcement.h"

#include "ElimAnnouncement.h"

#include "Components/HorizontalBox.h"

#include "Blueprint/WidgetLayoutLibrary.h"

#include "Components/CanvasPanelSlot.h"

void AMyCh_HUD::BeginPlay()
{
	Super::BeginPlay();

	//AddCharacterOverlay();

}

void AMyCh_HUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}

}

void AMyCh_HUD::AddAnnouncement()
{

	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}

}

void AMyCh_HUD::AddElimAnnouncement(FString Attacker, FString Victim)
{

	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if (ElimAnouncementWidget)
		{
			
			ElimAnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnouncementWidget->AddToViewport();


			for (UElimAnnouncement* Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(
							CanvasSlot->GetPosition().X,
							Position.Y - CanvasSlot->GetSize().Y
						);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}



			ElimMessages.Add(ElimAnouncementWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnouncementWidget);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				ElimMsgDelegate,
				ElimAnnouncementTime,
				false
			);
		}
	}


}

void AMyCh_HUD::DrawHUD()
{

	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread; 


		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f); 
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter,Spread,HUDPackage.CrosshairsColor); // draw the crosshair in the game and color it 
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter,Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter,Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter,Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter,Spread, HUDPackage.CrosshairsColor);
		}
	}

}

void AMyCh_HUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter , FVector2D Spread , FLinearColor Crosshaircolor)
{
	const float TextureWidth = Texture -> GetSizeX(); // give us the width of the texture 
	const float TextureHeight = Texture -> GetSizeY(); // give us the height of the texture
	
	// UE_LOG(LogTemp, Display, TEXT("GetSizex == %f"), TextureWidth);


	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,	    // draw is to be center of the skin 
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y	   // draw is to be center of the skin 
	);

	DrawTexture(									//drawtexture in-built function to draw the hud
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		Crosshaircolor		// texture color 
	);
}

void AMyCh_HUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{

	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}
