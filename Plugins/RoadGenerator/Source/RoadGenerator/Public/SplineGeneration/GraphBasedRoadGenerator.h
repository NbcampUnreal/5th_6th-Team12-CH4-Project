// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplineGeneration/SplineInfo.h"
#include "GraphBasedRoadGenerator.generated.h"


//forwardDeclares
class USplineComponent;
class AActor;

// this is for graph based spline path generator which will used udat


//LOG
ROADGENERATOR_API DECLARE_LOG_CATEGORY_EXTERN(RoadGeneratorLog, Log, All);
UCLASS()
class ROADGENERATOR_API AGraphBasedRoadGenerator : public AActor
{
	GENERATED_BODY()

public:
	AGraphBasedRoadGenerator();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Generation")
	AActor* SplineSourceActor=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Generation")
	FName SourceSplineTag=NAME_None;

	// Resampling Safety values
	UPROPERTY(EditAnywhere, Category = "Resample", meta=(ClampMin="2"))
	int32 MaxSamplePoints = 10000;

public:

	//Spline Reconstruction -> this is for making spline based on curve points
	UFUNCTION(BlueprintCallable, Category = "Spline|Generation")// spline point on even distances
	bool GenerateSplineComponent_Uniformed(
		const TArray<FCurvePointData>& CurvePoints,
		bool bIsClosed,
		USplineComponent*& OutGeneratedSpline);//old
	//--> no more using curvepoint for generation. extract the locations and use them as source.



	//=============== Updated Functions ==============================================================================//

	UFUNCTION(BlueprintCallable, Category = "Spline|Generation")
	bool GenerateSplineComponent_FromLocations(
		const TArray<FVector>& Locations,
		bool bIsClosed,
		ELocationType LocationType,
		USplineComponent*& OutGeneratedSpline);

	UFUNCTION(BlueprintCallable, Category = "Spline|Generation")
	bool RewriteSpline_FromLocations(
		USplineComponent* Spline,
		const TArray<FVector>& Locations,
		ELocationType LocationType,
		bool bIsClosed);

	/* ===================== Utilities ===================== */

	UFUNCTION(BlueprintCallable, Category = "Road|Spline")
	static bool ExtractLocationsFromCurvePoints(
		const TArray<FCurvePointData>& CurvePoints,
		TArray<FVector>& OutLocations);


//============ Tag Assignment for PCG Connection =====================================================================//

	UFUNCTION(BlueprintCallable, Category = "Spline|Generation")
	static void AddComponentTag(
		USplineComponent* Spline,
		FName Tag);



	

	UFUNCTION(BlueprintCallable, Category = "Spline|Generation")// this is for reconstructing segment into a spline
	bool GenerateCurveSegmentSpline(
		const FCurveSegment& CurveSegments,
		//out
		USplineComponent*& OutGeneratedSpline);
		
		

	//PCG Conversion
	//UFUNCTION(BlueprintCallable, Category = "Spline|Generation")
	

protected:



};
