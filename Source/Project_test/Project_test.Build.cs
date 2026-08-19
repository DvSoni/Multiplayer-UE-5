// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_test : ModuleRules
{
	public Project_test(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" ,"Niagara", "MultiplayerSessions", "OnlineSubsystem", "OnlineSubsystemSteam" });
	}
}
