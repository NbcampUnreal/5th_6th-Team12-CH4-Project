// Fill out your copyright notice in the Description page of Project Settings.

#include "Swapper/SwapperHelperComponent.h"

#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Swapper/InstanceActorSwappingDataAsset.h"
#include "Swapper/SwapperComponent.h"


USwapperHelperComponent::USwapperHelperComponent()
{
    PrimaryComponentTick.bCanEverTick = false; 
}

void USwapperHelperComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USwapperHelperComponent::ActivateComponent()
{
    SetComponentTickEnabled(true); 
    UE_LOG(SwapperComponent, Log,
        TEXT("Helper component tick enabled on Actor: %s"),
        *GetOwner()->GetName());
}

void USwapperHelperComponent::DeactivateComponent()
{
    SetComponentTickEnabled(false); 
    UE_LOG(SwapperComponent, Log,
        TEXT("Helper component tick disabled on Actor: %s"),
        *GetOwner()->GetName());
}

void USwapperHelperComponent::InitializeSwapContext(FName InContextName, int32 InContextVariationIndex,
    UInstancedStaticMeshComponent* InInstancingComponent, FName InInstancingComponentPathName, int32 InInstanceIndex,
    UStaticMesh* InOriginalMesh)
{
    if (!GetOwner())
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("USwapperHelperComponent::InitializeSwapContext >> Owner Actor is null."));
        return;
    }

 
    InstancingComponentPathName = InInstancingComponentPathName;
    InstanceIndex = InInstanceIndex;
    PreviousMesh = InOriginalMesh;
    InstancingComponent = InInstancingComponent;
    CurrentContextName = InContextName;
    CurrentContextVariationIndex = InContextVariationIndex;

    //flag cleanup just in case
    bIsStillProcessing = false;
    bDidTransformChanged = false;
    
    UE_LOG(SwapperComponent, Log,
        TEXT("USwapperHelperComponent::InitializeSwapContext >> Initialized %s. Context: %s [%d], Path: %s"), 
        *GetOwner()->GetName(), 
        *CurrentContextName.ToString(), 
        InstanceIndex,
        *InstancingComponentPathName.ToString());
    
    // Activation setup is done
    ActivateComponent();
}

void USwapperHelperComponent::SignalSwapCompletion(const FTransform& FinalWorldTransform, const FName& FinalContextName)
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(SwapperComponent, Error, TEXT("USwapperHelperComponent::SignalSwapCompletion >> Owner Actor is null."));
        return;
    }

    SetProcessingTask(false);// set it to false first
    
    UE_LOG(SwapperComponent, Log, 
            TEXT("USwapperHelperComponent::SignalSwapCompletion >> %s-> Final Context: %s. Firing delegate."), 
            *OwnerActor->GetName(), 
            *FinalContextName.ToString());

    // 3. Fire the delegate with all necessary swap-back data (3 parameters)
    OnReadyToSwapBack.Broadcast(OwnerActor, FinalWorldTransform, FinalContextName);
    
    // Deactivate the helper.
    DeactivateComponent();
}

bool USwapperHelperComponent::IsTriggeringSwapperComp(UPrimitiveComponent* OverlappedComp)
{
    if (!OverlappedComp)
    {
        UE_LOG(SwapperComponent, Error, 
            TEXT("USwapperHelperComponent::IsTriggeringSwapperComp >> OverlappedComp is null."));
        return false;
    }

    AActor* SwapperActor = OverlappedComp->GetOwner();
    if (!SwapperActor)
    {
        UE_LOG(SwapperComponent, Error, 
            TEXT("USwapperHelperComponent::IsTriggeringSwapperComp >> OverlappedComp (%s) has no Owner Actor."), 
            *OverlappedComp->GetName());
        return false;
    }

    USwapperComponent* Swapper = SwapperActor->FindComponentByClass<USwapperComponent>();
    if (!Swapper)
    {
        UE_LOG(SwapperComponent, Error, 
            TEXT("USwapperHelperComponent::IsTriggeringSwapperComp >> Actor %s does not have a USwapperComponent."), 
            *SwapperActor->GetName());
        return false;
    }
    
    UPrimitiveComponent* SwapperInteractionDetector = Swapper->GetInteractionDetector(); 
    if (!SwapperInteractionDetector)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("USwapperHelperComponent::IsTriggeringSwapperComp >> Swapper component on %s is missing its Interaction Detector setup."),
            *SwapperActor->GetName());
        return false;
    }
    
    return (OverlappedComp == SwapperInteractionDetector);
}

void USwapperHelperComponent::SetProcessingTask(bool bActive)
{
    if (!GetOwner())
    {
        UE_LOG(SwapperComponent, Error,
           TEXT("USwapperHelperComponent::SetProcessingTask >> InvalidOwner"));
        return;
    }
    
    bIsStillProcessing = bActive;
    UE_LOG(SwapperComponent, Log,
        TEXT("USwapperHelperComponent::SetProcessingTask >> %s is now %s"), 
        *GetOwner()->GetName(), 
        bActive ? TEXT("Active") : TEXT("Not Active"));
}

FTransform USwapperHelperComponent::GetCurrentTransform() const
{
    if (!GetOwner())
    {
        UE_LOG(SwapperComponent, Error,
          TEXT("USwapperHelperComponent::GetCurrentTransform >> InvalidOwner. Transform = FTransform::Identity"));
        return FTransform::Identity;
    }

    return GetOwner()->GetActorTransform();
}
