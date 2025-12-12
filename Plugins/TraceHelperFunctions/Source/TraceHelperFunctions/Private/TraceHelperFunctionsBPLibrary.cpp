// Copyright Epic Games, Inc. All Rights Reserved.

#include "TraceHelperFunctionsBPLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "Camera/PlayerCameraManager.h"
#include "CollisionQueryParams.h" 
#include "SceneView.h" // for FReversedZPerspectiveMatrix and view info
#include "MathPatterns/Public/MathPatternsBPLibrary.h" // Fibonacci pattern library
#include "GameFramework/Pawn.h" // Needed to find UCameraComponent on Pawn

// Helper function to reconstruct FCollisionQueryParams from Blueprint parameters
FCollisionQueryParams CreateTraceQueryParams(const TArray<AActor*>& IgnoreActors, bool bTraceComplex)
{
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TraceHelperFunctions), bTraceComplex);
    for (AActor* IgnoredActor : IgnoreActors)
    {
        Params.AddIgnoredActor(IgnoredActor);
    }
    return Params;
}

// =======================================================
// DEPROJECT HELPER (Manual Calculation)
// =======================================================

bool UTraceHelperFunctionsBPLibrary::DeprojectScreenToWorldForCamera(
    UCameraComponent* CameraComponent,
    const FVector2D& ScreenPosition,
    int32 ViewportSizeX,
    int32 ViewportSizeY,
    FVector& OutWorldOrigin,
    FVector& OutWorldDirection)
{
    if (!CameraComponent) return false;

    FMinimalViewInfo ViewInfo;
    CameraComponent->GetCameraView(0.f, ViewInfo);

    float AspectRatio = (float)ViewportSizeX / (float)ViewportSizeY;
    
    // 1. Build Projection Matrix
    FMatrix ProjectionMatrix = FReversedZPerspectiveMatrix(
        FMath::DegreesToRadians(ViewInfo.FOV) * 0.5f,
        AspectRatio,
        1.0f,
        GNearClippingPlane
    );

    // 2. Build View Matrix
    FMatrix ViewRotationMatrix = FInverseRotationMatrix(ViewInfo.Rotation);
    FMatrix ViewMatrix = ViewRotationMatrix * FTranslationMatrix(-ViewInfo.Location);
    
    // 3. Normalized Device Coordinates (NDC)
    // ScreenPosition is expected to be in virtual pixel space (0..ViewportSize)
    float NormalizedX = (ScreenPosition.X / ViewportSizeX) * 2.0f - 1.0f;
    float NormalizedY = 1.0f - (ScreenPosition.Y / ViewportSizeY) * 2.0f; 

    // 4. Unproject (Inverse View * Projection)
    FMatrix InvViewProjMatrix = (ViewMatrix * ProjectionMatrix).InverseFast();
    FVector4 Near(NormalizedX, NormalizedY, 0.0f, 1.0f);
    FVector4 Far(NormalizedX, NormalizedY, 1.0f, 1.0f); 

    FVector4 NearWorld = InvViewProjMatrix.TransformFVector4(Near);
    FVector4 FarWorld = InvViewProjMatrix.TransformFVector4(Far);

    // Perform W-divide
    if (FMath::IsNearlyZero(NearWorld.W) || FMath::IsNearlyZero(FarWorld.W)) return false;

    NearWorld /= NearWorld.W;
    FarWorld /= FarWorld.W;

    OutWorldOrigin = FVector(NearWorld);
    OutWorldDirection = (FVector(FarWorld) - OutWorldOrigin).GetSafeNormal();

    return true;
}

// =======================================================
// BASE: DEPROJECTED LINE TRACE (Camera Component)
// =======================================================

TArray<AActor*> UTraceHelperFunctionsBPLibrary::LineTraceFromCameraView(UObject* WorldContextObject,
    UCameraComponent* CameraComponent,
    const FVector2D& NormalizedScreenCoord, float TraceDistance,
    const TArray<TEnumAsByte<ECollisionChannel>>& TraceChannels, 
    const TArray<AActor*>& IgnoreActors, 
    bool bTraceComplex, 
    bool DebugDraw, float DebugThickness)
{
    TArray<AActor*> OutActors;
    if (!WorldContextObject || !CameraComponent) return OutActors;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return OutActors;
    
    FCollisionQueryParams QueryParams = CreateTraceQueryParams(IgnoreActors, bTraceComplex);

    const int32 VirtualViewportX = 1920; 
    const int32 VirtualViewportY = 1080;
    
    // Convert normalized [-1..1] coordinates to virtual pixel coordinates (0..ViewportSize)
    float PixelX = (NormalizedScreenCoord.X * 0.5f + 0.5f) * VirtualViewportX;
    float PixelY = (0.5f - NormalizedScreenCoord.Y * 0.5f) * VirtualViewportY; 
    
    FVector WorldOrigin, WorldDir;

    if (DeprojectScreenToWorldForCamera(
            CameraComponent, 
            FVector2D(PixelX, PixelY),
            VirtualViewportX, 
            VirtualViewportY, 
            WorldOrigin, 
            WorldDir))
    {
        FVector Start = WorldOrigin;
        FVector End = WorldOrigin + WorldDir * TraceDistance;

        for (auto Channel : TraceChannels)
        {
            FHitResult Hit;
            if (World->LineTraceSingleByChannel(Hit, Start, End, Channel, QueryParams))
            {
                if (Hit.GetActor() && !OutActors.Contains(Hit.GetActor()))
                    OutActors.Add(Hit.GetActor());
            }
        }

        if (DebugDraw)
        {
            DrawDebugLine(World, Start, End, FColor::Yellow, false, -1.f, 0, DebugThickness);
        }
    }

    return OutActors;
}
// =======================================================
// FIBONACCI SPIRAL TRACE (Camera Component)
// =======================================================

TArray<AActor*> UTraceHelperFunctionsBPLibrary::LineTraceInCoordRadius(UObject* WorldContextObject,
    UCameraComponent* CameraComponent, 
    const FVector2D& CenterCoord,
    float Radius, float Density, float TraceDistance,
    const TArray<TEnumAsByte<ECollisionChannel>>& TraceChannels,
    const TArray<AActor*>& IgnoreActors,
    bool bTraceComplex, 
    bool DebugDraw, float DebugThickness)
{
    TArray<AActor*> OutActors;
    if (!WorldContextObject || !CameraComponent) return OutActors;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return OutActors;
    
    FCollisionQueryParams QueryParams = CreateTraceQueryParams(IgnoreActors, bTraceComplex);

    const int32 VirtualViewportX = 1920; 
    const int32 VirtualViewportY = 1080;

    // Call the static Fibonacci function from your separate library
    TArray<FVector2D> Points = UMathPatternsBPLibrary::GenerateFibonacciSpiralPoints(CenterCoord, Radius, Density);

    for (const FVector2D& Pt : Points)
    {
        // 1. Convert normalized [-1,1] to virtual screen pixel coordinates
        float PixelX = (Pt.X * 0.5f + 0.5f) * VirtualViewportX;
        float PixelY = (0.5f - Pt.Y * 0.5f) * VirtualViewportY; 

        FVector WorldOrigin, WorldDir;
        
        // 2. Use the custom helper function to get the ray
        if (DeprojectScreenToWorldForCamera(
                CameraComponent, 
                FVector2D(PixelX, PixelY),
                VirtualViewportX, 
                VirtualViewportY, 
                WorldOrigin, 
                WorldDir))
        {
            FVector Start = WorldOrigin;
            FVector End = WorldOrigin + WorldDir * TraceDistance;

            for (auto Channel : TraceChannels)
            {
                FHitResult Hit;
                if (World->LineTraceSingleByChannel(Hit, Start, End, Channel, QueryParams))
                {
                    if (Hit.GetActor() && !OutActors.Contains(Hit.GetActor()))
                    {
                        OutActors.Add(Hit.GetActor());
                    }
                }
            }

            if (DebugDraw)
            {
                DrawDebugLine(World, Start, End, FColor::Cyan, false, -1.f, 0, DebugThickness);
            }
        }
    }

    return OutActors;
}

// =======================================================
// FRUSTUM TRACE (Mexican Pyramid)
// =======================================================

TArray<AActor*> UTraceHelperFunctionsBPLibrary::FrustumTrace_CameraPOV(UObject* WorldContextObject, 
    UCameraComponent* CameraComponent, 
    int32 GridResolution,
    float MaxDepth, float CullingDistance, 
    const TArray<TEnumAsByte<ECollisionChannel>>& TraceChannels,
    const TArray<AActor*>& IgnoreActors,
    bool bTraceComplex,
    bool DebugDraw, float DebugThickness, float DetectionExpandXY)
{
    TArray<AActor*> OutActors;
    if (!WorldContextObject || !CameraComponent || GridResolution <= 0) return OutActors;
    
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return OutActors;

    // --- Retrieve View Information from the CameraComponent ---
    FMinimalViewInfo ViewInfo;
    CameraComponent->GetCameraView(0.f, ViewInfo); 

    FCollisionQueryParams QueryParams = CreateTraceQueryParams(IgnoreActors, bTraceComplex);

    // Camera parameters (pulled from ViewInfo)
    FVector CamLocation = ViewInfo.Location; 
    FRotator CamRotation = ViewInfo.Rotation; 
    float FOV = ViewInfo.FOV;               
    float Aspect = CameraComponent->AspectRatio; 

    // View Angle Calculations
    float HalfFovRad = FMath::DegreesToRadians(FOV * 0.5f);
    float ViewHalfHeight = FMath::Tan(HalfFovRad);
    float ViewHalfWidth = ViewHalfHeight * Aspect;

    // Pyramid step calculations
    float DepthRange = MaxDepth - CullingDistance;
    float DepthStep = DepthRange / GridResolution;

    FVector Forward = CamRotation.Vector();
    FQuat BoxRotation = CamRotation.Quaternion();

    // Create stacked boxes (Mexican pyramid style)
    for (int32 i = 0; i < GridResolution; ++i)
    {
        float CurrentDepth = CullingDistance + (i * DepthStep);
        float MidDepth = CurrentDepth + (DepthStep * 0.5f);
        
        // Calculate box half-sizes at this depth (frustum scaling)
        float BoxHalfWidth = ViewHalfWidth * MidDepth + DetectionExpandXY;
        float BoxHalfHeight = ViewHalfHeight * MidDepth + DetectionExpandXY;
        float BoxHalfDepth = DepthStep * 0.5f;

        // Box center position
        FVector BoxCenter = CamLocation + Forward * MidDepth;
        
        // Box extent (half-sizes)
        FVector BoxExtent(BoxHalfDepth, BoxHalfWidth, BoxHalfHeight);

        // Perform Box Overlap/Sweep for each channel
        for (auto Channel : TraceChannels)
        {
            TArray<FHitResult> HitResults;
            FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);
            
            // Using SweepMulti (Start=End) to act as an Overlap
            FVector Start = BoxCenter; 
            FVector End = BoxCenter;
            
            if (World->SweepMultiByChannel(HitResults, Start, End, BoxRotation, Channel, BoxShape, QueryParams))
            {
                for (const FHitResult& Hit : HitResults)
                {
                    if (Hit.GetActor() && !OutActors.Contains(Hit.GetActor()))
                    {
                        OutActors.Add(Hit.GetActor());
                    }
                }
            }
        }

        // Debug draw each box
        if (DebugDraw)
        {
            TArray<FVector> BoxVertices = ComputeBoxVertices(BoxCenter, BoxExtent, CamRotation);
            FColor BoxColor = FColor::MakeRedToGreenColorFromScalar(static_cast<float>(i) / GridResolution);
            DrawDebugBoxFromVertices(World, BoxVertices, BoxColor, false, -1.f, 0, DebugThickness);
        }
    }
    return OutActors;
}

// =======================================================
// DEBUG DRAW HELPER FUNCTIONS
// =======================================================

TArray<FVector> UTraceHelperFunctionsBPLibrary::ComputeBoxVertices(const FVector& Center, const FVector& Extent,
    const FRotator& Rotation)
{
    TArray<FVector> Vertices;
    Vertices.SetNum(8);
    FQuat RotQuat = Rotation.Quaternion();

    Vertices[0] = Center + RotQuat.RotateVector(FVector(-Extent.X, -Extent.Y, -Extent.Z));
    Vertices[1] = Center + RotQuat.RotateVector(FVector( Extent.X, -Extent.Y, -Extent.Z));
    Vertices[2] = Center + RotQuat.RotateVector(FVector(-Extent.X,  Extent.Y, -Extent.Z));
    Vertices[3] = Center + RotQuat.RotateVector(FVector( Extent.X,  Extent.Y, -Extent.Z));
    Vertices[4] = Center + RotQuat.RotateVector(FVector(-Extent.X, -Extent.Y,  Extent.Z));
    Vertices[5] = Center + RotQuat.RotateVector(FVector( Extent.X, -Extent.Y,  Extent.Z));
    Vertices[6] = Center + RotQuat.RotateVector(FVector(-Extent.X,  Extent.Y,  Extent.Z));
    Vertices[7] = Center + RotQuat.RotateVector(FVector( Extent.X,  Extent.Y,  Extent.Z));

    return Vertices;
}

void UTraceHelperFunctionsBPLibrary::DrawDebugBoxFromVertices(UWorld* World, const TArray<FVector>& Vertices, const FColor& Color,
                                                   bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness)
{
    if (!World || Vertices.Num() != 8) return;

    const int32 EdgeIndices[12][2] = {
        {0,1}, {1,3}, {3,2}, {2,0}, 
        {4,5}, {5,7}, {7,6}, {6,4}, 
        {0,4}, {1,5}, {2,6}, {3,7}  
    };

    for (int i = 0; i < 12; ++i)
    {
        DrawDebugLine(
            World,
            Vertices[EdgeIndices[i][0]],
            Vertices[EdgeIndices[i][1]],
            Color,
            bPersistentLines,
            LifeTime,
            DepthPriority,
            Thickness
        );
    }
}

void UTraceHelperFunctionsBPLibrary::DrawDebugSphereOnDetectedActor(UWorld* World, const AActor* Actor, FDebugDrawInfo DebugDrawInfo, float Radius )
{
    if (!World || !Actor) return;

    DrawDebugSphere(
        World,
        Actor->GetActorLocation(),
        Radius,
        12, 
        DebugDrawInfo.Color.ToFColorSRGB(),
        false,
        DebugDrawInfo.DrawTime,
        DebugDrawInfo.DepthPriority,
        DebugDrawInfo.Thickness);
}

void UTraceHelperFunctionsBPLibrary::DrawDebugSphereOnDetectedActors(UWorld* World, const TArray<AActor*>& Actors, FDebugDrawInfo DebugDrawInfo, float Radius )
{
    if (Actors.IsEmpty()) return;
    
    for (const AActor* Actor : Actors)
    {
        DrawDebugSphereOnDetectedActor(World, Actor, DebugDrawInfo, Radius);
    }
}