// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class V12_the_game : ModuleRules
{
	public V12_the_game(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ChaosVehicles",
			"PhysicsCore",
			//UI
			"UMG", "Slate", "SlateCore",
			//PCG
			"PCG", "PCG_InstanceActorSwapper",
			//Niagara
            "Niagara",
			//"MathPatterns", "TraceHelperFunctions",// unfinished code removed
		});

		PublicIncludePaths.AddRange(new string[] {
			"V12_the_game",
			"V12_the_game/SportsCar",
			"V12_the_game/OffroadCar",
			"V12_the_game/Variant_Offroad",
			"V12_the_game/Variant_TimeTrial",
			"V12_the_game/Variant_TimeTrial/UI"
		});

        PrivateDependencyModuleNames.AddRange(new string[] { });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
