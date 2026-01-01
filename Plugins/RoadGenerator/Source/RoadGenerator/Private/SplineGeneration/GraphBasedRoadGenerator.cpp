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

bool AGraphBasedRoadGenerator::GenerateSplineComponent_FromLocations(const TArray<FVector>& Locations, bool bIsClosed,
	ELocationType LocationType, USplineComponent*& OutGeneratedSpline)
{
	OutGeneratedSpline = nullptr;

	if (Locations.Num() < 2)
	{
		UE_LOG(RoadGeneratorLog, Warning,
			TEXT("AGraphBasedRoadGenerator::GenerateSplineComponent_FromLocations >> Not enough points [%d]"),
			Locations.Num());
		return false;
	}

	OutGeneratedSpline = NewObject<USplineComponent>(this, TEXT("GeneratedSpline"));
	if (!OutGeneratedSpline)
	{
		UE_LOG(RoadGeneratorLog, Error,
			TEXT("AGraphBasedRoadGenerator::GenerateSplineComponent_FromLocations >> Failed to create spline"));
		return false;
	}

	OutGeneratedSpline->RegisterComponent();
	OutGeneratedSpline->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepWorldTransform);

	OutGeneratedSpline->ClearSplinePoints(false);

	const ESplineCoordinateSpace::Type Space =
		(LocationType == ELocationType::World)
		? ESplineCoordinateSpace::World
		: ESplineCoordinateSpace::Local;

	for (int32 i = 0; i < Locations.Num(); ++i)
	{
		OutGeneratedSpline->AddSplinePoint(Locations[i], Space, false);
		OutGeneratedSpline->SetSplinePointType(i, ESplinePointType::Curve, false);
	}

	OutGeneratedSpline->SetClosedLoop(bIsClosed, false);
	OutGeneratedSpline->UpdateSpline();

	return true;
}

bool AGraphBasedRoadGenerator::RewriteSpline_FromLocations(USplineComponent* Spline, const TArray<FVector>& Locations,
	ELocationType LocationType, bool bIsClosed)
{
	if (!IsValid(Spline))
	{
		UE_LOG(RoadGeneratorLog, Error,
			TEXT("AGraphBasedRoadGenerator::RewriteSpline_FromLocations >> Invalid spline"));
		return false;
	}

	if (Locations.Num() < 2)
	{
		UE_LOG(RoadGeneratorLog, Warning,
			TEXT("AGraphBasedRoadGenerator::RewriteSpline_FromLocations >> Not enough points [%d]"),
			Locations.Num());
		return false;
	}

	Spline->ClearSplinePoints(false);

	const ESplineCoordinateSpace::Type Space =
		(LocationType == ELocationType::World)
		? ESplineCoordinateSpace::World
		: ESplineCoordinateSpace::Local;

	for (int32 i = 0; i < Locations.Num(); ++i)
	{
		Spline->AddSplinePoint(Locations[i], Space, false);
		Spline->SetSplinePointType(i, ESplinePointType::Curve, false);
	}

	Spline->SetClosedLoop(bIsClosed, false);
	Spline->UpdateSpline();

	return true;
}

bool AGraphBasedRoadGenerator::ExtractLocationsFromCurvePoints(const TArray<FCurvePointData>& CurvePoints, TArray<FVector>& OutLocations)
{
	OutLocations.Reset();

	if (CurvePoints.IsEmpty())
	{
		UE_LOG(RoadGeneratorLog, Error,
			TEXT("AGraphBasedRoadGenerator::ExtractLocationsFromCurvePoints >> No curve points"));
		return false;
	}

	OutLocations.Reserve(CurvePoints.Num());

	for (const FCurvePointData& CP : CurvePoints)
	{
		OutLocations.Add(CP.Location);
	}

	return true;
}

bool AGraphBasedRoadGenerator::AddComponentTag(USplineComponent* Spline, FName Tag)
{
	if (!Spline)
	{
		UE_LOG(RoadGeneratorLog, Error,
			TEXT("AGraphBasedRoadGenerator::AddComponentTag >> Invalid spline"));
		return false;
	}

	if (Tag.IsNone())
	{
		UE_LOG(RoadGeneratorLog, Error,
			TEXT("AGraphBasedRoadGenerator::AddComponentTag >> Invalid tag"));
		return false;
	}

	if (Spline->ComponentTags.Contains(Tag)) // already have tag
	{
		UE_LOG(RoadGeneratorLog, Warning,
			TEXT("AGraphBasedRoadGenerator::AddComponentTag >> Spline [%s] already has tag [%s]"),
			*Spline->GetName(),
			*Tag.ToString());
		return true;
	}

#if WITH_EDITOR
	// --- Editor safety ---!!!!!!!
	Spline->Modify();// Undo/Redo + mark dirty

	if (AActor* Owner = Spline->GetOwner())
	{
		Owner->Modify();// ensure actor is marked dirty
	}
#endif

	// all good to go. add the tag
	Spline->ComponentTags.Add(Tag);

	UE_LOG(RoadGeneratorLog, Log,
		TEXT("AGraphBasedRoadGenerator::AddComponentTag >> Spline [%s] now has tag [%s]"),
		*Spline->GetName(),
		*Tag.ToString());

	// --- Make sure editor & PCG see the change ---
	Spline->MarkRenderStateDirty();   // viewport refresh (safe)
	Spline->ReregisterComponent();    // critical for PCG detection

	return true;
}

bool AGraphBasedRoadGenerator::GenerateCurveSegmentSpline(const FCurveSegment& CurveSegment,
                                                          USplineComponent*& OutGeneratedSpline)
{
	//Generate new spline
	if (OutGeneratedSpline)
	{
		OutGeneratedSpline->DestroyComponent();
		OutGeneratedSpline = nullptr;
	}

	// Create spline component owned by THIS actor
	OutGeneratedSpline = NewObject<USplineComponent>(this);
	if (!OutGeneratedSpline)
	{
		UE_LOG(RoadGeneratorLog, Error,
			TEXT("GenerateCurveSegmentSpline >> Failed to create spline component"));
		return false;
	}

	// Attach & register
	OutGeneratedSpline->SetupAttachment(GetRootComponent());
	OutGeneratedSpline->RegisterComponent();

	// Clear existing points
	OutGeneratedSpline->ClearSplinePoints(false);

	if (!CurveSegment.IsCurve)// not curve, straight path
	{
		OutGeneratedSpline->AddSplinePoint(CurveSegment.StartPoint.Location, ESplineCoordinateSpace::Local);

		OutGeneratedSpline->AddSplinePoint(CurveSegment.EndPoint.Location, ESplineCoordinateSpace::Local);

		OutGeneratedSpline->SetSplinePointType(0, ESplinePointType::Linear);
		OutGeneratedSpline->SetSplinePointType(1, ESplinePointType::Linear);

	}
	else // curve case
	{
		OutGeneratedSpline->AddSplinePoint(CurveSegment.StartPoint.Location, ESplineCoordinateSpace::Local);

		// Peak (explicit curvature control)
		OutGeneratedSpline->AddSplinePoint(CurveSegment.PeakPoint.Point.Location, ESplineCoordinateSpace::Local);

		// End
		OutGeneratedSpline->AddSplinePoint(CurveSegment.EndPoint.Location, ESplineCoordinateSpace::Local);

		// All curve points
		OutGeneratedSpline->SetSplinePointType(0, ESplinePointType::Curve);
		OutGeneratedSpline->SetSplinePointType(1, ESplinePointType::Curve);
		OutGeneratedSpline->SetSplinePointType(2, ESplinePointType::Curve);

		// Tangents
		OutGeneratedSpline->SetTangentAtSplinePoint(
			0,
			CurveSegment.StartTangent,
			ESplineCoordinateSpace::Local);

		OutGeneratedSpline->SetTangentAtSplinePoint(
			1,
			CurveSegment.PeakPoint.Point.Tangent,
			ESplineCoordinateSpace::Local);

		OutGeneratedSpline->SetTangentAtSplinePoint(
			2,
			CurveSegment.EndTangent,
			ESplineCoordinateSpace::Local);
	}

	// all done
	OutGeneratedSpline->UpdateSpline();// now, update the spline
	
	return true;
}
