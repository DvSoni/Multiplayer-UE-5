// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotgunWeapon.generated.h"

/**
 * 
 */
UCLASS()
class AShotgunWeapon : public AHitScanWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")	// scattering of bullets when fire in shotgun
		uint32 NumberOfPellets = 10;
	
};
