// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MathPatternsBPLibrary.generated.h"

UCLASS()
class MATHPATTERNS_API UMathPatternsBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Execute Sample function", Keywords = "MathPatterns sample test testing"), Category = "MathPatternsTesting")
	static float MathPatternsSampleFunction(float Param);

	//FIBONACCI!!!!! FUCK YEAH
	UFUNCTION(BlueprintPure,
		meta = (DisplayName = "Fibonacci Pattern",
			Keywords = "MathPattern"),
			Category = "MathPattern")
	static TArray<FVector2D> GenerateFibonacciSpiralPoints(const FVector2D& Center, float Radius, float Density);
	
	//Hexagon
	// Returns the 6 vertices of a regular hexagon centered at (0,0).
    // !!!!! Vertices are returned in order (e.g., counter-clockwise). !!!!
	UFUNCTION(BlueprintPure,
		meta = (DisplayName = "Calculate Hexagon Vertices (Flat-Top)",
			Keywords = "MathPattern"),
			Category = "MathPattern")
   static  TArray<FVector2D> CalculateHexagonVertices(float Radius);

	UFUNCTION(BlueprintPure,
		meta = (DisplayName = "Populate Hexagon Centers (Grid, Flat-Top)",
			Keywords = "MathPattern"),
			Category = "MathPattern")
	static TArray<FVector2D> PopulateHexagonCenters(float Radius, int32 GridWidth, int32 GridHeight);
};
