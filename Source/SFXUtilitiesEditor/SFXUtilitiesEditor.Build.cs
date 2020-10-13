// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SFXUtilitiesEditor : ModuleRules
{
	public SFXUtilitiesEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"Engine",
			"CoreUObject",
			"InputCore",
			//"LevelEditor",
			//"Slate",
			//"EditorStyle",
			//"AssetTools",
			//"EditorWidgets",
			//"UnrealEd",
			//"BlueprintGraph",
			//"AnimGraph",
			//"ComponentVisualizers",
			"SFXUtilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			//"Core",
			//"CoreUObject",
			//"Engine",
			//"AppFramework",
			//"SlateCore",
			//"AnimGraph",
			"UnrealEd"
			//"KismetWidgets",
			//"MainFrame",
			//"PropertyEditor",
			//"ComponentVisualizers",
			//"SFXUtilities"
		});
	}
}