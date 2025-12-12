// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwapperHelperComponent.generated.h"


// Forward declares
struct FSwappingMeshActorInfo;
class UInstancedStaticMeshComponent; 
class AActor; 

// Delegate: Fired when the interaction is finished and the actor is ready to become an instance again.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwapperReadyToSwapBack, AActor*, SwappedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTempOnSwitchSignalSent,AActor*, SwappedActor, const FSwappingMeshActorInfo&, Info);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_INSTANCEACTORSWAPPER_API USwapperHelperComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    USwapperHelperComponent();

protected:

    UPROPERTY()
    FName InstancingComponentPathName = NAME_None;
    UPROPERTY()
    int32 InstanceIndex = INDEX_NONE;//index of the instanced static mesh from the ISMC
    
    UPROPERTY()
    FName TargetName = NAME_None;// name of the actor/static mesh
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap Result")
    FName CurrentContextName = TEXT("NothingChanged");// pass this to the swapper through the delegate 

    // reference to the ISMC this instance came from
    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> InstancingComponent = nullptr;

    // Flag Setting
    UPROPERTY()
    bool bActorLeftRange = false;
    UPROPERTY()
    bool bInteractionFinished = false;
    
public:
    // This delegate is bound by USwapperComponent to trigger the swap-back execution logic.
    UPROPERTY(BlueprintAssignable, Category = "Swapper Helper")
    FSwapperReadyToSwapBack OnReadyToSwapBack;

protected:
    virtual void BeginPlay() override;

public:
    // Activation (to manage lifecycle post-swap-out)
    void ActivateComponent();
    void DeactivateComponent();

    // Initialization: Called by USwapperComponent::ExecuteSwapInstanceToActor
    void InitializeSwapContext(FName InTargetName, FName InContextName, UInstancedStaticMeshComponent* InInstancingComponent);
    
    UFUNCTION(BlueprintPure, Category = "Swapper Helper")
    FName GetFinalContextName() const { return CurrentContextName; }// this will return the context of the actor state
    
    FName GetTargetName() const { return TargetName; }

    // Called by the Actor/Blueprint when the interaction is done
    UFUNCTION(BlueprintCallable, Category = "Swapper Helper")
    void SignalSwapCompletion();
    //TODO-> Make it pass the required info for swapping
};