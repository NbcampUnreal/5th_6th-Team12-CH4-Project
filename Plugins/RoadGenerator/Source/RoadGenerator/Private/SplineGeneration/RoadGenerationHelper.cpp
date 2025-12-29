// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineGeneration/RoadGenerationHelper.h"

#include "SplineGeneration/SplineCorrectionHelper.h"
#include "Components/SplineComponent.h"

//LOG
DEFINE_LOG_CATEGORY(RoadGenerationHelper);




bool URoadGenerationHelper::GetAlphaWeightByPeakAtDistance(float DistanceFromSplineOG, const TArray<FCurvePeak>& Peaks,
	float ComfortCurvature, float MaxExpectedCurvature, float MinInfluenceDistance, float MaxInfluenceDistance,
	float IntensityScale, float EntranceRatio, float ExitRatio, float EaseExponent, float& OutWeightAlpha)
{
	OutWeightAlpha = 0.f;

	if (Peaks.Num() == 0)
	{
		UE_LOG(RoadGenerationHelper, Warning,
			TEXT("URoadGenerationHelper::GetAlphaWeightByPeakAtDistance >> No peaks provided"));
		return false;
	}

	if (DistanceFromSplineOG < 0.f)
	{
		UE_LOG(RoadGenerationHelper, Warning,
			TEXT("URoadGenerationHelper::GetAlphaWeightByPeakAtDistance >> DistanceFromSplineOG < 0 (%.3f)"),
			DistanceFromSplineOG);
		return false;
	}

	float MaxAlpha = 0.f;
	bool bAnyPeakContributed = false;

	for (int32 PeakIndex = 0; PeakIndex < Peaks.Num(); ++PeakIndex)
	{
		const FCurvePeak& Peak = Peaks[PeakIndex];

		const float PeakDistance = Peak.Point.DistanceFromSlineOGPoint;
		const float Curvature = Peak.Curvature;

		float Severity = 0.f;
		if (!ComputeSeverity(
			Curvature,
			ComfortCurvature,
			MaxExpectedCurvature,
			Severity))
		{
			UE_LOG(RoadGenerationHelper, Log,
				TEXT("URoadGenerationHelper::GetAlphaWeightByPeakAtDistance >> "
				"Peak[%d]: ComputeSeverity failed (Curvature=%.4f)"),
				PeakIndex, Curvature);
			continue;
		}

		if (Severity <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		float EntranceDistance = 0.f;
		float ExitDistance = 0.f;
		if (!ComputeInfluenceDistances(
			Severity,
			MinInfluenceDistance,
			MaxInfluenceDistance,
			IntensityScale,
			EntranceRatio,
			ExitRatio,
			EntranceDistance,
			ExitDistance))
		{
			UE_LOG(RoadGenerationHelper, Log,
				TEXT("URoadGenerationHelper::GetAlphaWeightByPeakAtDistance >> "
				"Peak[%d]: ComputeInfluenceDistances failed (Severity=%.3f)"),
				PeakIndex, Severity);
			continue;
		}

		float Alpha = 0.f;
		if (!EvaluatePeakAlpha(
			DistanceFromSplineOG,
			PeakDistance,
			EntranceDistance,
			ExitDistance,
			EaseExponent,
			Alpha))
		{
			UE_LOG(RoadGenerationHelper, Log,
				TEXT("URoadGenerationHelper::GetAlphaWeightByPeakAtDistance >> Peak[%d]: EvaluatePeakAlpha failed"),
				PeakIndex);
			continue;
		}

		if (Alpha > 0.f)
		{
			bAnyPeakContributed = true;
			MaxAlpha = FMath::Max(MaxAlpha, Alpha);
		}
	}

	if (!bAnyPeakContributed)
	{
		UE_LOG(RoadGenerationHelper, Log,
			TEXT("URoadGenerationHelper::GetAlphaWeightByPeakAtDistance >> No peak influence at Distance %.3f"),
			DistanceFromSplineOG);
		return false;
	}

	OutWeightAlpha = FMath::Clamp(MaxAlpha, 0.f, 1.f);

	UE_LOG(RoadGenerationHelper, Log,
		TEXT("URoadGenerationHelper::GetAlphaWeightByPeakAtDistance >> Distance %.3f -> Alpha %.3f"),
		DistanceFromSplineOG,
		OutWeightAlpha);

	return true;
}

// internal helper
bool URoadGenerationHelper::ComputeSeverity(float Curvature, float ComfortCurvature, float MaxExpectedCurvature,
	float& OutSeverity)
{
	OutSeverity = 0.f;

	if (MaxExpectedCurvature <= ComfortCurvature)
	{
		return false;
	}

	if (Curvature <= ComfortCurvature)
	{
		OutSeverity = 0.f;
		return true;
	}

	const float Raw =
		(Curvature - ComfortCurvature) /
		(MaxExpectedCurvature - ComfortCurvature);

	OutSeverity = FMath::Clamp(Raw, 0.f, 1.f);
	return true;
}

bool URoadGenerationHelper::ComputeInfluenceDistances(float Severity, float MinInfluenceDistance,
	float MaxInfluenceDistance, float IntensityScale, float EntranceRatio, float ExitRatio, float& OutEntranceDistance,
	float& OutExitDistance)
{
	OutEntranceDistance = 0.f;
	OutExitDistance = 0.f;

	if (Severity <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	if (MinInfluenceDistance < 0.f || MaxInfluenceDistance < MinInfluenceDistance)
	{
		return false;
	}

	if (IntensityScale <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float TotalInfluence =
		FMath::Lerp(MinInfluenceDistance, MaxInfluenceDistance, Severity)
		* IntensityScale;

	const float RatioSum = EntranceRatio + ExitRatio;
	if (RatioSum <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float SafeEntranceRatio = EntranceRatio / RatioSum;
	const float SafeExitRatio = ExitRatio / RatioSum;

	OutEntranceDistance = TotalInfluence * SafeEntranceRatio;
	OutExitDistance = TotalInfluence * SafeExitRatio;

	return true;
}

bool URoadGenerationHelper::EvaluatePeakAlpha(float DistanceFromSplineOG, float PeakDistance, float EntranceDistance,
	float ExitDistance, float EaseExponent, float& OutWeightAlpha)
{
	OutWeightAlpha = 0.f;

	if (EntranceDistance <= KINDA_SMALL_NUMBER &&
		ExitDistance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	float Alpha = 0.f;

	if (DistanceFromSplineOG < PeakDistance)
	{
		if (EntranceDistance <= KINDA_SMALL_NUMBER)
		{
			return false;
		}

		const float Start = PeakDistance - EntranceDistance;
		Alpha = (DistanceFromSplineOG - Start) / EntranceDistance;
	}
	else
	{
		if (ExitDistance <= KINDA_SMALL_NUMBER)
		{
			return false;
		}

		Alpha = 1.f - (DistanceFromSplineOG - PeakDistance) / ExitDistance;
	}

	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

	if (EaseExponent > 1.f)
	{
		Alpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, EaseExponent);
	}

	OutWeightAlpha = Alpha;
	return true;
}

//--------------------------------------------------------------------------------------------------------------------//

//==============================================  road Banking  ======================================================//
#pragma region RoadBanking
bool URoadGenerationHelper::SampleSplineDistances(USplineComponent* Spline, float SampleDistance,
                                                    int32 MaxSamplePoints, TArray<float>& OutDistances)
{
	OutDistances.Empty();//reset first

	if (!Spline || SampleDistance <= 0.f)
	{
		UE_LOG(RoadGenerationHelper, Error,
			TEXT("URoadGenerationHelper::SampleSplineDistances >> Invalid input"));
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

bool URoadGenerationHelper::ComputeSmoothBankedRoll(const TArray<float>& SampledDistances,
	const TArray<FCurvePeak>& Peaks, float ComfortCurvature, float MaxExpectedCurvature, float MinInfluenceDistance,
	float MaxInfluenceDistance, float IntensityScale, float EntranceRatio, float ExitRatio, float EaseExponent,
	float MaxBankDegrees, TArray<float>& OutRollDegrees)
{
	OutRollDegrees.Reset();

	if (SampledDistances.Num() == 0 || Peaks.Num() == 0)
	{
		UE_LOG(RoadGenerationHelper, Warning, TEXT("ComputeSmoothBankedRoll >> Invalid input."));
		return false;
	}

	TArray<float> AlphaArray;
	AlphaArray.SetNum(SampledDistances.Num());

	// Step 1: Compute alpha per distance
	for (int32 i = 0; i < SampledDistances.Num(); ++i)
	{
		float Alpha = 0.f;
		URoadGenerationHelper::GetAlphaWeightByPeakAtDistance(
			SampledDistances[i], Peaks,
			ComfortCurvature, MaxExpectedCurvature,
			MinInfluenceDistance, MaxInfluenceDistance, IntensityScale,
			EntranceRatio, ExitRatio, EaseExponent,
			Alpha
		);
		AlphaArray[i] = Alpha;
	}

	// Step 2: Smooth the alpha array (forward pass)
	const float SmoothFactor = 0.2f; // tweakable: 0..1
	float PrevValue = AlphaArray[0];
	for (int32 i = 1; i < AlphaArray.Num(); ++i)
	{
		float Smoothed = FMath::Lerp(PrevValue, AlphaArray[i], SmoothFactor);
		AlphaArray[i] = Smoothed;
		PrevValue = Smoothed;
	}

	// Optional: backward pass for bidirectional smoothing
	PrevValue = AlphaArray.Last();
	for (int32 i = AlphaArray.Num() - 2; i >= 0; --i)
	{
		float Smoothed = FMath::Lerp(PrevValue, AlphaArray[i], SmoothFactor);
		AlphaArray[i] = Smoothed;
		PrevValue = Smoothed;
	}

	// Step 3: Compute final roll
	OutRollDegrees.SetNum(AlphaArray.Num());
	for (int32 i = 0; i < AlphaArray.Num(); ++i)
	{
		OutRollDegrees[i] = AlphaArray[i] * MaxBankDegrees;
	}

	return true;
}


/*bool URoadGenerationHelper::ComputeRoadRoll(USplineComponent* Spline, const TArray<float>& Distances,
                                            float BankStrength, float MaxBankDegrees,bool bIsClosed, TArray<float>& OutRollDegrees)
{
	if (!Spline || Distances.Num() < 2)
		return false;

	const int32 Num = Distances.Num();

	OutRollDegrees.Reset(Num);

	// Precompute tangents
	TArray<FVector> Tangents;
	Tangents.SetNum(Num);

	for (int32 i = 0; i < Num; ++i)
	{
		Tangents[i] =
			Spline->GetDirectionAtDistanceAlongSpline(Distances[i], ESplineCoordinateSpace::World).GetSafeNormal();
	}

	// Compute roll per sample
	for (int32 i = 0; i < Num; ++i)
	{
		int32 PrevIdx, NextIdx;
		if (!USplineCorrectionHelper::GetNeighborIndices(i, Num, bIsClosed, PrevIdx, NextIdx))
		{
			// Open spline endpoint → no roll
			OutRollDegrees.Add(0.f);
			continue;
		}

		const FVector& PrevTangent = Tangents[PrevIdx];
		const FVector& NextTangent = Tangents[NextIdx];

		// Signed curvature direction (around Up axis)
		const FVector CurvatureDir =FVector::CrossProduct(PrevTangent, NextTangent);

		const float SignedCurvature =FVector::DotProduct(CurvatureDir, FVector::UpVector);

		// Curvature magnitude
		const float Dot =
			FMath::Clamp(FVector::DotProduct(PrevTangent, NextTangent),-1.f, 1.f);

		const float CurvatureAngle = FMath::Acos(Dot);

		if (CurvatureAngle < KINDA_SMALL_NUMBER)
		{
			OutRollDegrees.Add(0.f);
			continue;
		}

		// Severity-driven roll
		const float Severity =FMath::Clamp(CurvatureAngle * BankStrength, 0.f, 1.f);
		const float Roll =Severity * MaxBankDegrees * FMath::Sign(SignedCurvature);

		OutRollDegrees.Add(Roll);
	}

	return true;
}*/

bool URoadGenerationHelper::ComputeRollFromPeaks(const TArray<float>& SampledDistances, const TArray<FCurvePeak>& Peaks,
	float ComfortCurvature, float MaxExpectedCurvature, float MinInfluenceDistance, float MaxInfluenceDistance,
	float IntensityScale, float EntranceRatio, float ExitRatio, float EaseExponent, float MaxBankDegrees,
	TArray<float>& OutRollDegrees)
{
	if (SampledDistances.Num() == 0 || Peaks.Num() == 0)
		return false;

	OutRollDegrees.Reset(SampledDistances.Num());

	for (float Distance : SampledDistances)
	{
		float Alpha = 0.f;
		if (GetAlphaWeightByPeakAtDistance(
				Distance,
				Peaks,
				ComfortCurvature,
				MaxExpectedCurvature,
				MinInfluenceDistance,
				MaxInfluenceDistance,
				IntensityScale,
				EntranceRatio,
				ExitRatio,
				EaseExponent,
				Alpha))
		{
			// Signed roll based on curvature direction of the peak
			// Here, take the peak with the highest influence
			const FCurvePeak* Peak = nullptr;
			float MaxPeakAlpha = 0.f;
			for (const FCurvePeak& P : Peaks)
			{
				float PeakAlpha = 0.f;
				GetAlphaWeightByPeakAtDistance(Distance, 
				{P},
				ComfortCurvature,
				MaxExpectedCurvature,
				MinInfluenceDistance,
				MaxInfluenceDistance,
				IntensityScale,
				EntranceRatio,
				ExitRatio,
				EaseExponent,
				PeakAlpha);
				
				if (PeakAlpha > MaxPeakAlpha)
				{
					MaxPeakAlpha = PeakAlpha;
					Peak = &P;
				}
			}

			float Roll = MaxPeakAlpha * MaxBankDegrees * FMath::Sign(Peak ? Peak->Curvature : 1.f);
			OutRollDegrees.Add(Roll);
		}
		else// not found, just 0
		{
			OutRollDegrees.Add(0.f);
		}
	}

	return true;
}

#pragma endregion
//--------------------------------------------------------------------------------------------------------------------//