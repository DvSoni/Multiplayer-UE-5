#pragma once

#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250	//outline of Weapons && PickUps
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252
#define CUSTOM_DEPTH_RED 249	//outline of Weapons && PickUps
#define CUSTOM_DEPTH_Green 248	
#define CUSTOM_DEPTH_Yellow 247	



UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubMachineGun UMETA(DisplayName = "SubMachine Gun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};