// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineGeneration/GraphBasedRoadGenerator.h"
#include "Components/SplineComponent.h"
#include "SplineGeneration/SplineCorrectionHelper.h"

//Log

DEFINE_LOG_CATEGORY(RoadGeneratorLog);

AGraphBasedRoadGenerator::AGraphBasedRoadGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AGraphBasedRoadGenerator::GenerateSplineComponent_Uniformed(const TArray<FCurvePointData>& CurvePoints, bool bIsClosed,
	USplineComponent*& OutGeneratedSpline)
{
	OutGeneratedSpline = nullptr;

	if (CurvePoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ARoadGenerator::GenerateSplineComponent_Uniformed>> Not enough points. [%d]<2"),
			CurvePoints.Num());
		return false;
	}

	//Generate new spline
	OutGeneratedSpline = NewObject<USplineComponent>(this, TEXT("CorrectedSpline"));
	if (!OutGeneratedSpline)
	{
		UE_LOG(LogTemp, Warning,
		TEXT("ARoadGenerator::GenerateSplineComponent_Uniformed>> Failed to make new spline"));
		return false;
	}

	OutGeneratedSpline->RegisterComponent();
	OutGeneratedSpline->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepWorldTransform);

	OutGeneratedSpline->ClearSplinePoints(false);//reset

	for (int32 i = 0; i < CurvePoints.Num(); ++i)
	{
		const FCurvePointData& CP = CurvePoints[i];

		FSplinePoint SplinePoint;
		SplinePoint.InputKey = i;
		SplinePoint.Position = CP.Location;
		SplinePoint.Type = ESplinePointType::Curve;

		OutGeneratedSpline->AddPoint(SplinePoint, false);
	}

	OutGeneratedSpline->SetClosedLoop(bIsClosed, false);
	OutGeneratedSpline->UpdateSpline();// now, update the spline
	return true;
}
