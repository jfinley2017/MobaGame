// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

using UnrealBuildTool;

public class Agora : ModuleRules
{
	public Agora(ReadOnlyTargetRules Target) : base(Target)
	{
        bEnableExceptions = true;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		bEnableUndefinedIdentifierWarnings = false;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "AIModule",
            "GameplayAbilities",
            "GameplayTasks",
            "GameplayTags",
            "UMG",
            "Slate",
            "SlateCore",
            "Http",
            "Json",
            "JsonUtilities",
            "GameLiftServerSDK",
            "GameLiftClientSDK",
            "Landscape",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "Steamworks",
        });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        PrivateDependencyModuleNames.Add("OnlineSubsystem");

        
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
    }
}
