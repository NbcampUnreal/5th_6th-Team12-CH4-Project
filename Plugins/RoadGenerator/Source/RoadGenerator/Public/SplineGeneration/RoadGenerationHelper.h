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

	/*UFUNCTION(BlueprintCallable, Category = "Road|Banking")//calculate road bank degree
	static bool ComputeSmoothBankedRollFromPoints(
		const TArray<FCurvePointData>& CurvePoints,
		const TArray<FCurvePeak>& Peaks,
		float ComfortCurvature,
		float MaxExpectedCurvature,
		float MinInfluenceDistance,
		float MaxInfluenceDistance,
		float IntensityScale,
		float EntranceRatio,
		float ExitRatio,
		float EaseExponent,
		float MaxBankDegrees,
		bool bIsClosed,
		//out
		TArray<float>& OutRollDegrees);*/
	//TODO: this roll cause the road to sunk down. seperate weight output and roll appliace and use the wegith for the lifting data


	UFUNCTION(BlueprintCallable, Category="Road|Banking")
	static bool ComputeSmoothWeightAlphaFromPoints(
		const TArray<FCurvePointData>& CurvePoints,
		const TArray<FCurvePeak>& Peaks,
		float ComfortCurvature,
		float MaxExpectedCurvature,
		float MinInfluenceDistance,
		float MaxInfluenceDistance,
		float IntensityScale,
		float EntranceRatio,
		float ExitRatio,
		float EaseExponent,
		bool bIsClosed,
		//out
		TArray<float>& OutWeightAlpha);

	
	UFUNCTION(BlueprintCallable, Category="Road|Banking")
	static bool ComputeRollFromWeightAlpha(
		const TArray<float>& WeightAlpha,
		float MaxBankDegrees,
		float RollIntensity,
		//out
		TArray<float>& OutRollDegrees);




	
	
	UFUNCTION(BlueprintCallable, Category = "Road|Banking")
	static bool ComputeRollFromPeaks(
		const TArray<float>& SampledDistances,
        const TArray<FCurvePeak>& Peaks,
        float ComfortCurvature,
        float MaxExpectedCurvature,
        float MinInfluenceDistance,
        float MaxInfluenceDistance,
        float IntensityScale,
        float EntranceRatio,
        float ExitRatio,
        float EaseExponent,
        float MaxBankDegrees,
        //out
        TArray<float>& OutRollDegrees);
           
	// this is for updating the curve point data for recreating new spline based on the height update
	UFUNCTION(BlueprintCallable, Category = "Road|Banking")
	static bool LiftUpCurvePoints(
		const TArray<float>& WeightAlpha,
		float LiftingHeight,
		//out
		TArray<FCurvePointData>& OutCurvePointData);
		
	UFUNCTION(BlueprintCallable, Category="Road|GuardRail")// for generating spline on the side
    static bool GenerateOffsetSplineLocationsFromRoll(
        const TArray<FCurvePointData>& CurvePoints,
        const TArray<float>& RollDegrees,
        FVector2D SideOffset,
        // out
        TArray<FVector>& OutLocations);	
		

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
