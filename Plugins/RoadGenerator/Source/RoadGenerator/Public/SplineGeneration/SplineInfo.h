// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "SplineInfo.generated.h"
/**
 *  this is for structs required in making splines
 */


UENUM(BlueprintType)
enum class ELocationType:uint8
{
	Local UMETA(DisplayName = "Local Space"),
	World UMETA(DisplayName = "World Space"),
};

//Required struct for reconstructing spline
USTRUCT(BlueprintType)
struct FCurvePointData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Location;
	UPROPERTY(BlueprintReadOnly)
	float DistanceFromSlineOGPoint;// the location from the starting point of the spline
	
	UPROPERTY(BlueprintReadOnly)
	FVector ForwardDirection;
	UPROPERTY(BlueprintReadOnly)
	FVector UpDirection;
	UPROPERTY(BlueprintReadOnly)
	FVector RightDirection;

	UPROPERTY(BlueprintReadOnly)
	float CurvatureValue;

	UPROPERTY(BlueprintReadOnly)//complementary value for the segment building
	FVector Tangent; // for point-level evaluation
};

USTRUCT(BlueprintType)
struct FCurvePeak
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FCurvePointData Point;

	UPROPERTY(BlueprintReadOnly)
	float Curvature;
};

//Required struct for segment info ++ Aslo for reconstructing part of the spline
USTRUCT(BlueprintType)
struct FCurveSegment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FCurvePointData StartPoint ;

	UPROPERTY(BlueprintReadOnly)
	FCurvePeak PeakPoint;// the curve peak( will be also used to calculate tangents for curve spline)

	UPROPERTY(BlueprintReadOnly)
	FCurvePointData EndPoint;
	
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<ESplinePointType::Type> PointType = ESplinePointType::Curve;
	
	UPROPERTY(BlueprintReadOnly)// if it is curve or just straight line
	bool IsCurve=false;// if it is yes, ignore the peak

	
	UPROPERTY(BlueprintReadOnly)
	FVector StartTangent;

	// Tangent at the end point (can be calculated from End -> Peak or End -> Start)
	UPROPERTY(BlueprintReadOnly)
	FVector EndTangent;

	// segment length, for procedural sampling or subdivision
	UPROPERTY(BlueprintReadOnly)
	float SegmentLength = 0.f;
};



USTRUCT(BlueprintType)
struct FAdaptiveSplinePoint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	UPROPERTY(BlueprintReadOnly)// the handle of the curve
	FVector Tangent;

	UPROPERTY(BlueprintReadOnly)
	FVector UpVector;
};


USTRUCT(BlueprintType)// value required for evaluation it the segment is curve of not
struct FCurveEvaluationValues
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float ComfortCurvature= 0.02f;

	UPROPERTY(BlueprintReadWrite)
	float MaxExpectedCurvature= 0.15f;

	UPROPERTY(BlueprintReadWrite)
	float EntranceScaleMax= 2.0f;
	
	UPROPERTY(BlueprintReadWrite)
	float ExitScaleMax= 2.5f;
};
