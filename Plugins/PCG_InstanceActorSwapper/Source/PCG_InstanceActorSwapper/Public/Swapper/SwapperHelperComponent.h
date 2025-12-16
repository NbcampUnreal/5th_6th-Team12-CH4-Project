// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwapperHelperComponent.generated.h"


// Forward declares
class UInstancedStaticMeshComponent; 
class AActor;
class USwapperComponent;
class UStaticMesh;

// Delegate: Fired when the interaction is finished and the actor is ready to become an instance again.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSwapperReadyToSwapBack,
    AActor*, SwappedActor,
    const FTransform&, FinalTransform,
    FName, FinalContextName);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_INSTANCEACTORSWAPPER_API USwapperHelperComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    USwapperHelperComponent();

protected:
    
    UPROPERTY()
    FName InstancingComponentPathName = NAME_None; // path to the ISMC owner
    UPROPERTY()
    int32 InstanceIndex = INDEX_NONE; // index of the instanced static mesh from the ISMC

    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> PreviousMesh=nullptr; // the mesh before swapped into actor
    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap Result")
    FName TargetName = NAME_None; // name of the actor/static mesh

    // Context is set by the Actor/Interaction during runtime
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap Result")
    FName CurrentContextName = TEXT("NoChange");

    // The variation index chosen by the actor/interaction
    UPROPERTY()
    int32 CurrentContextVariationIndex = 0;

    // reference to the ISMC this instance came from
    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> InstancingComponent = nullptr;
    
    UPROPERTY()
    bool bIsStillProcessing = false;
   
    UPROPERTY()
    bool bDidTransformChanged = false; // Indicates if the actor's transform or state has changed
    
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
    void InitializeSwapContext(
        FName InContextName,
        int32 InContextVariationIndex, // Index is required for swap-out tracking
        UInstancedStaticMeshComponent* InInstancingComponent,
        FName InInstancingComponentPathName,
        int32 InInstanceIndex,
        UStaticMesh* InOriginalMesh);
    
    UFUNCTION(BlueprintPure, Category = "Swapper Helper")
    FName GetFinalContextName() const { return CurrentContextName; }
    
    // Getter for the change flag (maps to bDidTransformChanged)
    UFUNCTION(BlueprintPure, Category = "Swapper Helper")
    bool HasChangeOccurred() const { return bDidTransformChanged; }
    
    // Name
    FName GetTargetName() const { return TargetName; }
    void SetTargetName(FName InTargetName) { TargetName = InTargetName; }
    
    // Called by the Actor/Blueprint when the interaction is done
    UFUNCTION(BlueprintCallable, Category = "Swapper Helper")
    void SignalSwapCompletion(const FTransform& FinalWorldTransform, const FName& FinalContextName);

    // helper function for bp to check if the overlapped component a swapper component
    UFUNCTION(BlueprintPure, Category = "Swapper Helper")
    static bool IsTriggeringSwapperComp(UPrimitiveComponent* OverlappedComp);

    // Process flag setting
    UFUNCTION(BlueprintPure, Category = "Swapper Helper")
    bool IsProcessingTask() const { return bIsStillProcessing; }

    UFUNCTION(BlueprintCallable, Category = "Swapper Helper")
    void SetProcessingTask(bool bActive);

    // Setter for the change flag
    UFUNCTION(BlueprintCallable, Category = "Swapper Helper")
    void SetChangeOccurred(bool bChanged) { bDidTransformChanged = bChanged; }


    //getter functions
    FName GetISMCPathName() const { return InstancingComponentPathName; }
    int32 GetInstanceIndex() const { return InstanceIndex; }
    TSoftObjectPtr<UStaticMesh> GetPreviousMesh() const {return PreviousMesh; }

    FTransform GetCurrentTransform() const;
    
};