// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "TraceHelperFunctionsBPLibrary.generated.h"

/**
 * Debug draw information structure for consistent visualization settings.
 */
USTRUCT(BlueprintType)
struct FDebugDrawInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugDrawInfo")
    FLinearColor Color = FLinearColor{1, 1, 1};
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugDrawInfo")
    float Thickness = 1.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugDrawInfo")
    float DrawTime = 1.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DebugDrawInfo")
    uint8 DepthPriority = 0;

    FDebugDrawInfo() = default;
};

/**
 * Function Library for advanced tracing operations (Fibonacci spiral trace, Frustum trace).
 */
UCLASS()
class TRACEHELPERFUNCTIONS_API UTraceHelperFunctionsBPLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    // --- Deproject Line Trace (Camera Component Base) ---
	
    UFUNCTION(BlueprintCallable, Category = "Detection|Trace")
    static TArray<AActor*> LineTraceFromCameraView( // <--- BASE FUNCTION
        UObject* WorldContextObject,
        UCameraComponent* CameraComponent,
        const FVector2D& NormalizedScreenCoord, // Normalized [-1..1]
        float TraceDistance,
        const TArray<TEnumAsByte<ECollisionChannel>>& TraceChannels,
        const TArray<AActor*>& IgnoreActors,
        bool bTraceComplex = false,
        bool DebugDraw = false,
        float DebugThickness = 1.f);

    // --- Deproject Radius Trace (Fibonacci - Camera Component) ---
    
    /** * Performs multiple line traces in a cone shape based on a Fibonacci spiral pattern 
     * originating from a screen coordinate radius, using a specific camera component. 
     */
    UFUNCTION(BlueprintCallable, Category = "Detection|Trace")
    static TArray<AActor*> LineTraceInCoordRadius(
        UObject* WorldContextObject,
        UCameraComponent* CameraComponent,
        const FVector2D& CenterCoord,
        float Radius,
        float Density,
        float TraceDistance,
        const TArray<TEnumAsByte<ECollisionChannel>>& TraceChannels,
        const TArray<AActor*>& IgnoreActors,
        bool bTraceComplex = false,
        bool DebugDraw = false,
        float DebugThickness = 1.f);
    
    // --- Frustum Trace (Mexican Pyramid) ---

    /**
     * Performs multiple box sweeps/overlaps to trace the camera's view frustum, 
     * creating a detection volume (like a Mexican pyramid/stacked boxes).
     */
    UFUNCTION(BlueprintCallable, Category = "Detection|Trace")
    static TArray<AActor*> FrustumTrace_CameraPOV(
       UObject* WorldContextObject,
       UCameraComponent* CameraComponent,
       int32 GridResolution,
       float MaxDepth,
       float CullingDistance,
       const TArray<TEnumAsByte<ECollisionChannel>>& TraceChannels,
       const TArray<AActor*>& IgnoreActors,
       bool bTraceComplex = false,
       bool DebugDraw = false,
       float DebugThickness = 1.f,
       float DetectionExpandXY = 0.f);

    // --- Helper & Debug Draw Functions ---

    /** Helper function to manually calculate World Origin and Direction from a screen-space position using a camera. */
    static bool DeprojectScreenToWorldForCamera(
        UCameraComponent* CameraComponent,
        const FVector2D& ScreenPosition,
        int32 ViewportSizeX,
        int32 ViewportSizeY,
        FVector& OutWorldOrigin,
        FVector& OutWorldDirection);

    /** Computes 8 box vertices from center, extent, and rotation. */
    static TArray<FVector> ComputeBoxVertices(
       const FVector& Center,
       const FVector& Extent,
       const FRotator& Rotation);

    /** Draws a debug box using 8 vertices. */
    static void DrawDebugBoxFromVertices(UWorld* World,
       const TArray<FVector>& Vertices,
       const FColor& Color,
       bool bPersistentLines,
       float LifeTime,
       uint8 DepthPriority,
       float Thickness);

    /** Draws a debug sphere on a single detected Actor. */
    static void DrawDebugSphereOnDetectedActor(
       UWorld* World,
       const AActor* Actor,
       FDebugDrawInfo DebugDrawInfo,
       float Radius);
    
    /** Draws a debug sphere on all detected Actors in the array. */
    static void DrawDebugSphereOnDetectedActors(
       UWorld* World,
       const TArray<AActor*>& Actors,
       FDebugDrawInfo DebugDrawInfo,
       float Radius);
};