// Copyright Epic Games, Inc. All Rights Reserved.

#include "MathPatternsBPLibrary.h"

UMathPatternsBPLibrary::UMathPatternsBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UMathPatternsBPLibrary::MathPatternsSampleFunction(float Param)
{
	return -1;
}

TArray<FVector2D> UMathPatternsBPLibrary::GenerateFibonacciSpiralPoints(const FVector2D& Center, float Radius,
	float Density)
{
	TArray<FVector2D> Points;
	if (Density <= 0.f) return Points;

	const float GoldenAngle = 2.399963f; // ~137.5 degrees

	// Estimate number of points from density and area
	int32 NumPoints = FMath::CeilToInt(PI * Radius * Radius * Density);

	Points.Reserve(NumPoints);

	for (int32 i = 0; i < NumPoints; ++i)
	{
		float t = (float)i / (float)NumPoints;
		float r = Radius * FMath::Sqrt(t);  // polar radius
		float theta = i * GoldenAngle;      // spiral angle

		// Polar to Cartesian
		float X = r * FMath::Cos(theta);
		float Y = r * FMath::Sin(theta);

		Points.Add(Center + FVector2D(X, Y));
	}

	return Points;
}

TArray<FVector2D> UMathPatternsBPLibrary::CalculateHexagonVertices(float Radius)
{
	TArray<FVector2D> Vertices;
	Vertices.Reserve(6);

	// Flat-Top Hexagon: The first vertex is on the right (0 degrees)
	// The angle step between vertices is 60 degrees (M_PI / 3 radians)
	// The starting angle for a Flat-Top hex is 0 degrees (0 radians)
	// Vertices are calculated counter-clockwise.

	for (int i = 0; i < 6; ++i)
	{
		// Angle in radians: (60 degrees * i) + 0 degrees start
		float AngleRad = (PI / 3.0f) * i;

		// Calculate X and Y coordinates using polar to cartesian conversion
		float X = Radius * FMath::Cos(AngleRad);
		float Y = Radius * FMath::Sin(AngleRad);

		// Add the vertex to the array
		Vertices.Add(FVector2D(X, Y));
	}

	return Vertices;
}

TArray<FVector2D> UMathPatternsBPLibrary::PopulateHexagonCenters(float Radius, int32 GridWidth, int32 GridHeight)
{
	TArray<FVector2D> CenterPoints;
    
	// 1. Calculate the core spacing based on the Radius (Flat-Top Hex)
	const float HorizontalSpacing = Radius * FMath::Sqrt(3.0f); // W
	const float VerticalSpacing = Radius * 1.5f;                // H

	// 2. Loop through Axial Coordinates (q for columns, r for rows)
	// Assuming the center of the grid is (0, 0)
	for (int32 r = -GridHeight; r <= GridHeight; ++r) // Row index (r)
	{
		for (int32 q = -GridWidth; q <= GridWidth; ++q) // Column index (q)
		{
			// 3. Convert Axial (q, r) to World Space (X, Y)
			float X = HorizontalSpacing * (q + (0.5f * r));
			float Y = VerticalSpacing * r;

			CenterPoints.Add(FVector2D(X, Y));
		}
	}

	return CenterPoints;
}

