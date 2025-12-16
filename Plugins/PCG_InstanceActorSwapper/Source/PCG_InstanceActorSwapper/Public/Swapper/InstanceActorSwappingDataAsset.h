// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InstanceActorSwappingDataAsset.generated.h"

//Log for Swapper and SwappingHelper
PCG_INSTANCEACTORSWAPPER_API DECLARE_LOG_CATEGORY_EXTERN(SwapperComponent, Log, All);


// Forward Declares
class UStaticMesh;
class AActor;


USTRUCT(BlueprintType)
struct FSwappingMeshActorPair
{
	GENERATED_BODY()
    
	// THE RESULT MESH: The mesh to be instanced after the swap
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo|Result")
	TSoftObjectPtr<UStaticMesh> ResultStaticMesh = nullptr; 
    
	// Actor Class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TSubclassOf<AActor> ActorClassToSpawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo|Actor Logic")
	bool bIsPermanentActor = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo|Direct Swap")
	bool bIsDirectInstanceSwap = false;
};

USTRUCT(BlueprintType)
struct FSwappingTransformationRule
{
    GENERATED_BODY()
    
    // THE INPUT MESH (The key used to search)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo|Key")
    TSoftObjectPtr<UStaticMesh> InputMeshKey = nullptr; 
    
    // The index is now part of the rule itself
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo|Key")
    int32 VariationIndex = 0; 
    
    // The output instructions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo|Result")
    FSwappingMeshActorPair OutputRule;
};

USTRUCT(BlueprintType)
struct FSwappingVariationRules
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TArray< FSwappingTransformationRule> RuleList;
};

USTRUCT(BlueprintType)
struct FSwappingMeshActorInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TMap<FName/*Interaction Context*/, FSwappingVariationRules/*list of rules*/> MeshActorMap;
};


UCLASS(BlueprintType)
class PCG_INSTANCEACTORSWAPPER_API UInstanceActorSwappingDataAsset : public UDataAsset
{
	GENERATED_BODY()

public: 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TMap<FName/*Target Name*/, FSwappingMeshActorInfo/*Target's Swapping info*/> SwappingInfoList;
};
