// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SplineGeneration/SplineInfo.h"
#include "SplineCorrectionHelper.generated.h"

/**
 *  helper functions for spline compoennt modificaitons
 */

//	Requirements

#pragma region Requirements

//Forward Declare
class USplineComponent;
class AActor;



//Evaluate the curvature (is it spiked or rounded) ---> only internal struct, not bp exposed
struct FCurveSegmentEval
{
	const FCurvePeak* Peak;
	float PeakCurvature;
    
	float Severity;// 0–1 normalized
	float EntranceDistance;// evaluated
	float ExitDistance;// evaluated
    
	float StartDistance;// along spline
	float PeakDistance;
	float EndDistance;
    
	bool bAsymmetric;// Symmetric (rounded) / Asymmetric(spiked)
};


#pragma endregion

//Log
ROADGENERATOR_API DECLARE_LOG_CATEGORY_EXTERN(SplineCorrectionHelper, Log, All);

UCLASS()
class ROADGENERATOR_API USplineCorrectionHelper : public UBlueprintFunctionLibrary//changed into bp library
{
	GENERATED_BODY()

public:
	
	// source detection
	UFUNCTION(BlueprintCallable, Category="Spline|Analysis")// get the spline component from the actor
	static bool GetSplineFromActor(
		const AActor* InActor,
		USplineComponent*& OutSpline,
		//optional search
		const FName& SplineTag = NAME_None);

	//Curve points
	UFUNCTION(BlueprintCallable, Category = "Road|Sampling")
	static bool ResampleSpline(//!!!!  even distribution of the points, not same as sampling with spline points
		USplineComponent* SourceSpline,
		float DesiredSampleDistance,
		int32 MaxSamplePoints,// to prevent too many spline point generating
		ELocationType Type,
		bool bIsClosed,// for tnagent calculation
		//Outs
		float& OutCorrectedDistance,
		TArray<FCurvePointData>& OutSplinePoints);// not ouptus the FCurvePointData with more info

	UFUNCTION(BlueprintCallable, Category = "Spline Generation")
	static TArray<FCurvePointData> SmoothCurvePoints(
		const TArray<FCurvePointData>& InCurvePoints,
		float SmoothnessWeight,
		int32 IterationCount,
		bool bIsClosed);
	
	//--> internal helper for the Smooth curve points, but still can be usefull as exposed function
	UFUNCTION(BlueprintCallable, Category="Spline|Analysis")// resetter after location change
	static void UpdateCurvePointsDirectionsAndCurvature(
		TArray<FCurvePointData>& CurvePoints,
		bool bIsClosed);



//Segment Detection

	UFUNCTION(BlueprintCallable, Category="Spline|Analysis")//peak point detection
	static bool DetectCurvePeaks(
		const TArray<FCurvePointData>& CurvePoints,
		float MinCurvatureThreshold, // minimum curvature to consider as a peak
		bool bIsClosed,
		TArray<FCurvePeak>& OutPeaks);


	// the segment will be divided based on the peak point of the curve
	UFUNCTION(BlueprintCallable, Category = "Spline|Analysis")
	static bool DetectCurveSegmentsFromPeaks(
		const USplineComponent* SourceSpline,
		const TArray<FCurvePeak>& Peaks,
		float EntranceBufferDistance,
		float ExitRecoveryDistance,
		float MidpointAlphaOffset,// 0 mid, + more to forward, - more to backward
		ELocationType Type,
		FCurveEvaluationValues EvaluationInfo,
		bool bIsClosed,
		//out
		TArray<FCurveSegment>& OutSegments);

	
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
		TArray<float>& OutRollDegrees,
		TArray<FVector>& OutTangents,
		TArray<FVector>& OutBankedRightVectors);
	
	/*UFUNCTION(BlueprintCallable, Category = "Road|Generation")
	static void GenerateSideSplinePoints(
		USplineComponent* Spline,
		const TArray<float>& Distances,
		const TArray<FVector>& Tangents,
		const TArray<float>& RollDegrees,
		float OffsetDistance,
		bool bRightSide,
		TArray<FVector>& OutSidePoints);*/
	// bool for right and left is too limited. pass the direction instead

	UFUNCTION(BlueprintCallable, Category = "Spline|Generation")
	static bool GenerateSideSplinePoints(
		USplineComponent* Spline,
		const TArray<float>& Distances,
		const TArray<FVector>& Tangents,
		const TArray<float>& RollDegrees,
		float OffsetDistance,
		bool bMirrorDirection,// backward-compatible bool
		const FVector& CustomOffsetDirection, // use this if non-zero
		const TArray<FVector>& BankedRightVectors,
		ELocationType Type,//world or local
		//out
		TArray<FVector>& OutSidePoints);

	UFUNCTION(BlueprintCallable, Category = "Spline|Generation")
	static bool GetPeakPointFromSplineCurveSegment(// this is for getting the peak point for the one curve(from one segmented curve!!!!)
		const TArray<FVector>& SegmentPoints,
		FVector& OutPeakPoint,
		float& OutDeviation);


	


private:

	//Internal helper functions
#pragma region Internal HelperFunctions
	//Direction
	static FVector ComputeForwardDirection(const FVector& Start, const FVector& End)
	{
		return (End - Start).GetSafeNormal();
	}

	static float ComputeCurvature(// curvature 
		const FVector& Prev,
		const FVector& Curr,
		const FVector& Next);

	static void ComputeCurvePointDirectionsAndCurvature_Internal(//internal wrapper
		const FVector& Prev,
		const FVector& Curr,
		const FVector& Next,
		FVector& OutForward,
		FVector& OutUp,
		FVector& OutRight,
		float& OutCurvature);


	static void SmoothCurvePoints_Internal(//internal
		TArray<FVector>& InCurvePointLocations,
		float SmoothnessWeight,
		bool bIsClosed);


	static FCurveSegmentEval EvaluateCurvePeak(
		const FCurvePeak& Peak,
		float ComfortCurvature,
		float MaxExpectedCurvature,
		float BaseEntranceDistance,
		float BaseExitDistance,
		float EntranceScaleMax,
		float ExitScaleMax,
		float MidpointAlphaOffset);


	// for getting neighboring index point
	static bool GetNeighborIndices(
    	int32 Index,
    	int32 Num,
    	bool bIsClosed,
    	//out
    	int32& OutPrev,
    	int32& OutNext);

	// for tangent calculation
	static FVector ComputeTangentAtIndex(
		const TArray<FVector>& Locations,
		int32 Index,
		bool bIsClosed);

	// project and flatten the curve for
	// after that use them for the peak

	static bool FlattenCurvePointsToTheDirection(
		const TArray<FCurvePointData>& SplineCurvePoints,
		bool IsClosed,
		FVector ProjectionNormal,
		//out
		TArray<float>& OutCurvatureValues);


#pragma endregion
	
};
