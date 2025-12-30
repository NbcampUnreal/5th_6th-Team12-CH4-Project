// Fill out your copyright notice in the Description page of Project Settings.


#include "LandscapeGeneration/LandscapingHelper.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeEdit.h"// to edit the height of the landscape
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

//Log
DEFINE_LOG_CATEGORY(LogLandscapingHelper);

//internal helper for helper function
UWorld* GetValidWorld(UWorld* World)
{
#if WITH_EDITOR
    if (!World && GEditor)
    {
        if (GEditor->GetEditorWorldContext().World())
        {
            return GEditor->GetEditorWorldContext().World();
        }
        else
        {
            UE_LOG(LogLandscapingHelper, Warning,
                TEXT("ULandscapingHelper::GetValidWorld >> Editor world context is invalid."));
        }
    }
#endif

    if (!World)
    {
        UE_LOG(LogLandscapingHelper, Warning,
            TEXT("ULandscapingHelper::GetValidWorld >> No valid world provided."));
    }

    return World;
}

//============= Landscape height modify ==============================================================================//

void ULandscapingHelper::RayTraceCurvePoints(UWorld* World, const TArray<AActor*> IgnoringActors, TArray<FVector>& SamplePoints, float TraceDistance, FVector TargetLocationOffset,
    bool bDrawDebug, float DebugDrawTime)
{
    UWorld* UsingWorld = GetValidWorld(World);
    if (!UsingWorld)
    {
        UE_LOG(LogLandscapingHelper, Warning, 
        TEXT("ApplyCurvePointsToLandscape: Invalid world."));
        return;
    }

    int32 HitCount = 0;

    for ( int32 i=0; i<SamplePoints.Num(); ++i)
    {
        FVector& WorldLocation=SamplePoints[i];

        FVector HitLocation;
        if (RayTraceLandscape_Internal(
            UsingWorld,
            IgnoringActors,
            WorldLocation,
            TraceDistance,
            HitLocation,
            bDrawDebug,
            DebugDrawTime))
        {
            WorldLocation.Z = HitLocation.Z + TargetLocationOffset.Z;
            HitCount++;
        }
        else
        {
            UE_LOG(LogLandscapingHelper, Error,
                TEXT("ULandscapingHelper::RayTraceCurvePoints >> No landscape hit at any direction for point[%d]"),
                i);
        }
    }

    UE_LOG(LogLandscapingHelper, Log,
           TEXT("ULandscapingHelper::RayTraceCurvePoints >> Ray traced %d/%d points."),
           HitCount, SamplePoints.Num());
}
//Internal function
bool ULandscapingHelper::RayTraceLandscape_Internal(UWorld* World, const TArray<AActor*> IgnoringActors, const FVector& Start, float TraceDistance,
    FVector& OutHitLocation, bool bDrawDebug, float DebugDrawTime)
{
    if (!World) return false;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActors(IgnoringActors);
    Params.bReturnPhysicalMaterial = false;
    Params.bTraceComplex = false;

    // Landscape only trace
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel2);//landscape only trace
    //ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic); //-> test with world static
    // this plugin must use custom trace channel for the landscape, or else, it will not work


    for (int i=0;i<2;++i)// do it twice up and down
    {
        //get end location(down fist // and then up)
        FVector TraceEnd = (i == 0) ?
            Start - FVector(0, 0, TraceDistance): Start + FVector(0, 0, TraceDistance);
        bool IsDown = (i == 0);

        
        bool bHit=
            IgnoringActors[0]->GetWorld()->LineTraceSingleByObjectType(Hit, Start, TraceEnd, ObjectQueryParams, Params);

        if (!bHit)// no hit
        {
            FString DirectionString=IsDown? TEXT("Down"):TEXT("Up");
            
            UE_LOG(LogLandscapingHelper, Warning, 
            TEXT("ULandscapingHelper::RayTraceLandscape_Internal >> no hit for %s."),
            *DirectionString);
            
            continue;
        }
        
        //if (!Hit.GetActor() || !Hit.GetActor()->IsA(ALandscapeProxy::StaticClass()))//actor isn't valid nor ALandscapeProxy
        if (!Hit.GetActor() || !Hit.GetActor()->IsA(ALandscape::StaticClass()))
        {
            UE_LOG(LogLandscapingHelper, Warning,
                TEXT("ULandscapingHelper::RayTraceLandscape_Internal >> did not hit with landscape."));
            continue;
        }
    

        //valid hit found
        OutHitLocation = Hit.ImpactPoint;
        
        if (bDrawDebug)//if debug is enabled
        {
            FColor LineColor = IsDown ? FColor::Blue : FColor::Red;
            DrawDebugLine(
                World,
                Start,
                TraceEnd,
                LineColor,
                false,
                DebugDrawTime,
                0,
                2.f
            );
        }
        return true;
    }
    // no valid hit found
    return false;

    /*if (!World) return false;

    // Try to find a landscape in the world
    ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(UGameplayStatics::GetActorOfClass(World, ALandscapeProxy::StaticClass()));
    if (!Landscape)
    {
        UE_LOG(LogLandscapingHelper, Warning, TEXT("RayTraceLandscape_Internal >> No landscape found in the world."));
        return false;
    }

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.bReturnPhysicalMaterial = false;
    Params.bTraceComplex = false;

    // Use landscape's collision object type automatically
    ECollisionChannel LandscapeChannel = Landscape->GetRootComponent()->GetCollisionObjectType();
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(LandscapeChannel);

    // Trace directions: down first, then up
    TArray<FVector> TraceEnds = { Start - FVector(0, 0, TraceDistance),
                                  Start + FVector(0, 0, TraceDistance) };

    for (int i = 0; i < TraceEnds.Num(); ++i)
    {
        bool bIsDown = (i == 0);
        if (World->LineTraceSingleByObjectType(Hit, Start, TraceEnds[i], ObjectQueryParams, Params))
        {
            if (Hit.GetActor() && Hit.GetActor()->IsA(ALandscapeProxy::StaticClass()))
            {
                OutHitLocation = Hit.ImpactPoint;

                if (bDrawDebug)
                {
                    DrawDebugLine(
                        World,
                        Start,
                        TraceEnds[i],
                        bIsDown ? FColor::Blue : FColor::Red,
                        false,
                        DebugDrawTime,
                        0,
                        2.f
                    );
                }

                return true; // valid hit found
            }
        }
    }

    // No valid hit found
    if (bDrawDebug)
    {
        // Draw a debug line downward to visualize where it tried
        DrawDebugLine(World, Start, Start - FVector(0,0,TraceDistance), FColor::Red, false, DebugDrawTime, 0, 1.f);
    }

    return false;*/
}

bool ULandscapingHelper::ApplyCurvePointsToLandscape(UWorld* World, const TArray<FVector>& SamplePoints,
    int32 SmoothRadius, float BlurStrength)
{
    UWorld* UsingWorld = GetValidWorld(World);
    if (!UsingWorld)
    {
        UE_LOG(LogLandscapingHelper, Warning, 
        TEXT("ApplyCurvePointsToLandscape: Invalid world."));
        return false;
    }

    ALandscape* Landscape = Cast<ALandscape>(UGameplayStatics::GetActorOfClass(UsingWorld, ALandscape::StaticClass()));
    if (!Landscape)
    {
        UE_LOG(LogLandscapingHelper, Warning, 
        TEXT("ApplyCurvePointsToLandscape: No Landscape found in world."));
        return false;
    }

    ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        UE_LOG(LogLandscapingHelper, Warning, 
        TEXT("ApplyCurvePointsToLandscape: Landscape has no LandscapeInfo."));
        return false;
    }

    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);

    int32 AppliedCount = 0;
    for (const FVector& WorldLocation : SamplePoints)
    {
        int32 X, Y;
        uint16 Height;
        if (WorldToHeightmapCoord_Internal(Landscape, WorldLocation, X, Y, Height))
        {
            ApplyHeightWithBlur_Internal(LandscapeEdit, X, Y, Height, SmoothRadius, BlurStrength);
            AppliedCount++;
        }
        else
        {
            UE_LOG(LogLandscapingHelper, Warning, 
            TEXT("ApplyCurvePointsToLandscape: Failed to convert world location %s to heightmap coords"), 
            *WorldLocation.ToString());
        }
    }

    LandscapeEdit.Flush();
    UE_LOG(LogLandscapingHelper, Log, 
    TEXT("ApplyCurvePointsToLandscape: Applied %d/%d points to landscape."),
     AppliedCount, SamplePoints.Num());

    return AppliedCount > 0;
}
//Internal function
bool ULandscapingHelper::WorldToHeightmapCoord_Internal(ALandscape* Landscape, const FVector& WorldLocation, int32& OutX,
    int32& OutY, uint16& OutHeight)
{
    if (!Landscape)
    {
        UE_LOG(LogLandscapingHelper, Warning, 
        TEXT("WorldToHeightmapCoords: Invalid Landscape."));
        return false;
    }

    FVector LocalPos = WorldLocation - Landscape->GetActorLocation();
    OutX = FMath::RoundToInt(LocalPos.X / Landscape->GetActorScale().X);
    OutY = FMath::RoundToInt(LocalPos.Y / Landscape->GetActorScale().Y);

    if (OutX < 0 || OutY < 0)
    {
        UE_LOG(LogLandscapingHelper, Warning, 
        TEXT("WorldToHeightmapCoords: Computed coordinates out of bounds (%d, %d) for world location %s"),
         OutX, OutY, *WorldLocation.ToString());
        return false;
    }

    float MinZ = Landscape->GetActorLocation().Z;
    float ScaleZ = Landscape->GetRootComponent()->GetComponentScale().Z;
    OutHeight = FMath::Clamp((WorldLocation.Z - MinZ) / ScaleZ, 0.f, 65535.f);

    return true;
}

bool ULandscapingHelper::SmoothLandscapePoints(UWorld* World, const TArray<FVector>& SamplePoints,
    int32 SmoothRadius, float BlurStrength)
{
    UWorld* UsingWorld = GetValidWorld(World);
    if (!UsingWorld) return false;

    ALandscape* Landscape = Cast<ALandscape>(UGameplayStatics::GetActorOfClass(World, ALandscape::StaticClass()));
    if (!Landscape) return false;

    ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
    if (!LandscapeInfo) return false;

    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);

    int32 SmoothedCount = 0;
    for (const FVector& WorldLocation : SamplePoints)
    {
        int32 X, Y;
        uint16 Height;
        if (WorldToHeightmapCoord_Internal(Landscape, WorldLocation, X, Y, Height))
        {
            ApplyHeightWithBlur_Internal(LandscapeEdit, X, Y, Height, SmoothRadius, BlurStrength);
            SmoothedCount++;
        }
    }

    LandscapeEdit.Flush();
    UE_LOG(LogLandscapingHelper, Log,
        TEXT("ULandscapingHelper::SmoothLandscapePoints >> Smoothed %d/%d curve points on landscape"),
        SmoothedCount, SamplePoints.Num());
    
    return SmoothedCount > 0;
}



//Internal function
void ULandscapingHelper::ApplyHeightWithBlur_Internal(FLandscapeEditDataInterface& LandscapeEdit, int32 CenterX, int32 CenterY,
    uint16 TargetHeight, int32 SmoothRadius, float BlurStrength)
{
    for (int32 X = CenterX - SmoothRadius; X <= CenterX + SmoothRadius; ++X)
    {
        for (int32 Y = CenterY - SmoothRadius; Y <= CenterY + SmoothRadius; ++Y)
        {
            // Exact point remains
            if (X == CenterX && Y == CenterY)
            {
                LandscapeEdit.SetHeightData(X, Y, X, Y, &TargetHeight, 1, false);
                continue;
            }

            float Dist = FVector2D(X - CenterX, Y - CenterY).Size();
            float Weight = FMath::Clamp(1.f - Dist / (SmoothRadius + 1), 0.f, 1.f) * BlurStrength;

            uint16 CurrentHeight = 0;
            LandscapeEdit.GetHeightData(X, Y, X, Y, &CurrentHeight, 1);

            uint16 NewHeight = FMath::Lerp(CurrentHeight, TargetHeight, Weight);

            LandscapeEdit.SetHeightData(X, Y, X, Y, &NewHeight, 1, false);
        }
    }
}


