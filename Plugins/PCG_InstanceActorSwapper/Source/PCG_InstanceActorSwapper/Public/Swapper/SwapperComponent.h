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

USTRUCT()
struct FActorToInstanceSwapInfo
{
    GENERATED_BODY()

    //ISMC Location
    UPROPERTY()
    FName InstancingComponentPathName = NAME_None;
    UPROPERTY()
    int32 InstanceIndex = INDEX_NONE;

    // Store the Original Static Mesh
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> OriginalStaticMesh = nullptr; 
    
    // The keys needed to re-access the Data Asset rule during swap-back.
    UPROPERTY()
    FName TargetName = NAME_None;
    UPROPERTY()
    FName OriginalContextName = NAME_None;
    
    UPROPERTY()
    bool bDidChangeHappened = false;
    
    UPROPERTY()
    FTransform ChangedInstanceTransform = FTransform::Identity;
    
    UPROPERTY()
    TObjectPtr<USwapperHelperComponent> SwapperHelper = nullptr; 

    //Flags to manage the two-step swap-back process
    UPROPERTY()
    bool bActorLeftRange = false; 
    
    UPROPERTY()
    bool bInteractionFinished = false;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCG_INSTANCEACTORSWAPPER_API USwapperComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    USwapperComponent();
    
protected:
    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> SwappableDetector = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swapper")
    FName InteractionTag = TEXT("InteractableInstance");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swapper")
    TObjectPtr<UInstanceActorSwappingDataAsset> SwapConfigDataAsset = nullptr;
    
    // Mapping: The Swapped Actor -> Runtime Tracking Data.
    UPROPERTY()
    TMap<TObjectPtr<AActor>, FActorToInstanceSwapInfo> SwappedActorsInfo;

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
    void OnSwapActorToInstance(AActor* SwappedActor);

    UFUNCTION()
    void OnOverlapBegin(
       UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
       int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
       
    UFUNCTION()
    void OnOverlapEnd(
       UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
       int32 OtherBodyIndex);

public:
    UFUNCTION(BlueprintCallable, Category = "Swapper|Execution")
    void ExecuteSwapInstanceToActor(UInstancedStaticMeshComponent* InstanceComponent, 
                             int32 InstanceIndex, 
                             TSubclassOf<AActor> ActorClassToSpawn,
                             FName TargetName,
                             FName ContextName);
    
    UFUNCTION(BlueprintCallable, Category = "Swapper|Execution")
    void ExecuteSwapActorToInstance(AActor* SwappedActor); 
    
    UFUNCTION(BlueprintCallable, Category = "Swapper|Setting")// set the collision comp of the owner class
    bool SetSwapperDetectorComp(UPrimitiveComponent* NewDetector);
    
private:

};