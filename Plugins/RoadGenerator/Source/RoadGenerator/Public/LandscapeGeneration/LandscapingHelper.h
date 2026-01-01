// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LandscapingHelper.generated.h"


/*
struct FLandscapeSample
{
	FVector WorldLocation;
};
*/// no need just FVector location is all it needs


/**
 *  this is for modifiying landscape data 
 */

//Forward Declares
class ALandscape;
struct FLandscapeEditDataInterface;


//log
ROADGENERATOR_API DECLARE_LOG_CATEGORY_EXTERN(LogLandscapingHelper, Log, All);

UCLASS()
class ROADGENERATOR_API ULandscapingHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Landscaping")
	// ray trace to the world z and -z direction to detect landscape and return the hit location
	static void RayTraceCurvePoints(
		UWorld* World,
		const TArray<AActor*> IgnoringActors,
		UPARAM(ref)TArray<FVector>& SamplePoints,
		float TraceDistance,
		FVector TargetLocationOffset,
		bool bDrawDebug,
		float DebugDrawTime);
	
	UFUNCTION(BlueprintCallable, Category="Landscaping")
	//move the hit landscape point to the sample point location
	static bool ApplyCurvePointsToLandscape(
		UWorld* World,
		const TArray<FVector>& SamplePoints,
		int32 SmoothRadius = 2,
		float BlurStrength = 0.5f);
	
	UFUNCTION(BlueprintCallable, Category="Landscaping")
	// smooth "out" the modified landscape for natural look, and preventing cliff
	static bool SmoothLandscapePoints(
		UWorld* World,
		const TArray<FVector>& SamplePoints,
		int32 SmoothRadius = 2,
		float BlurStrength = 0.5f);
	
private:
	//helper functions
	static bool RayTraceLandscape_Internal(
		UWorld* World,
		const TArray<AActor*> IgnoringActors,
	 	const FVector& Start, 
	 	float TraceDistance,
	 	FVector& OutHitLocation,
	 	bool bDrawDebug,
	 	float DebugDrawTime);

	static bool WorldToHeightmapCoord_Internal(
		ALandscape* Landscape,
		const FVector& WorldLocation,
		int32& OutX,
		int32& OutY,
		uint16& OutHeight);

	static void ApplyHeightWithBlur_Internal(
		FLandscapeEditDataInterface& LandscapeEdit,
		int32 CenterX,
		int32 CenterY,
		uint16 TargetHeight,
		int32 SmoothRadius,
		float BlurStrength);
};
