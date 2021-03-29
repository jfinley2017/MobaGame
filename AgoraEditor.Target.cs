// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

using UnrealBuildTool;
using System.Collections.Generic;

public class AgoraEditorTarget : TargetRules
{
	public AgoraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "Agora" } );

        // Uncomment these to test your builds before pushing.
        // Known Issues: AWS/GAS/ItemComponent/Cheatmanager? These don't seem to cause problems for some reason
        // and can be ignored. All others should be fixed

        bUseUnityBuild = false;
        bUsePCHFiles = false;

    }
}
