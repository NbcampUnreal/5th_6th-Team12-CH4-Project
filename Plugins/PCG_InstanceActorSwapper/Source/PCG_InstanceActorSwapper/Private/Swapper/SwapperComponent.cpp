// Fill out your copyright notice in the Description page of Project Settings.

#include "Swapper/SwapperComponent.h"
#include "Swapper/InstanceActorSwappingDataAsset.h" 
#include "Swapper/MeshActorSwappingFunctions.h" 
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Swapper/SwapperHelperComponent.h" 


USwapperComponent::USwapperComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USwapperComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USwapperComponent::OnSwapInstanceToActor_Implementation(UInstancedStaticMeshComponent* InstanceComponent,
    int32 InstanceIndex, TSubclassOf<AActor> ActorClassToSpawn, FName TargetName, FName ContextName)
{
    //empty here
}
void USwapperComponent::OnSwapActorToInstance_Implementation(AActor* SwappedActor)
{
    //empty there
}

bool USwapperComponent::SetSwapperDetectorComp(UPrimitiveComponent* NewDetector)
{
    if (!NewDetector)//invalid collision comp
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("USwapperComponent::SetSwapperDetectorComp >> Invalid Detector Comp"));
        SetComponentTickEnabled(false);
        return false;
    }
    SwappableDetector=NewDetector;
    //bind overlap react function
    SwappableDetector->OnComponentBeginOverlap.AddDynamic(this, &USwapperComponent::OnOverlapBegin);
    SwappableDetector->OnComponentEndOverlap.AddDynamic(this, &USwapperComponent::OnOverlapEnd);
    
    UE_LOG(SwapperComponent, Log,
        TEXT("USwapperComponent::BeginPlay >> Overlap events bound"));
    return true;
}




void USwapperComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                       int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult)
{
    UInstancedStaticMeshComponent* InstanceComponent = Cast<UInstancedStaticMeshComponent>(OtherComp);
    if (!InstanceComponent)// Cast failed
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("OnOverlapBegin >> OtherComp is not an ISMC --> Ignore."));
        return; 
    }

    if (OtherBodyIndex == INDEX_NONE)//Invalid Instance Index
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("OnOverlapBegin >> OtherBodyIndex is INDEX_NONE. Not an ISMC instance --> Ignore"));
        return;
    }
 
    if (SwappedActorsInfo.Contains(OtherActor))  // Actor already tracked
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("OnOverlapBegin >> Actor %s is already a swapped actor. Fuck it --> Ignore"),
            *OtherActor->GetName());
        return;
    }
    
    FSwappingMeshActorPair RulePair;
    FName TargetName;
    FName ContextName;

    // Use the Static Function Library for validation
    if (!UMeshActorSwappingFunctions::IsInstanceSwappable(
        InstanceComponent, 
        OtherBodyIndex, 
        SwapConfigDataAsset, // Pass the Data Asset
        InteractionTag,// Pass the Interaction Tag
        RulePair, 
        TargetName, 
        ContextName))
    {
        // Validation failed 
        return; 
    }

    // swap the instance to a matching actor
    ExecuteSwapInstanceToActor(
        InstanceComponent,
        OtherBodyIndex,
        RulePair.ActorClassToSpawn,
        TargetName,
        ContextName);
}

void USwapperComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex)
{
    FActorToInstanceSwapInfo* SwapInfoPtr = SwappedActorsInfo.Find(OtherActor);
    if (!SwapInfoPtr) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("OnOverlapEnd >> OtherActor %s is not a tracked swapped actor. Ignoring."), *OtherActor->GetName());
        return; 
    }
    
    SwapInfoPtr->bActorLeftRange = true;
    
    if (SwapInfoPtr->bInteractionFinished)
    {
        UE_LOG(SwapperComponent, Log,
            TEXT("USwapperComponent::OnOverlapEnd >> Interaction finished previously. Initiating immediate SwapActorToInstance."));
        ExecuteSwapActorToInstance(OtherActor);
    }
}

void USwapperComponent::ExecuteSwapInstanceToActor(UInstancedStaticMeshComponent* InstanceComponent, 
    int32 InstanceIndex, TSubclassOf<AActor> ActorClassToSpawn, FName TargetName, FName ContextName)
{
    AActor* NewActor = nullptr;
    FName InstancingComponentPath = NAME_None;
    FTransform OriginalTransform = FTransform::Identity;
    UStaticMesh* OriginalMesh = nullptr; 
    
    bool bSuccess = UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor(
        InstanceComponent, 
        InstanceIndex, 
        ActorClassToSpawn,
        NewActor,
        InstancingComponentPath,
        OriginalTransform,
        OriginalMesh
    );
    
    if (!bSuccess) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapInstanceToActor >> UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor failed. Aborting."));
        return; 
    }
    
    if (!NewActor)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapInstanceToActor >> Static function succeeded but returned null NewActor. Aborting."));
        return; 
    }

    // Make new swap info to pass on
    FActorToInstanceSwapInfo NewSwapInfo;
    NewSwapInfo.InstancingComponentPathName = InstancingComponentPath;
    NewSwapInfo.InstanceIndex = InstanceIndex;
    NewSwapInfo.TargetName = TargetName;
    NewSwapInfo.OriginalContextName = ContextName;
    NewSwapInfo.ChangedInstanceTransform = OriginalTransform; 
    NewSwapInfo.OriginalStaticMesh = OriginalMesh; 
    
    //Initialize the actor's swapping helper
    USwapperHelperComponent* Helper = NewActor->FindComponentByClass<USwapperHelperComponent>();
    if (!Helper)// actor has no swapping helper comp
    {
        // Cleanup
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapInstanceToActor >> NewActor %s is missing SwapperHelperComponent. Reverting ISMC swap and destroying actor."), *NewActor->GetName());
        UMeshActorSwappingFunctions::ExecuteSwapActorToInstance(
            InstancingComponentPath, InstanceIndex, OriginalMesh, OriginalTransform, false);
        NewActor->Destroy();
        return;
    }
    NewSwapInfo.SwapperHelper = Helper;
    Helper->InitializeSwapContext(TargetName, ContextName, InstanceComponent);

    // Bind back to the execution function (which is the entry point for swap back)
    Helper->OnReadyToSwapBack.AddDynamic(this, &USwapperComponent::ExecuteSwapActorToInstance);

    SwappedActorsInfo.Add(NewActor, NewSwapInfo);
    
    // notify on virtual function
    OnSwapInstanceToActor(InstanceComponent, InstanceIndex, ActorClassToSpawn, TargetName, ContextName);

    UE_LOG(SwapperComponent, Log,
        TEXT("ExecuteSwapInstanceToActor >> Swap to Actor complete. Actor: %s. OnSwap virtual hook triggered."),
        *NewActor->GetName());
}

void USwapperComponent::ExecuteSwapActorToInstance(AActor* SwappedActor)
{
    if (!SwappedActor) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> Invalid SwappedActor input (nullptr). Aborting."));
        return;
    }

    FActorToInstanceSwapInfo* SwapInfoPtr = SwappedActorsInfo.Find(SwappedActor);
    if (!SwapInfoPtr) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> Could not find swap info for actor %s. Aborting."),
            *SwappedActor->GetName());
        return;
    } 
    
    SwapInfoPtr->bInteractionFinished = true;
    if (!SwapInfoPtr->bActorLeftRange) 
    {
        UE_LOG(SwapperComponent, Log,
            TEXT("ExecuteSwapActorToInstance >> Interaction finished, but actor %s is still in range. Waiting for OnOverlapEnd."),
            *SwappedActor->GetName());
        return; 
    } 
    
    FActorToInstanceSwapInfo FinalSwapInfo = *SwapInfoPtr;
    UStaticMesh* OriginalMesh = FinalSwapInfo.OriginalStaticMesh.Get();
    
    if (!OriginalMesh)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> OriginalMesh reference is invalid for actor %s. Aborting cleanup."),
            *SwappedActor->GetName());
        SwappedActorsInfo.Remove(SwappedActor);
        SwappedActor->Destroy();
        return;
    }
    
    FName FinalContextName = FinalSwapInfo.SwapperHelper->GetFinalContextName();
    FinalSwapInfo.ChangedInstanceTransform = SwappedActor->GetActorTransform();

    FSwappingMeshActorPair FinalRulePair;
    UStaticMesh* FinalMeshAsset = nullptr;
    
    // Find final mesh asset
    
    //Context matches final state
    if (UMeshActorSwappingFunctions::FindRuleForMeshAndIndex(
        OriginalMesh, 0, SwapConfigDataAsset, FinalRulePair, FinalSwapInfo.TargetName, FinalContextName))
    {
        FinalMeshAsset = FinalRulePair.StaticMesh.LoadSynchronous();
        FinalSwapInfo.bDidChangeHappened = (FinalContextName != FinalSwapInfo.OriginalContextName);
        UE_LOG(SwapperComponent, Log, TEXT("ExecuteSwapActorToInstance >> Found rule using FinalContext: %s."), *FinalContextName.ToString());
    }
    //Fallback to Original Context if match failed
    else 
    {
        UE_LOG(SwapperComponent, Warning,
            TEXT("ExecuteSwapActorToInstance >> Final context rule lookup failed. Attempting fallback to OriginalContext: %s."),
            *FinalSwapInfo.OriginalContextName.ToString());
        if (UMeshActorSwappingFunctions::FindRuleForMeshAndIndex(
            OriginalMesh,
            0,
            SwapConfigDataAsset,
            FinalRulePair,
            FinalSwapInfo.TargetName,
            FinalSwapInfo.OriginalContextName))
        {
             FinalMeshAsset = FinalRulePair.StaticMesh.LoadSynchronous();
             FinalSwapInfo.bDidChangeHappened = false; 
             UE_LOG(SwapperComponent, Log,
                 TEXT("ExecuteSwapActorToInstance >> Fallback rule found."));
        }
    }
    
    if (!FinalMeshAsset)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance: CRITICAL - Failed to find any mesh for restoration for actor %s. Cleanup initiated."),
            *SwappedActor->GetName());
        SwappedActorsInfo.Remove(SwappedActor);
        SwappedActor->Destroy();
        return;
    }
    
    if (!UMeshActorSwappingFunctions::ExecuteSwapActorToInstance(
        FinalSwapInfo.InstancingComponentPathName,
        FinalSwapInfo.InstanceIndex,
        FinalMeshAsset,
        FinalSwapInfo.ChangedInstanceTransform,
        FinalSwapInfo.bDidChangeHappened
    ))
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> UMeshActorSwappingFunctions::ExecuteSwapActorToInstance failed for actor %s. Cleanup initiated."),
            *SwappedActor->GetName());
    }

    //Notify
    OnSwapActorToInstance(SwappedActor);

    // Cleanup
    FinalSwapInfo.SwapperHelper->OnReadyToSwapBack.RemoveDynamic(this, &USwapperComponent::ExecuteSwapActorToInstance);
    SwappedActorsInfo.Remove(SwappedActor);
    
    UE_LOG(SwapperComponent, Log,
        TEXT("ExecuteSwapActorToInstance >> Swap to ISMC complete. Actor %s destroyed."),
        *SwappedActor->GetName());

    SwappedActor->Destroy();
}