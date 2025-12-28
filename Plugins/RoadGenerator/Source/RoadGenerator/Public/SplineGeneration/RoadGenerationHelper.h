// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SplineGeneration/SplineInfo.h"

#include "RoadGenerationHelper.generated.h"

/**
 *  this is helper function for making generated road
 */

//LOG
ROADGENERATOR_API DECLARE_LOG_CATEGORY_EXTERN(RoadGenerationHelper, Log, All);

//forward Declares
class USplineComponent;

UCLASS()
class ROADGENERATOR_API URoadGenerationHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//Calculate 
	UFUNCTION(BlueprintCallable, Category="Road|Banking")
	static bool GetAlphaWeightByPeakAtDistance(
		float DistanceFromSplineOG,
		const TArray<FCurvePeak>& Peaks,

		// --- Curvature interpretation ---
		float ComfortCurvature,
		float MaxExpectedCurvature,

		// --- Influence control ---
		float MinInfluenceDistance,
		float MaxInfluenceDistance,
		float IntensityScale,

		// --- Shape ---
		float EntranceRatio,
		float ExitRatio,
		float EaseExponent,

		float& OutWeightAlpha);
		
		
	//Road Banking roll calculation
	UFUNCTION(BlueprintCallable, Category = "Road|Sampling")
	static bool SampleSplineDistances(
		USplineComponent* Spline,
		float SampleDistance,
		int32 MaxSamplePoints,
		TArray<float>& OutDistances);

	UFUNCTION(BlueprintCallable, Category = "Road|Banking")//calculate road bank degree
	static bool ComputeRoadRoll(
	USplineComponent* Spline,
		const TArray<float>& Distances,
		float BankStrength,
		float MaxBankDegrees,
		bool bIsClosed,
		//Outs
		TArray<float>& OutRollDegrees);
	

		

private:
	// internal helper for roll value

	static bool ComputeSeverity( // convert curvature value of curve point into normalized alpha severity[0~1]
		float Curvature,
		float ComfortCurvature,
		float MaxExpectedCurvature,
		//out
		float& OutSeverity);

	static bool ComputeInfluenceDistances(// compute influence distances from the severity
		float Severity,
		float MinInfluenceDistance,
		float MaxInfluenceDistance,
		float IntensityScale,
		float EntranceRatio,
		float ExitRatio,
		//out
		float& OutEntranceDistance,
		float& OutExitDistance);

	static bool EvaluatePeakAlpha( //
		float DistanceFromSplineOG,
		float PeakDistance,
		float EntranceDistance,
		float ExitDistance,
		float EaseExponent,
		//out
		float& OutWeightAlpha);
	
};
