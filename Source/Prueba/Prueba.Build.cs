using UnrealBuildTool;

public class Prueba : ModuleRules
{
    public Prueba(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "AIModule",
            "NavigationSystem",
            "GameplayTasks"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
        });

        // Solo si realmente haces herramientas del editor
        // if (Target.bBuildEditor)
        // {
        //     PrivateDependencyModuleNames.AddRange(new string[]
        //     {
        //         "UnrealEd",
        //         "PropertyEditor"
        //     });
        // }
    }
}