// Some copyright should be here...

using UnrealBuildTool;

public class TraceHelperFunctions : ModuleRules
{
	public TraceHelperFunctions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[] 
		{
			// ... add public include paths required here ...
		});
				
		
		PrivateIncludePaths.AddRange(new string[] 
		{
			// ... add other private include paths required here ...
		});
			
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject",
			// ... add other public dependencies that you statically link with here ...
		});
			
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Engine",// required for debug draw, math and trace
			"Kismet",// For UBlueprintFunctionLibrary implementation details
             
			// CRITICAL: Dependency on MathPatterns plugin to access the Fibonacci pattern generation
			"MathPatterns",
					
		});
		
		
		DynamicallyLoadedModuleNames.AddRange(new string[] 
		{
			// ... add any modules that your module loads dynamically here ...
		});
	}
}
