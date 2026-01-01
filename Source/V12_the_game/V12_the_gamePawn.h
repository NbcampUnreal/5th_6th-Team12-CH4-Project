// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Net/UnrealNetwork.h"
#include "V12_the_gamePawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
class UV12InventoryComponent;
class USoundBase;
class UAudioComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UV12_HealthComponent;
class USoundAttenuation;
struct FInputActionValue;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissileDefenseChanged, AV12_the_gamePawn*);

/**
 *  Vehicle Pawn class
 *  Handles common functionality for all vehicle types,
 *  including input handling and camera management.
 *  
 *  Specific vehicle configurations are handled in subclasses.
 */
UCLASS(abstract)
class AV12_the_gamePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BackCamera;

	USkeletalMeshComponent* VehicleMesh;

	UPROPERTY()
	UStaticMeshComponent* VehicleBodyMesh;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UV12_HealthComponent* HealthComponent;

	/** Cast pointer to the Chaos Vehicle movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

	//audio
	UPROPERTY(VisibleAnywhere, Category = "Audio")
	UAudioComponent* SideScrapeAudio;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* SideScrapeSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* FrontImpactSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundAttenuation* ImpactAttenuation;

	//effect
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	UNiagaraComponent* SideScrapeEffect;

	UPROPERTY(EditAnywhere, Category = "Effect")
	UNiagaraSystem* SideScrapeEffectAsset;

	UPROPERTY(EditAnywhere, Category = "Effect")
	float MinScrapeSpeedKmh = 30.f;

	float GroundNormalThreshold = 0.75f;

	//camera effect
	UPROPERTY(EditAnywhere, Category = "Effect|Camera")
	float DefaultCameraDistance = 650.f;

	UPROPERTY(EditAnywhere, Category = "Effect|Camera")
	float MaxCameraDistance = 900.f;

	UPROPERTY(EditAnywhere, Category = "Effect|Camera")
	float CameraZoomInterpSpeed = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Effect|Camera")
	float DefaultFOV = 90.f;

	UPROPERTY(EditAnywhere, Category = "Effect|Camera")
	float MaxFOV = 105.f;

	UPROPERTY(EditAnywhere, Category = "Effect|Camera")
	float FOVInterpSpeed = 6.f;

	UPROPERTY(EditAnywhere, Category = "Effect|Camera")
	UNiagaraComponent* SpeedEffect;

	UPROPERTY(EditAnywhere, Category = "Effect|Camera")
	UNiagaraSystem* SpeedEffectAsset;

protected:

	/** Steering Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SteeringAction;

	/** Throttle Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ThrottleAction;

	/** Brake Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* BrakeAction;

	/** Handbrake Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* HandbrakeAction;

	/** Look Around Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAroundAction;

	/** Toggle Camera Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ToggleCameraAction;

	/** Reset Vehicle Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ResetVehicleAction;


	//Drift
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DriftingAction;

	UPROPERTY(EditAnywhere, Category = "Drift")
	float DriftTorqueStrength = 200;

	UPROPERTY(EditAnywhere, Category = "Drift")
	float MaxEngineTorque = 2000;

	UPROPERTY(EditAnywhere, Category = "Drift")
	float DefaultEngineTorque = 750;

	UPROPERTY(EditAnywhere, Category = "Drift")
	float DriftSideSlipModifier = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Drift")
	float DriftFrictionForceMultiplier = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Drift")
	float DriftCorneringStiffness = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Drift")
	float CounterSteerStrength = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Drift")
	float DriftForwardForce = 1000.f;

	TArray<float> DefaultSideSlipModifier;
	TArray<float> DefaultFrictionForceMultiplier;
	TArray<float> DefaultCorneringStiffness;

	UPROPERTY(Replicated)
	bool bIsDrifting = false;

	UPROPERTY(Replicated)
	float RepSteerInput = 0.f;

	//Collision
	float SideDotThreshold = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Collision")
	float StrongImpactThreshold = 80000.f;

	float ScrapeStopDelay = 0.15f;

	FTimerHandle ScrapeStopTimer;

	/** Keeps track of which camera is active */
	bool bFrontCameraActive = false;

	/** Keeps track of whether the car is flipped. If this is true for two flip checks, resets the vehicle automatically */
	bool bPreviousFlipCheck = false;

	/** Time between automatic flip checks */
	UPROPERTY(EditAnywhere, Category="Flip Check", meta = (Units = "s"))
	float FlipCheckTime = 3.0f;

	/** Minimum dot product value for the vehicle's up direction that we still consider upright */
	UPROPERTY(EditAnywhere, Category="Flip Check")
	float FlipCheckMinDot = -0.2f;

	/** Flip check timer */
	FTimerHandle FlipCheckTimer;


public:
	AV12_the_gamePawn();

	// Begin Pawn interface

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	// End Pawn interface

	// Begin Actor interface

	/** Initialization */
	virtual void BeginPlay() override;

	/** Cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Update */
	virtual void Tick(float Delta) override;

	virtual void OnRep_PlayerState() override;

protected:

	/** Handles steering input */
	void Steering(const FInputActionValue& Value);

	/** Handles throttle input */
	void Throttle(const FInputActionValue& Value);

	/** Handles brake input */
	void Brake(const FInputActionValue& Value);

	/** Handles brake start/stop inputs */
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	/** Handles handbrake start/stop inputs */
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	/** Handles look around input */
	void LookAround(const FInputActionValue& Value);

	/** Handles toggle camera input */
	void ToggleCamera(const FInputActionValue& Value);

	/** Handles reset vehicle input */
	void ResetVehicle(const FInputActionValue& Value);

	//Drifting
	void StartDrifting(const FInputActionValue& Value);
	void StopDrifting(const FInputActionValue& Value);

	void ApplyDriftPhysics();
	void RestoreDriftPhysics();

	UFUNCTION(Server, Reliable)
	void Server_SetDrifting(bool bNewDrift);

	UFUNCTION(Server, Unreliable)
	void Server_SetSteerInput(float Steer);

#pragma region Items

	/** Use Item Action, 1,2 slot 아이템 사용 */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UseItemAction1;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UseItemAction2;
	
	/** CancelLockOnAction */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CancelLockOnAction;

	/** Handles use item input */
	void UseItem1(const FInputActionValue& Value);
	void UseItem2(const FInputActionValue& Value);
	void UseItemByIndex(int32 Index);

	/** CancelLockOn */
	void OnCancelLockOn();

public:
	/** Missile Defense */
	FOnMissileDefenseChanged OnMissileDefenseChanged;

	UPROPERTY(ReplicatedUsing = OnRep_MissileDefense, VisibleAnywhere, BlueprintReadOnly, Category = "Defense")
	bool bMissileDefenseActive = false;

	UFUNCTION()
	void OnRep_MissileDefense();

	void SetMissileDefense(bool bEnable);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifeTimeProps) const override;

	FTimerHandle MissileDefenseTimer;

	//damage 처리 함수
	UFUNCTION(Server, Reliable)
	void Server_RequestDamage(float Damage);
#pragma endregion

	void ApplyVehicleColor(const FLinearColor& Color);
	void TryApplyVehicleColor();

protected:
	UFUNCTION()
	void OnVehicleHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
	void StopSideScrape();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFrontImpact(FVector ImpactPoint);

	UPROPERTY()
	float LastFrontImpactTime = -100.f;

	UPROPERTY(EditDefaultsOnly)
	float FrontImpactCooldown = 0.3f;

public:
	/** Handle steering input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSteering(float SteeringValue);

	/** Handle throttle input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoThrottle(float ThrottleValue);

	/** Handle brake input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrake(float BrakeValue);

	/** Handle brake start input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrakeStart();

	/** Handle brake stop input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrakeStop();

	/** Handle handbrake start input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHandbrakeStart();

	/** Handle handbrake stop input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHandbrakeStop();

	/** Handle look input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoLookAround(float YawDelta);

	/** Handle toggle camera input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoToggleCamera();

	/** Handle reset vehicle input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoResetVehicle();

protected:
	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

	UPROPERTY(ReplicatedUsing = OnRep_Brake)
	bool bBrakeOn;

	UFUNCTION()
	void OnRep_Brake();

	UFUNCTION(Server, Reliable)
	void Server_SetBrake(bool bNewBrake);

	/** Checks if the car is flipped upside down and automatically resets it */
	UFUNCTION()
	void FlippedCheck();

public:
	/** Returns the front spring arm subobject */
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	/** Returns the front camera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	/** Returns the back spring arm subobject */
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	/** Returns the back camera subobject */
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	/** Returns the cast Chaos Vehicle Movement subobject */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	float GetSpeedKmh() const;

public:
	void setRaceStart(bool val);
	bool getRaceStart() const { return bRaceStart; }
private:
	bool bRaceStart = false;
};
