// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZombieWaveShooter : ModuleRules
{
	public ZombieWaveShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "AIModule", "NavigationSystem", "PhysicsCore", "UMG" });
	}
}
