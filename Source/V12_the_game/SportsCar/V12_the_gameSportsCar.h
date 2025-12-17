// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "V12_the_gamePawn.h"
#include "Items/V12ItemBase.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "V12_the_gameSportsCar.generated.h"


/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class AV12_the_gameSportsCar : public AV12_the_gamePawn
{
	GENERATED_BODY()
	
public:

	AV12_the_gameSportsCar();

	void BeginPlay() override;

	void Tick(float DeltaTime) override;

#pragma region Item System

public:
	// 미사일 맞았을 때 호출될 함수
	UFUNCTION(BlueprintCallable, Category = "Damage|Missile")
	void LaunchAndSpin(const FVector& HitLocation);

	// Boost
	UFUNCTION()
	void ActivateBoost(float BoostForce);

	UFUNCTION()
	void EndBoost();

protected:
	// ----- 미사일 피격시 받는 충격치-----

	UPROPERTY(EditAnywhere, Category = "Damage|Missile")
	float HorizontalImpulse = 5.f;

	UPROPERTY(EditAnywhere, Category = "Damage|Missile")
	float VerticalImpulse = 50.f;

	UPROPERTY(EditAnywhere, Category = "Damage|Missile")
	float SpinImpulse = 20.f;

	UPROPERTY(VisibleAnywhere, Category = "Damage|Missile")
	bool bControlDisabled = false;

	UPROPERTY(EditAnywhere, Category = "Damage|Missile")
	float DisableControlDuration = 1.0f;

	FTimerHandle ControlDisableTimer;

#pragma endregion

};
