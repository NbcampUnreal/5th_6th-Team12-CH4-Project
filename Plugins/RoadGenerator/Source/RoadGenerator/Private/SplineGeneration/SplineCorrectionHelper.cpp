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
	//reset first
	OutSplinePoints.Reset();
	OutCorrectedDistance=0.f;

	if (!SourceSpline)//invlaid spline
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::ResampleSpline >> Invalid Spline"));
		return false;
	}
	if (DesiredSampleDistance <= 0.f)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::ResampleSpline >> invlaid SampleDistance [%f] <=0"),
			DesiredSampleDistance);
		return false;
	}

	const float SplineLength = SourceSpline->GetSplineLength();
	
	int32 SplineSegmentCount= FMath::FloorToInt(SplineLength/DesiredSampleDistance);
	SplineSegmentCount=FMath::Max(1, SplineSegmentCount);

	const int32 RequiredPoints= SplineSegmentCount+1;
	
	//Safety check
	if (RequiredPoints> MaxSamplePoints)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::ResampleSpline >> Too Many Spline Points! Segment Count[%d] > SafetyCount[%d]"),
			RequiredPoints,
			MaxSamplePoints);
		return false;
	}

	//Perfect division count
	OutCorrectedDistance = SplineLength / SplineSegmentCount;
	//local or world
	const ESplineCoordinateSpace::Type CoordSpace =(Type == ELocationType::Local)?
	ESplineCoordinateSpace::Local: ESplineCoordinateSpace::World;

	OutSplinePoints.Reserve(RequiredPoints);// reserve the room for new points

	TArray<FVector> Locations;
	Locations.Reserve(RequiredPoints);// to calculate the tangent, collect the location first
	for (int32 i = 0; i <= SplineSegmentCount; i++)
	{
		Locations.Add(SourceSpline->GetLocationAtDistanceAlongSpline(i * OutCorrectedDistance, CoordSpace));
	}
	
	for (int32 i = 0; i <= SplineSegmentCount; i++)
	{
		/*FVector Location = Locations[i];
		FVector Up       = SourceSpline->GetUpVectorAtDistanceAlongSpline(i * OutCorrectedDistance, CoordSpace);
		FVector Tangent  = ComputeTangentAtIndex(Locations, i, bIsClosed);
		FVector Right    = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

		FCurvePointData PointData;
		PointData.Location = Location;
		PointData.DistanceFromSlineOGPoint = Distance;
		PointData.ForwardDirection = Tangent.GetSafeNormal();
		PointData.UpDirection = Up.GetSafeNormal();
		PointData.RightDirection = Right;

		//Calculate tangent of the point
		PointData.Tangent = (i < SplineSegmentCount)?
		(SourceSpline->GetLocationAtDistanceAlongSpline(Distance + OutCorrectedDistance, CoordSpace) - Location).GetSafeNormal()
		//or
		: (Location - SourceSpline->GetLocationAtDistanceAlongSpline(Distance - OutCorrectedDistance, CoordSpace)).GetSafeNormal();



		// Curvature estimate
		if (i == 0 || i == SplineSegmentCount)
		{
			PointData.CurvatureValue = 0.f;
		}
		else
		{
			FVector PrevLoc =
				SourceSpline->GetLocationAtDistanceAlongSpline(Distance - OutCorrectedDistance, CoordSpace);
			FVector NextLoc =
				SourceSpline->GetLocationAtDistanceAlongSpline(Distance + OutCorrectedDistance, CoordSpace);

			FVector PrevDir = (Location - PrevLoc).GetSafeNormal();
			FVector NextDir = (NextLoc - Location).GetSafeNormal();
			// get the curvature value by 
			PointData.CurvatureValue =
				FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(PrevDir, NextDir)));
		}

		OutSplinePoints.Add(PointData);*/ // with no tangent calculation

		FVector Location = Locations[i]; //get from the precollected locations
		FVector Up = SourceSpline->GetUpVectorAtDistanceAlongSpline(i * OutCorrectedDistance, CoordSpace).GetSafeNormal();
		FVector Tangent = ComputeTangentAtIndex(Locations, i, bIsClosed);//use helper
		FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();// make a right vector by cross product

		FCurvePointData PointData;//set new point data
		PointData.Location = Location;
		PointData.DistanceFromSlineOGPoint = i * OutCorrectedDistance;
		PointData.ForwardDirection = Tangent;
		PointData.UpDirection = Up;
		PointData.RightDirection = Right;
		PointData.Tangent = Tangent;

		// Curvature calculation
		if (i == 0 || i == SplineSegmentCount)
			PointData.CurvatureValue = 0.f;
		else
			PointData.CurvatureValue = ComputeCurvature(Locations[i-1], Location, Locations[i+1]);

		OutSplinePoints.Add(PointData);
	}

	UE_LOG(SplineCorrectionHelper, Log,
			TEXT("USplineCorrectionHelper::ResampleSpline >> Resampling done. SplinePointCount[%d], Distance[%f]"),
			SplineSegmentCount, OutCorrectedDistance);
	return true;
}
#pragma endregion
//--------------------------------------------------------------------------------------------------------------------//

//==============================================  Smoothing  =========================================================//
#pragma region Smoothing

TArray<FCurvePointData> USplineCorrectionHelper::SmoothCurvePoints(const TArray<FCurvePointData>& InCurvePoints, float SmoothnessWeight,
	int32 IterationCount,bool bIsClosed)
{
	TArray<FCurvePointData> Result = InCurvePoints;

	// Extract locations for smoothing
	TArray<FVector> Locations;
	Locations.Reserve(Result.Num());
	for (auto& P : Result)
		Locations.Add(P.Location);

	// Iterative smoothing(only the location)
	for (int32 i = 0; i < IterationCount; ++i)
		SmoothCurvePoints_Internal(Locations, SmoothnessWeight,bIsClosed);

	// Update locations
	for (int32 i = 0; i < Result.Num(); ++i)
		Result[i].Location = Locations[i];

	// Recalculate curvature and directions after smoothing
	UpdateCurvePointsDirectionsAndCurvature(Result, bIsClosed);

	return Result;
}

void USplineCorrectionHelper::SmoothCurvePoints_Internal(TArray<FVector>& InCurvePointLocations, float SmoothnessWeight, bool bIsClosed)
{
	int32 NumPoints = InCurvePointLocations.Num();
	
	if (NumPoints < 3)
		return;

	TArray<FVector> Temp = InCurvePointLocations;

	for (int32 i = 0; i < NumPoints; ++i)
	{
		int32 PrevIdx, NextIdx;
		if (!GetNeighborIndices(i, NumPoints, bIsClosed, PrevIdx, NextIdx))
		{
			// Open spline endpoints
			Temp[i] = InCurvePointLocations[i];
			continue;
		}

		Temp[i] =
			InCurvePointLocations[i] * SmoothnessWeight +
			0.5f * (InCurvePointLocations[PrevIdx] + InCurvePointLocations[NextIdx]) *
			(1.f - SmoothnessWeight);
	}

	InCurvePointLocations = MoveTemp(Temp);
}
#pragma endregion
//--------------------------------------------------------------------------------------------------------------------//

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

//=======================================<< Peak Detection >>=========================================================//
#pragma region Peak Detection
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
#pragma region CurveDetection

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

	/*
	//TODO: =============================== Should it be exposed or stay as it is? ==================================
	const float ComfortCurvature= 0.02f;
	const float MaxExpectedCurvature= 0.15f;
	const float EntranceScaleMax= 2.0f;
	const float ExitScaleMax= 2.5f;
	*/ // all exposed as one struct FCurveEvaluationValues


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

		if (CurveA.EndDistance < CurveB.StartDistance) continue;// not passedsp

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

	const ESplineCoordinateSpace::Type CoordSpace =(Type == ELocationType::Local)?
    ESplineCoordinateSpace::Local: ESplineCoordinateSpace::World;
    	
	for (const FCurveEnvelope& Env : Envelopes)
	{
		// Straight segment
		if (CurrentDist < Env.StartDistance)
		{
			FCurveSegment Straight;
			Straight.IsCurve = false;
			Straight.StartPoint.Location =
				SourceSpline->GetLocationAtDistanceAlongSpline(CurrentDist, CoordSpace);
			Straight.EndPoint.Location =
				SourceSpline->GetLocationAtDistanceAlongSpline(Env.StartDistance, CoordSpace);

			OutSegments.Add(Straight);
		}

		// Curve segment
        if (Env.StartDistance + KINDA_SMALL_NUMBER < Env.EndDistance)
        {
            FCurveSegment Curve;
            Curve.IsCurve   = true;
            Curve.PeakPoint = *Env.Peak;
            Curve.StartPoint.Location = SourceSpline->GetLocationAtDistanceAlongSpline(Env.StartDistance, CoordSpace);
            Curve.EndPoint.Location   = SourceSpline->GetLocationAtDistanceAlongSpline(Env.EndDistance, CoordSpace);
            OutSegments.Add(Curve);
        }

        CurrentDist = Env.EndDistance;
	}

	// Tail straight
	if (CurrentDist < SplineLength)
	{
		FCurveSegment Straight;
		Straight.IsCurve = false;
		Straight.StartPoint.Location =
			SourceSpline->GetLocationAtDistanceAlongSpline(CurrentDist, CoordSpace);
		Straight.EndPoint.Location =
			SourceSpline->GetLocationAtDistanceAlongSpline(SplineLength, CoordSpace);

		OutSegments.Add(Straight);
	}
	
	// For closed loop, wrap first segment if needed
	if (bIsClosed && OutSegments.Num() > 1)
	{
		FCurveSegment& First = OutSegments[0];
		FCurveSegment& Last  = OutSegments.Last();

		if (Last.EndPoint.Location.Equals(First.StartPoint.Location, KINDA_SMALL_NUMBER))
		{
			Last.EndPoint = First.EndPoint;
			OutSegments.RemoveAt(0);
		}
	}

	UE_LOG(SplineCorrectionHelper, Log,
		TEXT("DetectCurveSegmentsFromPeaks >> Generated %d segments"),
		OutSegments.Num());

	return true;
}

bool USplineCorrectionHelper::SampleSplineDistances(USplineComponent* Spline, float SampleDistance,
                                                    int32 MaxSamplePoints, TArray<float>& OutDistances)
{
	OutDistances.Empty();//reset first

	if (!Spline || SampleDistance <= 0.f)
	{
		UE_LOG(SplineCorrectionHelper, Error,
			TEXT("USplineCorrectionHelper::SampleSplineDistances >> Invalid input"));
		return false;
	}

	const float SplineLength = Spline->GetSplineLength();
	int32 NumSamples = FMath::CeilToInt(SplineLength / SampleDistance);

	NumSamples = FMath::Min(NumSamples, MaxSamplePoints);

	for (int32 i = 0; i <= NumSamples; ++i)
	{
		float Distance = i * SampleDistance;
		Distance = FMath::Min(Distance, SplineLength);
		OutDistances.Add(Distance);
	}

	return true;
}

bool USplineCorrectionHelper::ComputeRoadRoll(USplineComponent* Spline, const TArray<float>& Distances,
	float BankStrength, float MaxBankDegrees,bool bIsClosed, TArray<float>& OutRollDegrees, TArray<FVector>& OutTangents,
	TArray<FVector>& OutBankedRightVectors)
{
	if (!Spline || Distances.Num() < 2)
		return false;

	const int32 Num = Distances.Num();

	OutRollDegrees.Reset(Num);
	OutTangents.Reset(Num);
	OutBankedRightVectors.Reset(Num);

	// Precompute tangents
	TArray<FVector> TangentsTemp;
	TangentsTemp.SetNum(Num);

	for (int32 i = 0; i < Num; ++i)
	{
		//TODO: Make A case of condition for the curvature evaluate condition
		// for now, just 2d curvature
		//TangentsTemp[i] =Spline->GetDirectionAtDistanceAlongSpline(Distances[i], ESplineCoordinateSpace::World);
		TangentsTemp[i] = Spline->GetDirectionAtDistanceAlongSpline(Distances[i],ESplineCoordinateSpace::World).GetSafeNormal();

		OutTangents.Add(TangentsTemp[i]);
	}

	// Compute roll per sample (NO smoothing)
	for (int32 i = 0; i < Num; ++i)
	{
		int32 PrevIdx, NextIdx;
		if (!GetNeighborIndices(i, Num, bIsClosed, PrevIdx, NextIdx))
		{
			// Open spline endpoint → no roll
			OutRollDegrees.Add(0.f);
			continue;
		}

		const FVector& PrevTangent = TangentsTemp[PrevIdx];
		const FVector& NextTangent = TangentsTemp[NextIdx];

		// Curvature direction (signed)
		const FVector CurvatureDir = FVector::CrossProduct(PrevTangent, NextTangent);
		const float SignedCurvature =
			FVector::DotProduct(CurvatureDir, FVector::UpVector);

		// Curvature magnitude
		const float Dot =
			FMath::Clamp(FVector::DotProduct(PrevTangent, NextTangent), -1.f, 1.f);

		const float CurvatureAngle = FMath::Acos(Dot);

		if (CurvatureAngle < KINDA_SMALL_NUMBER)
		{
			OutRollDegrees.Add(0.f);
			continue;
		}

		// Severity-driven roll
		const float Severity =
			FMath::Clamp(CurvatureAngle * BankStrength, 0.f, 1.f);

		const float Roll =
			Severity * MaxBankDegrees * FMath::Sign(SignedCurvature);

		OutRollDegrees.Add(Roll);
	}

	// Compute banked right vectors
	for (int32 i = 0; i < Num; ++i)
	{
		const FVector& Tangent = TangentsTemp[i];
		const FVector Up =Spline->GetUpVectorAtDistanceAlongSpline(Distances[i],ESplineCoordinateSpace::World);

		// Right = Tangent x Up (UE coordinate system)
		const FVector Right =FVector::CrossProduct(Tangent, Up).GetSafeNormal();
		const FVector BankedRight =Right.RotateAngleAxis(OutRollDegrees[i], Tangent);

		OutBankedRightVectors.Add(BankedRight);
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

FVector USplineCorrectionHelper::ComputeTangentAtIndex(const TArray<FVector>& Locations, int32 Index, bool bIsClosed)
{
	const int32 NumPoints = Locations.Num();
	if (NumPoints < 2)
		return FVector::ZeroVector;

	int32 PrevIdx, NextIdx;
	if (!GetNeighborIndices(Index, NumPoints, bIsClosed, PrevIdx, NextIdx))
	{
		// Open spline endpoints → fallback to forward/backward difference
		if (Index == 0)
		{
			return (Locations[1] - Locations[0]).GetSafeNormal();
		}
		return (Locations.Last() - Locations[NumPoints - 2]).GetSafeNormal();
	}

	// Central difference
	return (Locations[NextIdx] - Locations[PrevIdx]).GetSafeNormal();
}



