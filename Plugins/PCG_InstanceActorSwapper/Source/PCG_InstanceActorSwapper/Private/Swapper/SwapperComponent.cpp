// Fill out your copyright notice in the Description page of Project Settings.

#include "Swapper/SwapperComponent.h"
#include "Swapper/InstanceActorSwappingDataAsset.h" 
#include "Swapper/ISMC_SwapperFunctions.h" 
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
void USwapperComponent::OnSwapActorToInstance_Implementation(AActor* SwappedActor, FTransform SwappedTransform, FName SwappedContext)
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
        TEXT("USwapperComponent::SetSwapperDetectorComp >> Overlap events bound"));
    return true;
}

bool USwapperComponent::SetInteractionRangeComp(UPrimitiveComponent* InteractionComp)
{
    if (!InteractionComp)//invalid collision comp
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("USwapperComponent::SetInteractionRangeComp >> Invalid InteractionComp"));
        return false;
    }

    InteractionDetector=InteractionComp;
    InteractionDetector->OnComponentBeginOverlap.AddDynamic(this, &USwapperComponent::OnInteractionOverlapBegin);
    
    UE_LOG(SwapperComponent, Log,
       TEXT("USwapperComponent::SetInteractionRangeComp >> InteractionComp settled"));
    return true;
}


void USwapperComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                       int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult)
{
    UInstancedStaticMeshComponent* InstanceComponent = Cast<UInstancedStaticMeshComponent>(OtherComp);
    if (!InstanceComponent || OtherBodyIndex == INDEX_NONE)
    {
        UE_LOG(SwapperComponent, Verbose, TEXT("OnOverlapBegin >> OtherComp is not a valid ISMC instance. Ignoring."));
        return; 
    }
 
    // Check if OtherActor already has a helper component
    if (OtherActor->FindComponentByClass<USwapperHelperComponent>())
    {
        // This is the case where a Swapped Actor overlaps a new Swapper Component. ignore it
        UE_LOG(SwapperComponent, Log, TEXT("OnOverlapBegin >> Actor %s is a swapped actor (has helper). Ignoring BeginOverlap on this component."),
            *OtherActor->GetName());
        return;
    }
    
    FSwappingMeshActorPair RulePair;
    FName TargetName;
    FName ContextName;
    int32 VariationIndex = 0; 
    
    // Check if swappable and retrieve the rule
    if (!UISMC_SwapperFunctions::IsInstanceSwappable(
        InstanceComponent, OtherBodyIndex, SwapConfigDataAsset, InteractionTag,
        RulePair, TargetName, ContextName, VariationIndex))
    {
        return; 
    }
    
    // no for mesh to mesh swap case
    if (RulePair.bIsDirectInstanceSwap)
    {
        UE_LOG(SwapperComponent, Verbose, TEXT("OnOverlapBegin >> Rule is a Direct Swap. Ignoring in broad detector."));
        return;
    }

    // Check for valid Actor class for the full swap
    if (!RulePair.ActorClassToSpawn)
    {
        UE_LOG(SwapperComponent, Warning, TEXT("OnOverlapBegin: Full Swap intended but no ActorClassToSpawn found. Skipping."));
        return;
    }
    
    // Execute the full swap (Instance to Actor)
    ExecuteSwapInstanceToActor(
        InstanceComponent,
        OtherBodyIndex,
        RulePair.ActorClassToSpawn,
        TargetName,
        ContextName);
}

void USwapperComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!GetOwner())
    {
        UE_LOG(SwapperComponent, Verbose,
           TEXT("USwapperComponent::OnOverlapEnd >> Invalid Owner."));
        return; 
    }
    if (OtherActor->IsPendingKillPending())
    {
        UE_LOG(SwapperComponent, Verbose,
           TEXT("USwapperComponent::OnOverlapEnd >> So close, it will destroyed after this tick"));
        return; 
    }
    
    // Find the Helper component directly on the exiting Actor
    USwapperHelperComponent* Helper = OtherActor->FindComponentByClass<USwapperHelperComponent>();
    if (!Helper) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("OnOverlapEnd >> Actor %s is not a tracked swapped actor (no helper). Ignoring."),
            *OtherActor->GetName());
        return; 
    }
    
    bool bTaskIsRunning = Helper->IsProcessingTask();
    
    if (!bTaskIsRunning)
    {
        UE_LOG(SwapperComponent, Log,
            TEXT("USwapperComponent::OnOverlapEnd >> Task inactive/finished. Initiating immediate SwapActorToInstance (cleanup)."));
        // Call the helper's signal, which triggers the delegate, which calls ExecuteSwapActorToInstance_DelegateEntry

        FTransform CurrentTransform=Helper->GetCurrentTransform();
        FName Context = Helper->GetFinalContextName();
        Helper->SignalSwapCompletion(CurrentTransform, Context); 
    }
    else
    {
        UE_LOG(SwapperComponent, Log,
            TEXT("USwapperComponent::OnOverlapEnd >> Task is active. Waiting for SignalSwapCompletion from helper on %s."),
            *OtherActor->GetName());
    }
}

void USwapperComponent::OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UInstancedStaticMeshComponent* InstanceComponent = Cast<UInstancedStaticMeshComponent>(OtherComp);
    if (!InstanceComponent || OtherBodyIndex == INDEX_NONE) return; 

    // Skip if already swapped (Actor has a helper component)
    if (OtherActor->FindComponentByClass<USwapperHelperComponent>()) return;

    FSwappingMeshActorPair RulePair;
    FName TargetName, ContextName;
    int32 VariationIndex = 0; 
    
    // Check if swappable and retrieve the rule
    if (!UISMC_SwapperFunctions::IsInstanceSwappable(
        InstanceComponent, OtherBodyIndex, SwapConfigDataAsset, InteractionTag,
        RulePair, TargetName, ContextName, VariationIndex))
    {
        return; 
    }
    
    if (!RulePair.bIsDirectInstanceSwap)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("InteractionOverlap: Rule is a Full Actor Swap. Ignoring in small detector."));
        return;
    }
    
    UStaticMesh* TargetMesh = RulePair.ResultStaticMesh.LoadSynchronous();
    if (!TargetMesh)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("InteractionOverlap: Direct Swap rule found but ResultStaticMesh is NULL. Aborting."));
        return;
    }

    FTransform InstanceTransform;
    if (!InstanceComponent->GetInstanceTransform(OtherBodyIndex, InstanceTransform, true)) return; 

    // Execute the fast swap and exit.
    UISMC_SwapperFunctions::ExecuteSwapInstancedMesh_AtoB(
        InstanceComponent,
        OtherBodyIndex,
        TargetMesh,
        InstanceTransform
    );
    
    UE_LOG(SwapperComponent, Log,
        TEXT("InteractionOverlap >> Direct Instance-to-Instance swap completed."));
}

void USwapperComponent::ExecuteSwapInstanceToActor(UInstancedStaticMeshComponent* InstanceComponent, 
                                                   int32 InstanceIndex, TSubclassOf<AActor> ActorClassToSpawn, FName TargetName, FName ContextName)
{
    if (!GetOwner())
    {
        UE_LOG(SwapperComponent, Error,
           TEXT("USwapperComponent::ExecuteSwapInstanceToActor >> Invalid Owner."));
        return;
    }
    
    AActor* NewActor = nullptr;
    FName InstancingComponentPath = NAME_None;
    FTransform OriginalTransform = FTransform::Identity;
    UStaticMesh* OriginalMesh = nullptr; 
    
    // 1. Perform low-level swap (Remove instance, spawn actor)
    bool bSuccess = UISMC_SwapperFunctions::ExecuteSwapInstanceToActor(
        InstanceComponent, 
        InstanceIndex, 
        ActorClassToSpawn,
        NewActor,
        InstancingComponentPath,
        OriginalTransform,
        OriginalMesh
    );
    
        if (!bSuccess || !NewActor) 
    {
        UE_LOG(SwapperComponent, Error, TEXT("ExecuteSwapInstanceToActor >> Low-level swap failed. Aborting."));
        return; 
    }
    
    USwapperHelperComponent* Helper = NewActor->FindComponentByClass<USwapperHelperComponent>();
    if (!Helper)
    {
        // CRITICAL FAILURE: Cleanup (Revert ISMC swap and destroy actor)
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapInstanceToActor >> NewActor %s is missing SwapperHelperComponent. Reverting."), *NewActor->GetName());
        UISMC_SwapperFunctions::ExecuteSwapActorToInstance(
            InstancingComponentPath, InstanceIndex, OriginalMesh, OriginalTransform, false);
        NewActor->Destroy();
        return;
    }

    // Initialize helper with persistent data (New signature)
    Helper->InitializeSwapContext(
        ContextName,// InContextName (Initial Context)
        0,// InContextVariationIndex (Placeholder: We only use Index 0 for swap-out matching)
        InstanceComponent,
        InstancingComponentPath,
        InstanceIndex,
        OriginalMesh
    );
    
    Helper->SetTargetName(TargetName); 
    Helper->OnReadyToSwapBack.AddDynamic(this, &USwapperComponent::ExecuteSwapActorToInstance_DelegateEntry);

    // 4. Notify on virtual function
    OnSwapInstanceToActor(InstanceComponent, InstanceIndex, ActorClassToSpawn, TargetName, ContextName);

    UE_LOG(SwapperComponent, Log,
        TEXT("ExecuteSwapInstanceToActor >> Swap complete. Actor: %s. Helper Initialized."),
        *NewActor->GetName());
}


void USwapperComponent::ExecuteSwapActorToInstance(AActor* SwappedActor, const FTransform& FinalTransform, FName FinalContextName)
{
    if (!GetOwner())
    {
        UE_LOG(SwapperComponent, Error,
           TEXT("USwapperComponent::ExecuteSwapInstanceToActor >> Invalid Owner."));
        return;
    }
    
    if (!SwappedActor) 
    {
        UE_LOG(SwapperComponent, Error, TEXT("ExecuteSwapActorToInstance >> Invalid SwappedActor input (nullptr). Aborting."));
        return;
    }

    USwapperHelperComponent* Helper = SwappedActor->FindComponentByClass<USwapperHelperComponent>();
    if (!Helper) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> Could not find SwapperHelper on actor %s. Cannot retrieve info. Aborting."),
            *SwappedActor->GetName());
        SwappedActor->Destroy();
        return;
    } 

    //  Retrieve persistent data from the Helper
    const FName InstancingComponentPath = Helper->GetISMCPathName();
    const int32 InstanceIndex = Helper->GetInstanceIndex();
    UStaticMesh* OriginalMesh = Helper->GetPreviousMesh().LoadSynchronous(); // Load soft pointer
    const FName TargetName = Helper->GetTargetName();
    const bool bDidChangeHappened = Helper->HasChangeOccurred();
    
    if (!OriginalMesh)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> CRITICAL: OriginalMesh reference is invalid for actor %s. Aborting cleanup."),
            *SwappedActor->GetName());
        SwappedActor->Destroy();
        return;
    }
    
    FSwappingMeshActorPair FinalRulePair;
    UStaticMesh* FinalMeshAsset = nullptr;

    // Find the final mesh asset using the actor's current state (FinalContextName)
    if (UISMC_SwapperFunctions::FindRuleForContextAndTarget(
            SwapConfigDataAsset, 
            TargetName, 
            FinalContextName, 
            FinalRulePair))
    {
        // Use the new, correctly named field: ResultStaticMesh
        FinalMeshAsset = FinalRulePair.ResultStaticMesh.LoadSynchronous(); 
        
        UE_LOG(SwapperComponent, Log,
            TEXT("ExecuteSwapActorToInstance >> Found final mesh using FinalContext: %s and Mesh: %s."),
            *FinalContextName.ToString(),
            *FinalMeshAsset->GetName());
    }
    else 
    {
        // CRITICAL FALLBACK: If the final state wasn't in the Data Asset, use the Original Mesh itself
        UE_LOG(SwapperComponent, Warning,
            TEXT("ExecuteSwapActorToInstance >> Final context rule lookup failed for %s/%s. Falling back to Original Mesh: %s."),
            *TargetName.ToString(),
            *FinalContextName.ToString(),
            *OriginalMesh->GetName());
        
        FinalMeshAsset = OriginalMesh; 
    }
    
    if (!FinalMeshAsset)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance: CRITICAL - Failed to determine any mesh for restoration for actor %s. Cleanup initiated."),
            *SwappedActor->GetName());
        SwappedActor->Destroy();
        return;
    }
    
    // 3. Execute the low-level swap back
    if (!UISMC_SwapperFunctions::ExecuteSwapActorToInstance(
        InstancingComponentPath,
        InstanceIndex,
        FinalMeshAsset,
        FinalTransform,
        bDidChangeHappened
    ))
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> UMeshActorSwappingFunctions::ExecuteSwapActorToInstance failed for actor %s. Cleanup initiated."),
            *SwappedActor->GetName());
    }

    // Notify
    OnSwapActorToInstance(SwappedActor,FinalTransform,FinalContextName);
    
    // Cleanup
    // Unbind the delegate before destroying the actor
    Helper->OnReadyToSwapBack.RemoveDynamic(this, &USwapperComponent::ExecuteSwapActorToInstance_DelegateEntry);

    UE_LOG(SwapperComponent, Log,
        TEXT("ExecuteSwapActorToInstance >> Swap to ISMC complete. Actor %s destroyed."),
        *SwappedActor->GetName());

    SwappedActor->Destroy();
}

void USwapperComponent::ExecuteSwapActorToInstance_DelegateEntry(AActor* SwappedActor, const FTransform& FinalTransform, FName FinalContextName)
{
    // Simply reroute to the main execution function with the required data
    ExecuteSwapActorToInstance(SwappedActor, FinalTransform, FinalContextName);
}