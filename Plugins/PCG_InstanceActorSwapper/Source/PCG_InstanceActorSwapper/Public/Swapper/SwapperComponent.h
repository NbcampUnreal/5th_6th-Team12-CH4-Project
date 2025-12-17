// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Swapper/InstanceActorSwappingDataAsset.h" 
#include "SwapperComponent.generated.h"

// Forward Declares
class UPrimitiveComponent; 
class UInstancedStaticMeshComponent;
class UStaticMesh;
class AActor; 
class USwapperHelperComponent; 


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_INSTANCEACTORSWAPPER_API USwapperComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    USwapperComponent();
    
protected:
    UPROPERTY()// this is for the detecting the static meshes or actor so that they can be swapped
    TObjectPtr<UPrimitiveComponent> SwappableDetector = nullptr;
    UPROPERTY()// this is for interacting with the swapped actor
    TObjectPtr<UPrimitiveComponent> InteractionDetector = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swapper")
    FName InteractionTag = TEXT("InteractableInstance");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swapper")
    TObjectPtr<UInstanceActorSwappingDataAsset> SwapConfigDataAsset = nullptr;


protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Swapper")
    void OnSwapInstanceToActor(
        UInstancedStaticMeshComponent* InstanceComponent, 
        int32 InstanceIndex, 
        TSubclassOf<AActor> ActorClassToSpawn,
        FName TargetName,
        FName ContextName);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Swapper")
    void OnSwapActorToInstance(AActor* SwappedActor, FTransform SwappedTransform, FName SwappedContext);

    // Overlap event for mesh actor swapping
    UFUNCTION()
    void OnOverlapBegin(
       UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
       int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnOverlapEnd(
      UPrimitiveComponent* OverlappedComp,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      int32 OtherBodyIndex);

    //instant mesh to mesh swapping
    UFUNCTION()
    void OnInteractionOverlapBegin(
        UPrimitiveComponent* OverlappedComp, 
        AActor* OtherActor, 
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, 
        bool bFromSweep, 
        const FHitResult& SweepResult);

public:

    // getter for a swapping component
    UPrimitiveComponent* GetSwappableDetector() const { return SwappableDetector; }
    UPrimitiveComponent* GetInteractionDetector() const { return InteractionDetector; }
    
    UFUNCTION(BlueprintCallable, Category = "Swapper|Execution")
    void ExecuteSwapInstanceToActor(UInstancedStaticMeshComponent* InstanceComponent, 
                             int32 InstanceIndex, 
                             TSubclassOf<AActor> ActorClassToSpawn,
                             FName TargetName,
                             FName ContextName);
    
    UFUNCTION(BlueprintCallable, Category = "Swapper|Execution")
    void ExecuteSwapActorToInstance(AActor* SwappedActor, const FTransform& FinalTransform, FName FinalContextName);

    UFUNCTION()
    void ExecuteSwapActorToInstance_DelegateEntry(// the bound delegate for delaying switch
        AActor* SwappedActor,
        const FTransform& FinalTransform,
        FName FinalContextName);
    
    UFUNCTION(BlueprintCallable, Category = "Swapper|Setting")// set the collision comp of the owner class
    bool SetSwapperDetectorComp(UPrimitiveComponent* NewDetector);
    
    UFUNCTION(BlueprintCallable, Category = "Swapper|Setting")// set the collision comp of the owner class
    bool SetInteractionRangeComp(UPrimitiveComponent* InteractionComp);
private:

};