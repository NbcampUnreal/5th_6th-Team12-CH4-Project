// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineGeneration/SplineCorrectionHelper.h"

#include "Components/SplineComponent.h"

//Log
DEFINE_LOG_CATEGORY(SplineCorrectionHelper);

//============ Reference Getter =============//
bool USplineCorrectionHelper::GetSplineFromActor(const AActor* InActor, USplineComponent*& OutSpline, const FName& SplineTag)
{
	OutSpline=nullptr;
	
	if (!InActor)
	{
		UE_LOG(SplineCorrectionHelper, Error,
        			TEXT("USplineCorrectionHelper::GetSplineFromActor >> Invalid InActor"));
		return false;
	}

	TArray<USplineComponent*> SplineComponents;//catcher
	
	InActor->GetComponents<USplineComponent>(SplineComponents);
	if (SplineComponents.IsEmpty())
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::GetSplineFromActor >> No Spline Found from Actor[%s]"),
			*InActor->GetName());
		return false;
	}

	if (SplineTag!=NAME_None)// the tag name has been passed, no default spline[0]
	{
		for (USplineComponent* SplineComponent : SplineComponents)
		{
			if (!SplineComponent) continue;
			if (!SplineComponent->ComponentHasTag(SplineTag)) continue;

			OutSpline=SplineComponent;
			return true;
		}
		
		// Spline with given tag not found
		UE_LOG(SplineCorrectionHelper,Error,
			TEXT("GetSplineFromActor: No spline with tag [%s] found on Actor [%s]"),
			*SplineTag.ToString(),
			*InActor->GetName());

		return false;
	}

	OutSpline=SplineComponents[0];//default(for now, just for 1 spline component case)

	if (SplineComponents.Num() >1)//more than 1
	{
		UE_LOG(SplineCorrectionHelper,Warning,
			TEXT("GetSplineFromActor: Actor [%s] has multiple splines. Using first [%s] as default"),
			*InActor->GetName(),
			*OutSpline->GetName()
		);
	}
	// all done
	return true;
}

//=============================================== Resampleing ========================================================//
#pragma region CurvePoints Resampling
bool USplineCorrectionHelper::ResampleSpline(USplineComponent* SourceSpline, float DesiredSampleDistance, int32 MaxSamplePoints,
                                             ELocationType Type, bool bIsClosed,float& OutCorrectedDistance, TArray<FCurvePointData>& OutSplinePoints)
{
	if (!SourceSpline)// passing it without checking valid cause runtime error 
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::InternalResampleSpline >> Invalid SourceSpline"));
		return false;
	}
	
	return ResampleSpline_Internal(SourceSpline, DesiredSampleDistance, MaxSamplePoints, Type,
								  0.f, SourceSpline->GetSplineLength(), bIsClosed,
								  OutCorrectedDistance, OutSplinePoints);
}

bool USplineCorrectionHelper::ResampleSplineInRange(USplineComponent* SourceSpline, float DesiredSampleDistance,
	int32 MaxSamplePoints, ELocationType Type, float StartDistance, float EndDistance, float& OutCorrectedDistance,
	TArray<FCurvePointData>& OutSplinePoints)
{
	if (!SourceSpline)//same
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::InternalResampleSpline >> Invalid SourceSpline"));
		return false;
	}
	
	return ResampleSpline_Internal(SourceSpline, DesiredSampleDistance, MaxSamplePoints, Type,
								  StartDistance, EndDistance, false,
								  OutCorrectedDistance, OutSplinePoints);
}

bool USplineCorrectionHelper::ResampleSpline_Internal(USplineComponent* SourceSpline, float DesiredSampleDistance,
	int32 MaxSamplePoints, ELocationType Type, float StartDistance, float EndDistance, bool bIsClosed,
	float& OutCorrectedDistance, TArray<FCurvePointData>& OutSplinePoints)
{
	OutSplinePoints.Reset();
	OutCorrectedDistance = 0.f;
	
    
	const float SplineLength = SourceSpline->GetSplineLength();
	StartDistance = FMath::Clamp(StartDistance, 0.f, SplineLength);
	EndDistance = FMath::Clamp(EndDistance, StartDistance, SplineLength);
    
	const float RangeLength = EndDistance - StartDistance;
	if (RangeLength <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::InternalResampleSpline >> Invalid Range: Start[%f], End[%f]"),
			StartDistance, EndDistance);
		return false;
	}    

	//compute number of segments
	int32 NumSegments = FMath::FloorToInt(RangeLength / DesiredSampleDistance);
	NumSegments = FMath::Max(1, NumSegments);

	// Actual corrected distance
	OutCorrectedDistance = RangeLength / NumSegments;

	// location type
	ESplineCoordinateSpace::Type CoordSpace =
		(Type == ELocationType::Local) ? ESplineCoordinateSpace::Local : ESplineCoordinateSpace::World;
	
	UE_LOG(SplineCorrectionHelper, Log,
		TEXT("USplineCorrectionHelper::InternalResampleSpline >> Resampling from [%f] to [%f] with [%d] segments, CorrectedDistance[%f]"),
		StartDistance, EndDistance, NumSegments, OutCorrectedDistance);
	
	TArray<FVector> Locations;
	Locations.Reserve(NumSegments + 1);

	TArray<float> Distances;
	Distances.Reserve(NumSegments + 1);

	for (int32 i = 0; i <= NumSegments; ++i)
	{
		const float Distance = StartDistance + i * OutCorrectedDistance;

		Locations.Add(SourceSpline->GetLocationAtDistanceAlongSpline(Distance, CoordSpace));
		Distances.Add(Distance);
	}

	OutSplinePoints.Reserve(Locations.Num());

	for (int32 i = 0; i < Locations.Num(); ++i)
	{
		FVector Tangent =SourceSpline->GetTangentAtDistanceAlongSpline(Distances[i], CoordSpace);// get accurate one, not by the neighbor computation
		FVector Up =SourceSpline->GetUpVectorAtDistanceAlongSpline(Distances[i], CoordSpace).GetSafeNormal();
		FVector Right =FVector::CrossProduct(Tangent, Up).GetSafeNormal();

		FCurvePointData PointData;
		PointData.Location = Locations[i];
		PointData.DistanceFromSlineOGPoint = Distances[i];
		PointData.ForwardDirection = Tangent;
		PointData.UpDirection = Up;
		PointData.RightDirection = Right;
		PointData.Tangent = Tangent;

		if (i == 0 || i == Locations.Num() - 1)
		{
			PointData.CurvatureValue = 0.f;
		}
		else
		{
			PointData.CurvatureValue =
				ComputeCurvature(Locations[i - 1], Locations[i], Locations[i + 1]);
		}

		OutSplinePoints.Add(PointData);
	}

	UE_LOG(SplineCorrectionHelper, Log,
		TEXT("USplineCorrectionHelper::InternalResampleSpline >> Resampling complete. TotalPoints[%d]"),
		OutSplinePoints.Num());

	return true;
}
#pragma endregion
//--------------------------------------------------------------------------------------------------------------------//

//==============================================  Smoothing  =========================================================//
#pragma region Smoothing

TArray<FCurvePointData> USplineCorrectionHelper::SmoothCurvePoints(const TArray<FCurvePointData>& InCurvePoints, float SmoothnessWeight,
	int32 IterationCount,bool bIsClosed, bool bUseTaubinSmoothing)
{
	TArray<FCurvePointData> Result = InCurvePoints;

	// Extract locations for smoothing
	TArray<FVector> Locations;
	Locations.Reserve(Result.Num());
	for (auto& P : Result)
		Locations.Add(P.Location);

	// Iterative smoothing(only the location)
	if (bUseTaubinSmoothing)
	{
		for (int32 i = 0; i < IterationCount; ++i)
			Taubin_SmoothCurvePoints_Internal(Locations, SmoothnessWeight,bIsClosed);// changed with Taubin method
	}
	else
	{
		for (int32 i = 0; i < IterationCount; ++i)
			Laplacian_SmoothCurvePoints_Internal(Locations, SmoothnessWeight,bIsClosed);
	}


	// Update locations
	for (int32 i = 0; i < Result.Num(); ++i)
		Result[i].Location = Locations[i];

	// Recalculate curvature and directions after smoothing
	UpdateCurvePointsDirectionsAndCurvature(Result, bIsClosed);

	return Result;
}

//Laplacian smoothing-> loses the key shape of the original form
void USplineCorrectionHelper::Laplacian_SmoothCurvePoints_Internal(TArray<FVector>& InCurvePointLocations, float SmoothnessWeight, bool bIsClosed)
{
	/*
	const int32 Num = InCurvePointLocations.Num();
	if (Num < 3)
		return;

	TArray<FVector> Original = InCurvePointLocations;

	const float Lambda = 1.f - SmoothnessWeight;

	for (int32 i = 0; i < Num; ++i)
	{
		int32 Prev, Next;
		if (!GetNeighborIndices(i, Num, bIsClosed, Prev, Next))
			continue;

		const FVector Avg = (Original[Prev] + Original[Next]) * 0.5f;
		const FVector Laplacian = Avg - Original[i];

		InCurvePointLocations[i] = Original[i] + Lambda * Laplacian;
	}
	*/
	
	int32 NumPoints = InCurvePointLocations.Num();
	 if (NumPoints < 3) return;
	
	TArray<FVector> Temp = InCurvePointLocations;
	for (int32 i = 0; i < NumPoints; ++i)
	{
	   	int32 PrevIdx, NextIdx;
	     if (!GetNeighborIndices(i, NumPoints, bIsClosed, PrevIdx, NextIdx)) 
	     { // Open spline endpoints
	     	Temp[i] = InCurvePointLocations[i]; continue;
	}
		Temp[i] = InCurvePointLocations[i] * SmoothnessWeight
			+0.5f * (InCurvePointLocations[PrevIdx]
			+ InCurvePointLocations[NextIdx]) * (1.f - SmoothnessWeight);
	}
	
	InCurvePointLocations = MoveTemp(Temp);
}

void USplineCorrectionHelper::Taubin_SmoothCurvePoints_Internal(TArray<FVector>& Points, float Weight, bool bIsClosed)
{
	// Taubin parameters
	// Weight controls overall strength (0~1)
	// Lambda removes noise
	// Mu counteracts shrinkage
	const float Lambda = Weight;
	const float Mu     = -0.53f * Weight;

	// First pass: standard Laplacian smoothing
	Laplacian_SmoothCurvePoints_Internal(
		Points,
		1.f - Lambda,   // convert to your SmoothnessWeight convention
		bIsClosed);

	// Second pass: reverse Laplacian (inflation)
	Laplacian_SmoothCurvePoints_Internal(
		Points,
		1.f - Mu,       // Mu is negative → expands shape
		bIsClosed);
}


void USplineCorrectionHelper::UpdateCurvePointsDirectionsAndCurvature(TArray<FCurvePointData>& CurvePoints, bool bIsClosed)
{
	const int32 NumPoints = CurvePoints.Num();
	if (NumPoints < 2) return;

	for (int32 i = 0; i < NumPoints; ++i)
	{
		/*//fucking const initialization piece of shit
		const int32 PrevIdx = (i == 0)? (bIsClosed ? NumPoints - 1/*closed#1# : i/*not closed#1#)/*start#1#: i - 1/*not start#1#;
		const int32 NextIdx =(i == NumPoints - 1)? (bIsClosed ? 0 : i): i + 1;*/// fuck yeah
		int32 PrevIdx, NextIdx;
		if (!GetNeighborIndices(i, NumPoints, bIsClosed, PrevIdx, NextIdx))
			continue;
		
		const FVector& Prev = CurvePoints[PrevIdx].Location;
		const FVector& Curr = CurvePoints[i].Location;//current
		const FVector& Next = CurvePoints[NextIdx].Location;

		FVector Forward, Up, Right;
		float Curvature;

		ComputeCurvePointDirectionsAndCurvature_Internal(Prev, Curr, Next, Forward, Up, Right, Curvature);

		CurvePoints[i].ForwardDirection = Forward;
		CurvePoints[i].UpDirection = Up;
		CurvePoints[i].RightDirection = Right;
		CurvePoints[i].CurvatureValue = Curvature;
	}
}


#pragma endregion
//--------------------------------------------------------------------------------------------------------------------//

//==============================================  Detection  =========================================================//
#pragma region Detection

bool USplineCorrectionHelper::DetectCurvePeaks(const TArray<FCurvePointData>& CurvePoints, float MinCurvatureThreshold,bool bIsClosed,
                                               TArray<FCurvePeak>& OutPeaks)
{
	TArray<FVector> AnalysisPoints;
	
	AnalysisPoints.Reserve(CurvePoints.Num());

	for (const FCurvePointData& P : CurvePoints)
	{
		AnalysisPoints.Add(P.Location);
	}

	return DetectCurvePeaks_Internal(// now use internal function
		CurvePoints,
		AnalysisPoints,
		MinCurvatureThreshold,
		bIsClosed,
		OutPeaks);
}

bool USplineCorrectionHelper::DetectCurvePeaks_BasedOnNormal(const TArray<FCurvePointData>& CurvePoints,
	FVector ProjectionNormal, float MinCurvatureThreshold, bool bIsClosed, TArray<FCurvePeak>& OutPeaks)
{
	TArray<FVector> FlattenedPoints;//catcher
	if (!FlattenCurvePointsByProjectionNormal(CurvePoints,bIsClosed,ProjectionNormal,FlattenedPoints))
	{
		return false;//flattening failed
	}

	return DetectCurvePeaks_Internal(CurvePoints,FlattenedPoints,MinCurvatureThreshold,bIsClosed,OutPeaks);
}

// --------- Internal helpers
bool USplineCorrectionHelper::DetectCurvePeaks_Internal(const TArray<FCurvePointData>& CurvePoints,
	const TArray<FVector>& AnalysisPoints, float MinDeviationThreshold, bool bIsClosed, TArray<FCurvePeak>& OutPeaks)
{
	OutPeaks.Reset();//reset first
	
	const int32 CurvePointCount = AnalysisPoints.Num();
	if (CurvePoints.Num() != CurvePointCount/*Not Matching*/)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::DetectCurvePeaks_Internal >> CurvePoints and AnalysisPoints does not match"));
		return false;
	}
	
	if ((bIsClosed && CurvePointCount < 3)/*ClosedLoop Condition*/ || (!bIsClosed && CurvePointCount<2))/*None_looping Condition*/
	{
		FString ErrorMessage = bIsClosed? TEXT("Closed Loop [3]"):TEXT("Unclosed Spline [2]");
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::DetectCurvePeaks_Internal >> not enough point for %s <%d."),
			*ErrorMessage, CurvePointCount);
		return false;
	}

	// Compute Deviation for all points
	TArray<float> Deviations;
	Deviations.SetNumZeroed(CurvePointCount);

	for (int32 i = 0; i < CurvePointCount; ++i)
	{
		int32 PrevIdx, NextIdx;
		if (!GetNeighborIndices(i, CurvePointCount, bIsClosed, PrevIdx, NextIdx))
		{
			continue;
		}

		FVector DummyPeak;
		float Deviation = 0.f;

		TArray<FVector> Segment;
		Segment.Reserve(3);
		Segment.Add(AnalysisPoints[PrevIdx]);
		Segment.Add(AnalysisPoints[i]);
		Segment.Add(AnalysisPoints[NextIdx]);

		if (GetPeakPointFromSplineCurveSegment(Segment, DummyPeak, Deviation))
		{
			Deviations[i] = Deviation;
		}
	}

	//Detect Local Max
	for (int32 i = 0; i < CurvePointCount; ++i)
	{
		float Curr = Deviations[i];
		if (Curr < MinDeviationThreshold)
		{
			continue;
		}

		int32 PrevIdx, NextIdx;
		if (!GetNeighborIndices(i, CurvePointCount, bIsClosed, PrevIdx, NextIdx))
		{
			continue;
		}

		float Prev = Deviations[PrevIdx];
		float Next = Deviations[NextIdx];

		// Local maximum condition
		if (Curr >= Prev && Curr > Next)
		{
			FCurvePeak Peak;
			Peak.Point = CurvePoints[i];
			Peak.Curvature = Curr;

			OutPeaks.Add(Peak);
		}
	}

	return OutPeaks.Num() > 0;
}

void USplineCorrectionHelper::BuildCurveSegmentTangents(const FCurvePointData& StartPoint,
	const FCurvePointData& PeakPoint, const FCurvePointData& EndPoint, FVector& OutStartTangent, FVector& OutEndTangent)
{
	//Get locations
	const FVector& StartPosition=StartPoint.Location;
	const FVector& PeakPosition=PeakPoint.Location;
	const FVector& EndPosition=EndPoint.Location;

	// get direction Start-peak, peak-end
	const FVector StartDirection=(PeakPosition - StartPosition).GetSafeNormal();
	const FVector EndDirection=(EndPosition - PeakPosition).GetSafeNormal();

	//Get distance Start-peak, peak-end
	const float StartLength=(PeakPosition - StartPosition).Size();
	const float EndLength=(EndPosition - PeakPosition).Size();

	//Make tangent value
	OutStartTangent=StartDirection*StartLength;
	OutEndTangent=EndDirection*EndLength;
}

void USplineCorrectionHelper::FinalizeCurveSegment(FCurveSegment& CurveSegment)
{
	CurveSegment.SegmentLength=(CurveSegment.EndPoint.Location-CurveSegment.StartPoint.Location).Size();
	
	if(!CurveSegment.IsCurve)// not a curve, straight path
	{
		CurveSegment.StartTangent = FVector::ZeroVector;
		CurveSegment.EndTangent   = FVector::ZeroVector;
		CurveSegment.PointType    = ESplinePointType::Linear;

		return;
	}

	// curve case
	BuildCurveSegmentTangents(
		CurveSegment.StartPoint,
		CurveSegment.PeakPoint.Point,
		CurveSegment.EndPoint,
		CurveSegment.StartTangent,
		CurveSegment.EndTangent
	);

	CurveSegment.PointType = ESplinePointType::CurveCustomTangent;
}

bool USplineCorrectionHelper::GeneratePeakWeightAlpha(const FCurveSegment& CurveSegment,
	const TArray<FCurvePointData>& ResampledCurve, TArray<float>& OutAlpha)
{
	OutAlpha.Empty();

	if (!CurveSegment.IsCurve)
	{
		UE_LOG(SplineCorrectionHelper, Warning,
			   TEXT("USplineCorrectionHelper::GeneratePeakWeightAlpha >> Segment is not a curve, returning 0 alphas."));
		OutAlpha.Init(0.f, ResampledCurve.Num());
		return true;
	}

	if (ResampledCurve.Num() < 2)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			   TEXT("USplineCorrectionHelper::GeneratePeakWeightAlpha >> Not enough points in ResampledCurve [%d]"),
			   ResampledCurve.Num());
		return false;
	}

	// Segment distances
	float StartDist = CurveSegment.StartPoint.DistanceFromSlineOGPoint;
	float PeakDist  = CurveSegment.PeakPoint.Point.DistanceFromSlineOGPoint;
	float EndDist   = CurveSegment.EndPoint.DistanceFromSlineOGPoint;

	float SegmentLength = EndDist - StartDist;
	if (SegmentLength <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			   TEXT("USplineCorrectionHelper::GeneratePeakWeightAlpha >> Invalid segment length [%f]"),
			   SegmentLength);
		return false;
	}

	OutAlpha.Reserve(ResampledCurve.Num());

	for (const FCurvePointData& Point : ResampledCurve)
	{
		// Local distance relative to start of segment
		float LocalDist = Point.DistanceFromSlineOGPoint - StartDist;
		float LocalAlpha = 0.f;

		if (Point.DistanceFromSlineOGPoint <= PeakDist) // Start -> Peak
		{
			LocalAlpha = LocalDist / (PeakDist - StartDist);
		}
		else // Peak -> End
		{
			LocalAlpha = 1.f - (Point.DistanceFromSlineOGPoint - PeakDist) / (EndDist - PeakDist);
		}

		LocalAlpha = FMath::Clamp(LocalAlpha, 0.f, 1.f);
		OutAlpha.Add(LocalAlpha);
	}

	UE_LOG(SplineCorrectionHelper, Log,
		   TEXT("USplineCorrectionHelper::GeneratePeakWeightAlpha >> Generated %d alpha values for curve segment"),
		   OutAlpha.Num());

	return true;
}

bool USplineCorrectionHelper::ResampleAndBankSegment(const FCurveSegment& Segment, USplineComponent* SourceSpline,
	float DesiredSampleDistance, int32 MaxSamplePoints, ELocationType CoordType, float MaxRollDegrees,
	TArray<FCurvePointData>& OutRolledPoints)
{
     OutRolledPoints.Reset();

    if (!SourceSpline)
    {
        UE_LOG(SplineCorrectionHelper, Error, 
            TEXT("USplineCorrectionHelper::ResampleAndBankSegment >> Invalid SourceSpline"));
        return false;
    }

    float CorrectedDistance = 0.f;
    TArray<FCurvePointData> ResampledPoints;

    // Resample only within this segment
    if (!ResampleSpline_Internal(
        SourceSpline,
        DesiredSampleDistance,
        MaxSamplePoints,
        CoordType,
        Segment.StartPoint.DistanceFromSlineOGPoint,
        Segment.EndPoint.DistanceFromSlineOGPoint,
        false, // this is segment, no loop
        CorrectedDistance,
        ResampledPoints))
    {
        UE_LOG(SplineCorrectionHelper, Error,
            TEXT("USplineCorrectionHelper::ResampleAndBankSegment >> "
                 "Failed to resample segment [Start: %.2f, End: %.2f]"),
            Segment.StartPoint.DistanceFromSlineOGPoint,
            Segment.EndPoint.DistanceFromSlineOGPoint);
        return false;
    }

    UE_LOG(SplineCorrectionHelper, Log,
        TEXT("USplineCorrectionHelper::ResampleAndBankSegment >> "
			 "Resampled %d points, CorrectedDistance: %.3f"),
        ResampledPoints.Num(),
        CorrectedDistance);

    // Generate peak influence alpha
    TArray<float> PeakAlpha;
    if (!GeneratePeakWeightAlpha(Segment, ResampledPoints, PeakAlpha))
    {
        UE_LOG(SplineCorrectionHelper, Warning,
            TEXT("USplineCorrectionHelper::ResampleAndBankSegment >> "
                 "Failed to generate peak alpha for segment"));
        PeakAlpha.Init(0.f, ResampledPoints.Num());
    }

    // Apply the banking roll using alpha
    OutRolledPoints.Reserve(ResampledPoints.Num());
    for (int32 i = 0; i < ResampledPoints.Num(); ++i)
    {
        const FCurvePointData& SrcPoint = ResampledPoints[i];
        float RollAngle = MaxRollDegrees * PeakAlpha[i];

        // Build rotation around forward vector
        FQuat RollQuat = FQuat(SrcPoint.ForwardDirection, FMath::DegreesToRadians(RollAngle));

        FCurvePointData RolledPoint = SrcPoint;
        RolledPoint.UpDirection    = RollQuat.RotateVector(SrcPoint.UpDirection).GetSafeNormal();
        RolledPoint.RightDirection = RollQuat.RotateVector(SrcPoint.RightDirection).GetSafeNormal();

        OutRolledPoints.Add(RolledPoint);
    }

    UE_LOG(SplineCorrectionHelper, Log,
        TEXT("USplineCorrectionHelper::ResampleAndBankSegment >>"
			 " Applied road bank to %d points (MaxRoll: %.2f degree)"),
        OutRolledPoints.Num(), MaxRollDegrees);

    return true;
}



bool USplineCorrectionHelper::FlattenCurvePointsByProjectionNormal(const TArray<FCurvePointData>& SplineCurvePoints,
                                                                   bool IsClosed, FVector InProjectionNormal, TArray<FVector>& OutFlattenedLocations)
{
	//Reset
	OutFlattenedLocations.Reset();

	const int32 SplineCurveCount=SplineCurvePoints.Num();
	if ((IsClosed && SplineCurveCount<3) || (!IsClosed&&SplineCurveCount<2))
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::FlattenCurvePointsToTheDirection >> Not Enought Curvepoints [%d]"),
			SplineCurveCount);
		return false;
	}

	const FVector ProjectionNormal=InProjectionNormal.GetSafeNormal();
	if (ProjectionNormal.IsNearlyZero())
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::FlattenCurvePointsToTheDirection >> Invalid ProjectionNormal"));
		return false;
	}

	// Use first point as plane origin
	const FVector PlaneOrigin = SplineCurvePoints[0].Location;
	
	const FVector Normal = ProjectionNormal.GetSafeNormal();// normalize just in case
	
	for (int32 i = 0; i < SplineCurveCount; ++i)
	{
		const FVector& P = SplineCurvePoints[i].Location;

		// Project point onto plane
		const float SignedDistance = FVector::DotProduct(P - PlaneOrigin, Normal);
		const FVector Flattened = P - SignedDistance * Normal;

		OutFlattenedLocations.Add(Flattened);
	}

	// Closed splines keep logical closure, not duplicated point
	return true;
}

#pragma endregion
//====================================================================================================================//




//==================================== Curve Segment Detection =======================================================//

//Internal cpp struct for DetectCurveSegmentsFromPeaks
struct FCurveEnvelope
{
	float StartDistance;
	float PeakDistance;
	float EndDistance;

	//peak data
	const FCurvePeak* Peak;
	// roundness evaluation data
	float Severity;
	bool bAsymmetric;
};

bool USplineCorrectionHelper::DetectCurveSegmentsFromPeaks(const USplineComponent* SourceSpline,
	const TArray<FCurvePeak>& Peaks, float EntranceBufferDistance, float ExitRecoveryDistance,
	float MidpointAlphaOffset,ELocationType Type, FCurveEvaluationValues EvaluationInfo, bool bIsClosed, TArray<FCurveSegment>& OutSegments)
{
	//reset
	OutSegments.Empty();

	if (!SourceSpline)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::DetectCurveSegmentsFromPeaks >> Invalid SourceSpline"));
		return false;
	}
	if (Peaks.IsEmpty())
	{
		UE_LOG(SplineCorrectionHelper, Warning,
			TEXT("USplineCorrectionHelper::DetectCurveSegmentsFromPeaks >> No peaks provided"));
		return false;
	}

	const float SplineLength = SourceSpline->GetSplineLength();
	MidpointAlphaOffset = FMath::Clamp(MidpointAlphaOffset, -0.5f, 0.5f);//clamp the range to 1
	
	const ESplineCoordinateSpace::Type CoordSpace =
			(Type == ELocationType::Local)?/*condition*/ ESplineCoordinateSpace::Local /*Or*/: ESplineCoordinateSpace::World;

	
	//Build curve envelope data
	TArray<FCurveEnvelope> Envelopes;
	Envelopes.Reserve(Peaks.Num());
	
	const float MinSegmentLength = 1.0f;//safety -> min segment
	
	for (const FCurvePeak& Peak : Peaks)
	{
		const float PeakDist = Peak.Point.DistanceFromSlineOGPoint;

		// Normalize curvature → severity
		float Severity = (Peak.Curvature - EvaluationInfo.ComfortCurvature) /
			(EvaluationInfo.MaxExpectedCurvature - EvaluationInfo.ComfortCurvature);
		Severity = FMath::Clamp(Severity, 0.f, 1.f);

		const bool bAsymmetric = Severity > KINDA_SMALL_NUMBER;//round curve = symmetric, a spiked curve = asymmetric
		
		// Scale buffer by curvature
		const float Entrance =
			EntranceBufferDistance*FMath::Lerp(1.f, EvaluationInfo.EntranceScaleMax, Severity);
		const float Exit =
			ExitRecoveryDistance*FMath::Lerp(1.f, EvaluationInfo.ExitScaleMax, Severity);
		
		// make data
		FCurveEnvelope Env;
		Env.PeakDistance = PeakDist;
		Env.StartDistance = FMath::Max(0.f, PeakDist - Entrance);
		Env.EndDistance = FMath::Min(SplineLength, PeakDist + Exit);
		
		//peak data
		Env.Peak = &Peak;
		Env.Severity      = Severity;
		Env.bAsymmetric   = bAsymmetric;

		//Safety ++
		// Ensure minimum length
		if (Env.EndDistance - Env.StartDistance < MinSegmentLength)
		{
			float Half = MinSegmentLength * 0.5f;
			Env.StartDistance = FMath::Max(0.f, PeakDist - Half);
			Env.EndDistance   = FMath::Min(SplineLength, PeakDist + Half);
		}
		
		// add data
		Envelopes.Add(Env);	
    }

	// sort in Decrementing order
	Envelopes.Sort([](const FCurveEnvelope& A, const FCurveEnvelope& B)
	{
		return A.PeakDistance < B.PeakDistance;
	});

	//resolve overlaps
	for (int32 i = 0; i < Envelopes.Num() - 1; ++i)
	{
		FCurveEnvelope& CurveA = Envelopes[i];
		FCurveEnvelope& CurveB = Envelopes[i + 1];

		if (CurveA.EndDistance < CurveB.StartDistance) continue;// not passed

		const float OverlapMid =(CurveA.EndDistance + CurveB.StartDistance) * 0.5f;

		float BiasAlpha = (CurveA.bAsymmetric || CurveB.bAsymmetric) ?
		MidpointAlphaOffset * FMath::Max(CurveA.Severity, CurveB.Severity) /* or */:  0.f;

		const float OverlapLen = CurveA.EndDistance - CurveB.StartDistance;
		const float AdjustedMid =OverlapMid + OverlapLen * 0.5f * BiasAlpha;

		CurveA.EndDistance   = FMath::Clamp(AdjustedMid, CurveA.PeakDistance, CurveB.PeakDistance);
		CurveB.StartDistance = CurveA.EndDistance;

		UE_LOG(SplineCorrectionHelper, Log,
			TEXT("Resolved curve overlap at %.2f (severity %.2f)"),
			CurveA.EndDistance, FMath::Max(CurveA.Severity, CurveB.Severity));
	}
	
	// if the road is closed
	if (bIsClosed && Envelopes.Num() > 1)
	{
		FCurveEnvelope& First = Envelopes[0];
		FCurveEnvelope& Last  = Envelopes.Last();

		if (Last.EndDistance > SplineLength)
		{
			Last.EndDistance -= SplineLength;
		}
		
		if (Last.EndDistance > First.StartDistance)
		{
			float OverlapMid = (Last.EndDistance + First.StartDistance) * 0.5f;
			
			float BiasAlpha  = (Last.bAsymmetric || First.bAsymmetric) ?
			MidpointAlphaOffset * FMath::Max(Last.Severity, First.Severity) : 0.f;
			
			float OverlapLen = Last.EndDistance - First.StartDistance;
			float AdjustedMid = OverlapMid + 0.5f * OverlapLen * BiasAlpha;

			Last.EndDistance = FMath::Clamp(AdjustedMid, Last.PeakDistance, First.PeakDistance);
			First.StartDistance = Last.EndDistance;
		}
	}
	
	// --- finalize segments ---
	float CurrentDist = 0.f;

	for (const FCurveEnvelope& Env : Envelopes)
	{
		// Straight Path case
		if (CurrentDist < Env.StartDistance)
		{
			FCurveSegment Straight;
			Straight.IsCurve = false;

			Straight.StartPoint.DistanceFromSlineOGPoint = CurrentDist;
			Straight.EndPoint.DistanceFromSlineOGPoint   = Env.StartDistance;

			Straight.StartPoint.Location =
				SourceSpline->GetLocationAtDistanceAlongSpline(CurrentDist, CoordSpace);
			Straight.EndPoint.Location =
				SourceSpline->GetLocationAtDistanceAlongSpline(Env.StartDistance, CoordSpace);

			OutSegments.Add(Straight);
		}

		// curve path case
		if (Env.StartDistance + KINDA_SMALL_NUMBER < Env.EndDistance)
		{
			FCurveSegment Curve;
			Curve.IsCurve = true;
			Curve.PeakPoint = *Env.Peak;

			Curve.StartPoint.DistanceFromSlineOGPoint = Env.StartDistance;
			Curve.EndPoint.DistanceFromSlineOGPoint   = Env.EndDistance;

			Curve.StartPoint.Location =
				SourceSpline->GetLocationAtDistanceAlongSpline(Env.StartDistance, CoordSpace);
			Curve.EndPoint.Location =
				SourceSpline->GetLocationAtDistanceAlongSpline(Env.EndDistance, CoordSpace);

			OutSegments.Add(Curve);
		}

		CurrentDist = Env.EndDistance;
	}

	// Tail case
	if (CurrentDist < SplineLength)
	{
		FCurveSegment Straight;
		Straight.IsCurve = false;

		Straight.StartPoint.DistanceFromSlineOGPoint = CurrentDist;
		Straight.EndPoint.DistanceFromSlineOGPoint   = SplineLength;

		Straight.StartPoint.Location =
			SourceSpline->GetLocationAtDistanceAlongSpline(CurrentDist, CoordSpace);
		Straight.EndPoint.Location =
			SourceSpline->GetLocationAtDistanceAlongSpline(SplineLength, CoordSpace);

		OutSegments.Add(Straight);
	}

	UE_LOG(SplineCorrectionHelper, Log,
		TEXT("DetectCurveSegmentsFromPeaks >> Generated %d segments"),
		OutSegments.Num());

	// rebuild the segment connection
	return RepairAndReattachSegments(SourceSpline,bIsClosed,Type, OutSegments);
}

bool USplineCorrectionHelper::MergeShortStraightSegments(const USplineComponent* SourceSpline, float MinDistance, bool bIsClosed,
	TArray<FCurveSegment>& Segments)
{
	if (!SourceSpline)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::MergeShortStraightSegments >> SourceSpline is null"));
		return false;
	}

	if (MinDistance <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("MergeShortStraightSegments >> "
			     "USplineCorrectionHelper::MinDistance must be > KINDA_SMALL_NUMBER. Current: [%f]"),
			MinDistance);
		return false;
	}

	if (Segments.Num() < 2)
	{
		UE_LOG(SplineCorrectionHelper, Warning,
			TEXT("USplineCorrectionHelper::MergeShortStraightSegments >> Nothing to merge. Segment count: [%d]"),
			Segments.Num());
		return true;
	}

	const float SplineLength = SourceSpline->GetSplineLength();

	auto GetSegmentLength = [](const FCurveSegment& Segment)
	{
		return Segment.EndPoint.DistanceFromSlineOGPoint
		     - Segment.StartPoint.DistanceFromSlineOGPoint;
	};

	int32 NumMergedSegments = 0;
	int32 i = 0;

	while (i < Segments.Num())
	{
		FCurveSegment& Straight = Segments[i];

		if (Straight.IsCurve || GetSegmentLength(Straight) >= MinDistance)
		{
			++i;
			continue;
		}

		const int32 Num = Segments.Num();
		int32 LeftIdx  = i - 1;
		int32 RightIdx = i + 1;

		if (bIsClosed)
		{
			LeftIdx  = (i - 1 + Num) % Num;
			RightIdx = (i + 1) % Num;
		}

		const bool bHasLeftCurve  = Segments.IsValidIndex(LeftIdx)  && Segments[LeftIdx].IsCurve;
		const bool bHasRightCurve = Segments.IsValidIndex(RightIdx) && Segments[RightIdx].IsCurve;

		if (!bHasLeftCurve && bHasRightCurve)
		{
			Segments[RightIdx].StartPoint = Straight.StartPoint;
			Segments.RemoveAt(i);
			++NumMergedSegments;
			continue;
		}

		if (bHasLeftCurve && !bHasRightCurve)
		{
			Segments[LeftIdx].EndPoint = Straight.EndPoint;
			Segments.RemoveAt(i);
			++NumMergedSegments;
			--i;
			continue;
		}

		if (bHasLeftCurve && bHasRightCurve)
		{
			const float StartDist = Straight.StartPoint.DistanceFromSlineOGPoint;
			const float EndDist   = Straight.EndPoint.DistanceFromSlineOGPoint;
			const float MidDist   = 0.5f * (StartDist + EndDist);

			FCurvePointData MidPoint;
			MidPoint.DistanceFromSlineOGPoint = MidDist;
			MidPoint.Location =
				SourceSpline->GetLocationAtDistanceAlongSpline(MidDist, ESplineCoordinateSpace::World);
			MidPoint.Tangent =
				SourceSpline->GetTangentAtDistanceAlongSpline(MidDist, ESplineCoordinateSpace::World);

			Segments[LeftIdx].EndPoint  = MidPoint;
			Segments[RightIdx].StartPoint = MidPoint;

			Segments.RemoveAt(i);
			++NumMergedSegments;
			--i;
			continue;
		}

		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("MergeShortStraightSegments >> "
			     "Short straight segment at index [%d] has no curve neighbors"),
			i);
		return false;
	}

	// Repair Distance alongside spline
	for (int32 Idx = 0; Idx < Segments.Num(); ++Idx)
	{
		FCurveSegment& Seg = Segments[Idx];

		Seg.StartPoint.DistanceFromSlineOGPoint =
			FMath::Clamp(Seg.StartPoint.DistanceFromSlineOGPoint, 0.f, SplineLength);

		Seg.EndPoint.DistanceFromSlineOGPoint =
			FMath::Clamp(Seg.EndPoint.DistanceFromSlineOGPoint, 0.f, SplineLength);

		// for non-closed spline
		if (!bIsClosed &&
			Seg.EndPoint.DistanceFromSlineOGPoint < Seg.StartPoint.DistanceFromSlineOGPoint)
		{
			Swap(Seg.StartPoint, Seg.EndPoint);
		}
	}

	// resort by distance
	Segments.Sort([](const FCurveSegment& A, const FCurveSegment& B)
	{
		return A.StartPoint.DistanceFromSlineOGPoint
		     < B.StartPoint.DistanceFromSlineOGPoint;
	});

	UE_LOG(SplineCorrectionHelper, Log,
		TEXT("MergeShortStraightSegments >> Merged %d short straight segments. Final segment count: %d"),
		NumMergedSegments,
		Segments.Num());

	// return the repaired segments
	return RepairAndReattachSegments(SourceSpline, bIsClosed, ELocationType::World, Segments);
}

bool USplineCorrectionHelper::RepairAndReattachSegments(const USplineComponent* SourceSpline, bool bIsClosed, ELocationType Type,
	TArray<FCurveSegment>& OutSegments)
{
	if (!SourceSpline || OutSegments.IsEmpty())
		return false;

	const float SplineLength = SourceSpline->GetSplineLength();

	const ESplineCoordinateSpace::Type CoordSpace =
		(Type == ELocationType::Local)? ESplineCoordinateSpace::Local: ESplineCoordinateSpace::World;

	// Sort by current start distance
	OutSegments.Sort([](const FCurveSegment& A, const FCurveSegment& B)
	{
		return A.StartPoint.DistanceFromSlineOGPoint <
			   B.StartPoint.DistanceFromSlineOGPoint;
	});
	
	//Preserve logical start for closed splines
	float RunningDistance = bIsClosed
		? OutSegments[0].StartPoint.DistanceFromSlineOGPoint
		: 0.f;

	// Rebuild distances SEQUENTIALLY!!!!! fuck
	for (int32 i = 0; i < OutSegments.Num(); ++i)
	{
		FCurveSegment& Seg = OutSegments[i];

		float OldLength =
			Seg.EndPoint.DistanceFromSlineOGPoint -
			Seg.StartPoint.DistanceFromSlineOGPoint;

		// Safety fallback (merges can cause tiny / invalid lengths)
		if (OldLength <= KINDA_SMALL_NUMBER)
		{
			OldLength = FVector::Distance(
				Seg.StartPoint.Location,
				Seg.EndPoint.Location);
		}

		OldLength = FMath::Max(OldLength, KINDA_SMALL_NUMBER);

		Seg.StartPoint.DistanceFromSlineOGPoint = RunningDistance;
		Seg.EndPoint.DistanceFromSlineOGPoint   = RunningDistance + OldLength;

		// Closed spline wrap safety
		if (bIsClosed)
		{
			Seg.EndPoint.DistanceFromSlineOGPoint =
				FMath::Fmod(Seg.EndPoint.DistanceFromSlineOGPoint, SplineLength);
		}
		else
		{
			Seg.EndPoint.DistanceFromSlineOGPoint =
				FMath::Min(Seg.EndPoint.DistanceFromSlineOGPoint, SplineLength);
		}

		// Re-evaluate transforms
		Seg.StartPoint.Location =
			SourceSpline->GetLocationAtDistanceAlongSpline(Seg.StartPoint.DistanceFromSlineOGPoint, CoordSpace);
		Seg.StartPoint.Tangent =
				SourceSpline->GetTangentAtDistanceAlongSpline(Seg.StartPoint.DistanceFromSlineOGPoint, CoordSpace);
		
		Seg.EndPoint.Location =
			SourceSpline->GetLocationAtDistanceAlongSpline(Seg.EndPoint.DistanceFromSlineOGPoint, CoordSpace);
		Seg.EndPoint.Tangent =
			SourceSpline->GetTangentAtDistanceAlongSpline(Seg.EndPoint.DistanceFromSlineOGPoint, CoordSpace);

		RunningDistance = Seg.EndPoint.DistanceFromSlineOGPoint;
	}

	return true;
}


bool USplineCorrectionHelper::GenerateSideSplinePoints(USplineComponent* Spline, const TArray<float>& Distances,
                                                       const TArray<FVector>& Tangents, const TArray<float>& RollDegrees, float OffsetDistance, bool bMirrorDirection,
                                                       const FVector& CustomOffsetDirection, const TArray<FVector>& BankedRightVectors, ELocationType Type, TArray<FVector>& OutSidePoints)
{
	//reset
	OutSidePoints.Empty();

	if (!Spline || Distances.IsEmpty() ||Tangents.Num() != Distances.Num() || RollDegrees.Num() != Distances.Num())
	{
		UE_LOG(SplineCorrectionHelper, Warning,
			TEXT("USplineCorrectionHelper::GenerateSideSplinePoints: Invalid input arrays"));
		return false;
	}
	
	const ESplineCoordinateSpace::Type CoordSpace =(Type == ELocationType::Local)?
	ESplineCoordinateSpace::Local: ESplineCoordinateSpace::World;
	
	for (int32 i = 0; i < Distances.Num(); ++i)
	{
		FVector Center = Spline->GetLocationAtDistanceAlongSpline(Distances[i], CoordSpace);
		FVector Tangent = Tangents[i];
		float Roll = RollDegrees[i];

		FVector OffsetDir;

		//  Use custom direction if provided
		if (!CustomOffsetDirection.IsNearlyZero())
		{
			OffsetDir = CustomOffsetDirection.GetSafeNormal();
		}
		//  Use precomputed banked right vectors if provided
		else if (BankedRightVectors.Num() == Distances.Num())
		{
			OffsetDir = BankedRightVectors[i];
			if (bMirrorDirection)
				OffsetDir *= -1.f;
		}
		// Otherwise, compute from tangent + roll (legacy fallback)
		else
		{
			FVector Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
			OffsetDir = Right.RotateAngleAxis(Roll, Tangent);
			if (bMirrorDirection)
				OffsetDir *= -1.f;
		}

		OutSidePoints.Add(Center + OffsetDir * OffsetDistance);
	}

	return true;
}


bool USplineCorrectionHelper::GetPeakPointFromSplineCurveSegment(const TArray<FVector>& SegmentPoints,
	FVector& OutPeakPoint, float& OutDeviation)
{
	//Reset
	OutPeakPoint=FVector::ZeroVector;
	OutDeviation=0.0f;

	if (SegmentPoints.Num()<2)
	{
		UE_LOG(SplineCorrectionHelper, Warning,
			TEXT("USplineCorrectionHelper::GetPeakPointFromSplineCurveSegment: Not enought segement points [%d]<2"),
			SegmentPoints.Num());
		return false;
	}

	const FVector& StartPoint= SegmentPoints[0];
	const FVector& EndPoint= SegmentPoints.Last();

	FVector LineDirection = (EndPoint - StartPoint).GetSafeNormal();
	float MaxDeviationDistance=0;//min value
	FVector PeakPoint=StartPoint;// the initial value
	for (const FVector& Point: SegmentPoints)
	{
		//project point onto the line
		FVector ProjectedPoint=StartPoint +FVector::DotProduct(Point - StartPoint, LineDirection) * LineDirection;
		// Compute the Deviation "From" the line
		float DeviationDistance=(Point-ProjectedPoint).Size();//distance
		
		if (DeviationDistance>MaxDeviationDistance)
		{
			MaxDeviationDistance=DeviationDistance;
			PeakPoint=Point;
		}
	}
	// loop done

	OutPeakPoint=PeakPoint;
	OutDeviation=MaxDeviationDistance;
	return true;
}


float USplineCorrectionHelper::ComputeCurvature(const FVector& Prev, const FVector& Curr, const FVector& Next)
{
	// Direction vectors between points
	/*FVector ForwardDirection = (Curr - Prev).GetSafeNormal();
	FVector BackwardDirection = (Next - Curr).GetSafeNormal();*/// this was for 3D curvature.
	// what this project need is 2D curve peek
	//TEMP
	//TODO: make a function to take CurveCompute Condition

	
	const FVector ForwardDirection = (Curr - Prev).GetSafeNormal();
	const FVector BackwardDirection = (Next - Curr).GetSafeNormal();

	// Angle between directions
	float Dot = FMath::Clamp(FVector::DotProduct(ForwardDirection, BackwardDirection), -1.f, 1.f);
	float Angle = FMath::Acos(Dot);

	// Approximate curvature: angle / distance between Prev and Next
	float Dist = (Next - Prev).Size();
	
	return Dist > 0.f ? Angle / Dist : 0.f;
}
void USplineCorrectionHelper::ComputeCurvePointDirectionsAndCurvature_Internal(const FVector& Prev, const FVector& Curr,
	const FVector& Next, FVector& OutForward, FVector& OutUp, FVector& OutRight, float& OutCurvature)
{
	// Forward direction along the spline
	//OutForward = (Next - Curr).GetSafeNormal(); // same here 2D, not 3D
	
	// what this project need is 2D curve peek
	//TEMP
	//TODO: make a function to take CurveCompute Condition
	OutForward = (Next - Curr).GetSafeNormal();// updated one
	
	// Up direction (you can choose world up or spline up)
	OutUp = FVector::UpVector;

	// Right direction
	OutRight = FVector::CrossProduct(OutForward, OutUp).GetSafeNormal();

	// Curvature
	OutCurvature = ComputeCurvature(Prev, Curr, Next);
}




FCurveSegmentEval USplineCorrectionHelper::EvaluateCurvePeak(const FCurvePeak& Peak, float ComfortCurvature,
	float MaxExpectedCurvature, float BaseEntranceDistance, float BaseExitDistance, float EntranceScaleMax,
	float ExitScaleMax, float MidpointAlphaOffset)
{
	FCurveSegmentEval Eval;
	Eval.Peak = &Peak;

	//Peak data
	Eval.PeakCurvature = Peak.Curvature;
	Eval.PeakDistance  = Peak.Point.DistanceFromSlineOGPoint;

	//Normalize curvature → severity (0..1)
	if (MaxExpectedCurvature <= ComfortCurvature)
	{
		// Safety fallback
		Eval.Severity = 0.f;
	}
	else
	{
		Eval.Severity = (Eval.PeakCurvature - ComfortCurvature)
			/ (MaxExpectedCurvature - ComfortCurvature);

		Eval.Severity = FMath::Clamp(Eval.Severity, 0.f, 1.f);
	}

	//  Decide symmetry
	// Rounded curves stay symmetric
	Eval.bAsymmetric = Eval.Severity > KINDA_SMALL_NUMBER;

	// Scale entrance / exit distances
	Eval.EntranceDistance =
		BaseEntranceDistance *
		FMath::Lerp(1.f, EntranceScaleMax, Eval.Severity);

	Eval.ExitDistance =
		BaseExitDistance *
		FMath::Lerp(1.f, ExitScaleMax, Eval.Severity);

	//  Midpoint bias (only meaningful for asymmetric curves)
	const float EffectiveMidpointShift =
		Eval.bAsymmetric
			? MidpointAlphaOffset * Eval.Severity
			: 0.f;

	// Compute start / end distances
	Eval.StartDistance =
		Eval.PeakDistance
		- Eval.EntranceDistance * (1.f + EffectiveMidpointShift);

	Eval.EndDistance =
		Eval.PeakDistance
		+ Eval.ExitDistance * (1.f - EffectiveMidpointShift);

	
	UE_LOG(SplineCorrectionHelper, Verbose,
		TEXT("EvaluateCurvePeak | Curvature=%.3f Severity=%.2f Asym=%d Start=%.2f Peak=%.2f End=%.2f"),
		Eval.PeakCurvature,
		Eval.Severity,
		Eval.bAsymmetric,
		Eval.StartDistance,
		Eval.PeakDistance,
		Eval.EndDistance);
	
	return Eval;
}

bool USplineCorrectionHelper::GetNeighborIndices(int32 Index, int32 Num, bool bIsClosed, int32& OutPrev, int32& OutNext)
{
	if (Num < 3)
		return false;
    
	if (bIsClosed)
	{
		OutPrev = (Index - 1 + Num) % Num;
		OutNext = (Index + 1) % Num;
		return true;
	}
    
	// Open spline
	if (Index <= 0 || Index >= Num - 1)
		return false;
    
	OutPrev = Index - 1;
	OutNext = Index + 1;
	return true;
}





