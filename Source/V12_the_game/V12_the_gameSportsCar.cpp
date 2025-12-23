// Copyright Epic Games, Inc. All Rights Reserved.


#include "V12_the_gameSportsCar.h"
#include "V12_the_gameSportsWheelFront.h"
#include "V12_the_gameSportsWheelRear.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Items/V12ItemBase.h"

AV12_the_gameSportsCar::AV12_the_gameSportsCar()
{
	// Note: for faster iteration times, the vehicle setup can be tweaked in the Blueprint instead

	// Set up the chassis
	GetChaosVehicleMovement()->ChassisHeight = 144.0f;
	GetChaosVehicleMovement()->DragCoefficient = 0.31f;

	// Set up the wheels
	GetChaosVehicleMovement()->bLegacyWheelFrictionPosition = true;
	GetChaosVehicleMovement()->WheelSetups.SetNum(4);

	GetChaosVehicleMovement()->WheelSetups[0].WheelClass = UV12_the_gameSportsWheelFront::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[0].BoneName = FName("Phys_Wheel_FL");
	GetChaosVehicleMovement()->WheelSetups[0].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[1].WheelClass = UV12_the_gameSportsWheelFront::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[1].BoneName = FName("Phys_Wheel_FR");
	GetChaosVehicleMovement()->WheelSetups[1].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[2].WheelClass = UV12_the_gameSportsWheelRear::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[2].BoneName = FName("Phys_Wheel_BL");
	GetChaosVehicleMovement()->WheelSetups[2].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[3].WheelClass = UV12_the_gameSportsWheelRear::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[3].BoneName = FName("Phys_Wheel_BR");
	GetChaosVehicleMovement()->WheelSetups[3].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	// Set up the engine
	// NOTE: Check the Blueprint asset for the Torque Curve
	GetChaosVehicleMovement()->EngineSetup.MaxTorque = 750.0f;
	GetChaosVehicleMovement()->EngineSetup.MaxRPM = 7000.0f;
	GetChaosVehicleMovement()->EngineSetup.EngineIdleRPM = 900.0f;
	GetChaosVehicleMovement()->EngineSetup.EngineBrakeEffect = 0.2f;
	GetChaosVehicleMovement()->EngineSetup.EngineRevUpMOI = 5.0f;
	GetChaosVehicleMovement()->EngineSetup.EngineRevDownRate = 600.0f;

	// Set up the transmission
	GetChaosVehicleMovement()->TransmissionSetup.bUseAutomaticGears = true;
	GetChaosVehicleMovement()->TransmissionSetup.bUseAutoReverse = true;
	GetChaosVehicleMovement()->TransmissionSetup.FinalRatio = 2.81f;
	GetChaosVehicleMovement()->TransmissionSetup.ChangeUpRPM = 6000.0f;
	GetChaosVehicleMovement()->TransmissionSetup.ChangeDownRPM = 2000.0f;
	GetChaosVehicleMovement()->TransmissionSetup.GearChangeTime = 0.2f;
	GetChaosVehicleMovement()->TransmissionSetup.TransmissionEfficiency = 0.9f;

	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios.SetNum(5);
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[0] = 4.25f;
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[1] = 2.52f;
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[2] = 1.66f;
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[3] = 1.22f;
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[4] = 1.0f;

	GetChaosVehicleMovement()->TransmissionSetup.ReverseGearRatios.SetNum(1);
	GetChaosVehicleMovement()->TransmissionSetup.ReverseGearRatios[0] = 4.04f;

	// Set up the steering
	// NOTE: Check the Blueprint asset for the Steering Curve
	GetChaosVehicleMovement()->SteeringSetup.SteeringType = ESteeringType::Ackermann;
	GetChaosVehicleMovement()->SteeringSetup.AngleRatio = 0.7f;
}

void AV12_the_gameSportsCar::BeginPlay()
{
	Super::BeginPlay();
}

void AV12_the_gameSportsCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#pragma region Item System

// Car Launch And Spin
void AV12_the_gameSportsCar::LaunchAndSpin(const FVector& HitLocation)
{
	UPrimitiveComponent* CarMesh =
		Cast<UPrimitiveComponent>(GetRootComponent());

	if (!CarMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("LaunchAndSpin : VehicleMesh invalid"));
		return;
	}

	CarMesh->WakeAllRigidBodies();

	// ===== 날아가는 방향 =====
	FVector Dir = GetActorLocation() - HitLocation;
	Dir.Z = 0.f;
	Dir.Normalize();

	// ===== 선형 임펄스 (위 + 뒤) =====
	FVector LaunchImpulse =
		Dir * HorizontalImpulse +
		FVector(0.f, 0.f, VerticalImpulse);

	CarMesh->AddImpulse(
		LaunchImpulse,
		NAME_None,
		true   // 질량 무시
	);

	// ===== 회전 임펄스 (한 바퀴) =====
	FVector AngularImpulse =
		GetActorRightVector() * SpinImpulse;

	CarMesh->AddAngularImpulseInRadians(
		AngularImpulse,
		NAME_None,
		true
	);
}

// StartBoost
void AV12_the_gameSportsCar::ActivateBoost(float BoostForce)
{
	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(GetRootComponent());
	if (!RootComp) return;

	FVector Forward = GetActorForwardVector();

	// 차량에 큰 임펄스 적용 (질량 무시)
	RootComp->AddImpulse(Forward * BoostForce, NAME_None, true);
}

// EndBoost
void AV12_the_gameSportsCar::EndBoost()
{
	UE_LOG(LogTemp, Warning, TEXT("Nitro End"));
}

#pragma endregion
