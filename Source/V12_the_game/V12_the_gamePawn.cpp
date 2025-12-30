// Copyright Epic Games, Inc. All Rights Reserved.

#include "V12_the_gamePawn.h"
#include "V12_the_gameWheelFront.h"
#include "V12_the_gameWheelRear.h"
#include "Player/V12PlayerState.h"
#include "SportsCar/V12_HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "V12_the_game.h"
#include "TimerManager.h"
#include "Items/V12InventoryComponent.h" 
#include "Blueprint/UserWidget.h"
#include "V12_the_gamePlayerController.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

AV12_the_gamePawn::AV12_the_gamePawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	//audio
	SideScrapeAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("SideScrapeAudio"));
	SideScrapeAudio->SetupAttachment(RootComponent);
	SideScrapeAudio->bAutoActivate = false;
	SideScrapeAudio->bAllowSpatialization = true;
	
	//scrape effect
	SideScrapeEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SideScrapeEffect"));
	SideScrapeEffect->SetupAttachment(RootComponent);
	SideScrapeEffect->SetAutoActivate(false);

	//camera effect
	SpeedEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SpeedEffect"));
	SpeedEffect->SetupAttachment(BackCamera);
	SpeedEffect->SetAutoActivate(false);

	//health component
	HealthComponent = CreateDefaultSubobject<UV12_HealthComponent>(TEXT("HealthComp"));

	bReplicates = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
}

void AV12_the_gamePawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AV12_the_gamePawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AV12_the_gamePawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AV12_the_gamePawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AV12_the_gamePawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AV12_the_gamePawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AV12_the_gamePawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::ResetVehicle);

		//Drifting
		EnhancedInputComponent->BindAction(DriftingAction, ETriggerEvent::Started, this, &AV12_the_gamePawn::StartDrifting);
		EnhancedInputComponent->BindAction(DriftingAction, ETriggerEvent::Completed, this, &AV12_the_gamePawn::StopDrifting);

		// Use item
		EnhancedInputComponent->BindAction(UseItemAction1, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::UseItem1);
		EnhancedInputComponent->BindAction(UseItemAction2, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::UseItem2);

		// Cancel LockOn
		EnhancedInputComponent->BindAction(CancelLockOnAction, ETriggerEvent::Triggered, this, &AV12_the_gamePawn::OnCancelLockOn);
	}
	else
	{
		UE_LOG(LogV12_the_game, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AV12_the_gamePawn::BeginPlay()
{
	Super::BeginPlay();

	// set up the flipped check timer
	GetWorld()->GetTimerManager().SetTimer(FlipCheckTimer, this, &AV12_the_gamePawn::FlippedCheck, FlipCheckTime, true);


	VehicleMesh = GetMesh();

	//몸체 색 변경을 위한 컴포넌트 변수 지정
	TArray<UActorComponent*> Components;
	GetComponents(UStaticMeshComponent::StaticClass(), Components);

	for (UActorComponent* Comp : Components)
	{
		if (UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Comp))
		{
			if (SM->ComponentHasTag(TEXT("VehicleBody")))
			{
				VehicleBodyMesh = SM;
				UE_LOG(LogTemp, Warning,
					TEXT("VehicleBody FOUND: %s"),
					*SM->GetName()
				);
				break;
			}
		}
	}

	ensureMsgf(VehicleBodyMesh, TEXT("VehicleBody StaticMesh NOT FOUND"));

	if (!VehicleBodyMesh)
	{
		UE_LOG(LogTemp, Error,
			TEXT("VehicleBodyMesh NOT FOUND")
		);
	}

	if (AV12PlayerState* PS = GetPlayerState<AV12PlayerState>())
	{
		ApplyVehicleColor(PS->VehicleColor);
		UE_LOG(LogTemp, Warning,
			TEXT("PlayerState FOUND Color=%s"),
			*PS->VehicleColor.ToString()
		);
	}

	//Drift
	int32 WheelCount = ChaosVehicleMovement->Wheels.Num();

	DefaultSideSlipModifier.SetNum(WheelCount);
	DefaultFrictionForceMultiplier.SetNum(WheelCount);
	DefaultCorneringStiffness.SetNum(WheelCount);

	for (int32 i = 0; i < ChaosVehicleMovement->Wheels.Num(); ++i)
	{
		DefaultSideSlipModifier[i] = ChaosVehicleMovement->Wheels[i]->SideSlipModifier;
		DefaultFrictionForceMultiplier[i] = ChaosVehicleMovement->Wheels[i]->FrictionForceMultiplier;
		DefaultCorneringStiffness[i] = ChaosVehicleMovement->Wheels[i]->CorneringStiffness;
	}

	//audio
	if (IsValid(VehicleMesh))
	{
		VehicleMesh->OnComponentHit.AddDynamic(this, &AV12_the_gamePawn::OnVehicleHit);
	}

	if (IsValid(SideScrapeSound))
	{
		SideScrapeAudio->SetSound(SideScrapeSound);
	}

	//scrape effect
	if (SideScrapeEffectAsset)
	{
		SideScrapeEffect->SetAsset(SideScrapeEffectAsset);
	}

	//camera
	BackSpringArm->TargetArmLength = DefaultCameraDistance;
	BackCamera->SetFieldOfView(DefaultFOV);

	if (SpeedEffectAsset)
	{
		SpeedEffect->SetAsset(SpeedEffectAsset);
	}
}

void AV12_the_gamePawn::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	// clear the flipped check timer
	GetWorld()->GetTimerManager().ClearTimer(FlipCheckTimer);

	Super::EndPlay(EndPlayReason);
}

void AV12_the_gamePawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	const float SpeedKmh = GetSpeedKmh();

	if (HasAuthority() && bIsDrifting)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DriftTick] Authority Drift Active"));

		//float Steer = ChaosVehicleMovement->GetSteeringInput();
		//float TorqueSign = (Steer >= 0.f ? 1.f : -1.f);

		UE_LOG(LogTemp, Warning, TEXT("[Drift] Server Steer: %f"), RepSteerInput);
		FVector Torque(0, 0, DriftTorqueStrength * RepSteerInput);

		VehicleMesh->AddTorqueInDegrees(Torque, NAME_None, true);

		FVector ForwardForce = GetActorForwardVector() * DriftForwardForce;
		VehicleMesh->AddForce(ForwardForce);

		/*float SteerInput = ChaosVehicleMovement->GetSteeringInput();

		FVector Vel = GetVelocity();
		FVector Right = GetActorRightVector();
		float LateralSpeed = FVector::DotProduct(Vel, Right);

		float CounterSteer = -LateralSpeed * CounterSteerStrength;

		float FinalSteer = FMath::Clamp(SteerInput + CounterSteer,-1.f, 1.f);

		ChaosVehicleMovement->SetSteeringInput(FinalSteer);*/
	}

	if (!IsLocallyControlled())
	{
		return;
	}

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));

	float TargetDistance = DefaultCameraDistance;
	float TargetFOV = DefaultFOV;

	if (SpeedKmh >= MinScrapeSpeedKmh)
	{
		float SpeedAlpha = FMath::Clamp(
			(SpeedKmh - MinScrapeSpeedKmh / 100.f),
			0.f,
			1.f
		);

		TargetDistance = FMath::Lerp(
			DefaultCameraDistance,
			MaxCameraDistance,
			SpeedAlpha
		);

		TargetFOV = FMath::Lerp(
			DefaultFOV,
			MaxFOV,
			SpeedAlpha
		);

	}
	BackSpringArm->TargetArmLength = FMath::FInterpTo(
		BackSpringArm->TargetArmLength,
		TargetDistance,
		Delta,
		CameraZoomInterpSpeed
	);

	BackCamera->SetFieldOfView(
		FMath::FInterpTo(
			BackCamera->FieldOfView,
			TargetFOV,
			Delta,
			FOVInterpSpeed
		)
	);

	if (!bFrontCameraActive)
	{
		if (SpeedKmh >= MinScrapeSpeedKmh)
		{
			if (!SpeedEffect->IsActive())
			{
				SpeedEffect->Activate();
			}

			SpeedEffect->SetFloatParameter(
				TEXT("SpeedRatio"),
				FMath::Clamp(SpeedKmh / 200.f, 0.2f, 1.f)
			);
		}
		else
		{
			if (SpeedEffect->IsActive())
			{
				SpeedEffect->Deactivate();
			}
		}
	}
	else
	{
		if (SpeedEffect->IsActive())
		{
			SpeedEffect->Deactivate();
		}
	}


	if (ChaosVehicleMovement)
	{
		float RPM = ChaosVehicleMovement->GetEngineRotationSpeed();

		GEngine->AddOnScreenDebugMessage(
			1,
			0.f,
			FColor::Green,
			FString::Printf(TEXT("RPM: %.0f"), RPM)
		);
	}

	
}

void AV12_the_gamePawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	TryApplyVehicleColor();
}

void AV12_the_gamePawn::Steering(const FInputActionValue& Value)
{
	float Steer = Value.Get<float>();
	// route the input
	DoSteering(Steer);

	if (IsLocallyControlled())
	{
		Server_SetSteerInput(Steer);
	}
}

void AV12_the_gamePawn::Throttle(const FInputActionValue& Value)
{
	/// 카운트다운중 입력 막기 여기서
	if (!bRaceStart) return;
	// route the input
	DoThrottle(Value.Get<float>());
}

void AV12_the_gamePawn::Brake(const FInputActionValue& Value)
{
	// route the input
	/// 카운트다운중 입력 막기 여기서
	if (!bRaceStart) return;
	DoBrake(Value.Get<float>());
}

void AV12_the_gamePawn::StartBrake(const FInputActionValue& Value)
{
	// route the input
	/// 카운트다운중 입력 막기 여기서
	if (!bRaceStart) return;
	DoBrakeStart();
}

void AV12_the_gamePawn::StopBrake(const FInputActionValue& Value)
{
	// route the input
	/// 카운트다운중 입력 막기 여기서
	if (!bRaceStart) return;
	DoBrakeStop();
}

void AV12_the_gamePawn::StartHandbrake(const FInputActionValue& Value)
{
	// route the input
	DoHandbrakeStart();
}

void AV12_the_gamePawn::StopHandbrake(const FInputActionValue& Value)
{
	// route the input
	DoHandbrakeStop();
}

void AV12_the_gamePawn::LookAround(const FInputActionValue& Value)
{
	// route the input
	DoLookAround(Value.Get<float>());
}

void AV12_the_gamePawn::ToggleCamera(const FInputActionValue& Value)
{
	// route the input
	DoToggleCamera();
}

void AV12_the_gamePawn::ResetVehicle(const FInputActionValue& Value)
{
	// route the input
	DoResetVehicle();
}

void AV12_the_gamePawn::StartDrifting(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;

	//DoHandbrakeStart();
	Server_SetBrake(true);
	ApplyDriftPhysics();
	Server_SetDrifting(true);
}

void AV12_the_gamePawn::StopDrifting(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;

	//DoHandbrakeStop();
	Server_SetBrake(false);
	RestoreDriftPhysics();
	Server_SetDrifting(false);
}

void AV12_the_gamePawn::ApplyDriftPhysics()
{
	ChaosVehicleMovement->SetMaxEngineTorque(MaxEngineTorque);

	for (UChaosVehicleWheel* Wheel : ChaosVehicleMovement->Wheels)
	{
		if (Wheel)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Drift] Wheel BEFORE: Slip=%.2f, Friction=%.2f, Corner=%.2f"),
				Wheel->SideSlipModifier, Wheel->FrictionForceMultiplier, Wheel->CorneringStiffness);
			if (Wheel->AxleType == EAxleType::Rear)
			{
				Wheel->SideSlipModifier = DriftSideSlipModifier;
				Wheel->FrictionForceMultiplier *= DriftFrictionForceMultiplier;
				Wheel->CorneringStiffness *= DriftCorneringStiffness;
			}

			UE_LOG(LogTemp, Warning, TEXT("[Drift] Wheel AFTER: Slip=%.2f, Friction=%.2f, Corner=%.2f"),
				Wheel->SideSlipModifier, Wheel->FrictionForceMultiplier, Wheel->CorneringStiffness);
		}
	}
}

void AV12_the_gamePawn::RestoreDriftPhysics()
{
	ChaosVehicleMovement->SetMaxEngineTorque(DefaultEngineTorque);

	for (int32 i = 0; i < ChaosVehicleMovement->Wheels.Num(); ++i)
	{
		ChaosVehicleMovement->Wheels[i]->SideSlipModifier = DefaultSideSlipModifier[i];
		ChaosVehicleMovement->Wheels[i]->FrictionForceMultiplier = DefaultFrictionForceMultiplier[i];
		ChaosVehicleMovement->Wheels[i]->CorneringStiffness = DefaultCorneringStiffness[i];
	}
}

void AV12_the_gamePawn::Server_SetSteerInput_Implementation(float Steer)
{
	RepSteerInput = FMath::Clamp(Steer, -1.f, 1.f);
	DoSteering(Steer);
}

void AV12_the_gamePawn::Server_SetDrifting_Implementation(bool bNewDrift)
{
	bIsDrifting = bNewDrift;

	if (bIsDrifting)
	{
		ApplyDriftPhysics();
	}
	else
	{
		RestoreDriftPhysics();
	}
}

#pragma region Items

void AV12_the_gamePawn::UseItem1(const FInputActionValue& Value)
{
	UseItemByIndex(0);
}

void AV12_the_gamePawn::UseItem2(const FInputActionValue& Value)
{
	UseItemByIndex(1);
}

void AV12_the_gamePawn::UseItemByIndex(int32 Index)
{
	// 아이템 사용
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("컨트롤러 없음"));
		return;
	}
	UV12InventoryComponent* InvComp = PC->GetComponentByClass<UV12InventoryComponent>();
	if (!InvComp)
	{
		UE_LOG(LogTemp, Error, TEXT("Not Found InventoryComponent in PlayerController"));
		return;
	}

	InvComp->UseItem(Index);

	UE_LOG(LogTemp, Warning, TEXT("Itme No.%d Use!"), Index + 1);
}

void AV12_the_gamePawn::OnCancelLockOn()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (AV12_the_gamePlayerController* V12PC =
			Cast<AV12_the_gamePlayerController>(PC))
		{
			V12PC->CancelLockOn();
		}
	}
}

void AV12_the_gamePawn::SetMissileDefense(bool bEnable)
{
	if (!HasAuthority())
	{
		return;
	}
	
	bMissileDefenseActive = bEnable;
	OnRep_MissileDefense();
}

void AV12_the_gamePawn::OnRep_MissileDefense()
{
	UE_LOG(LogTemp, Warning, TEXT("Missile Defense %s"),
		bMissileDefenseActive ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AV12_the_gamePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AV12_the_gamePawn, bMissileDefenseActive);
	DOREPLIFETIME(AV12_the_gamePawn, bIsDrifting);
	DOREPLIFETIME(AV12_the_gamePawn, RepSteerInput);
}

#pragma endregion

void AV12_the_gamePawn::Server_RequestDamage_Implementation(float Damage)
{
	if (!HealthComponent) return;

	HealthComponent->ApplyDamage(Damage);
}

void AV12_the_gamePawn::ApplyVehicleColor(const FLinearColor& Color)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[ApplyVehicleColor] Pawn=%s | NetMode=%d | Role=%d"),
		*GetName(),
		(int32)GetNetMode(),
		(int32)GetLocalRole()
	);
	
	if (!VehicleBodyMesh)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("VehicleBodyMesh is NULL")
		);
		return;
	}
	const int32 PaintMaterialIndex = 1;

	UMaterialInterface* BaseMaterial = VehicleBodyMesh->GetMaterial(PaintMaterialIndex);

	if (!BaseMaterial) return;

	UE_LOG(LogTemp, Warning,
		TEXT("Material[%d]: %s"),
		PaintMaterialIndex,
		*BaseMaterial->GetName()
	);

	UMaterialInstanceDynamic* MID = VehicleBodyMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(PaintMaterialIndex, BaseMaterial);

	if (MID)
	{
		MID->SetVectorParameterValue(TEXT("Paint Tint"), Color);
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Paint Tint applied successfully")
	);
}

void AV12_the_gamePawn::TryApplyVehicleColor()
{
	if (!VehicleBodyMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh not ready"));
		return;
	}

	AV12PlayerState* PS = GetPlayerState<AV12PlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState not ready"));
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("TryApplyVehicleColor Color=%s"),
		*PS->VehicleColor.ToString()
	);

	ApplyVehicleColor(PS->VehicleColor);
}

void AV12_the_gamePawn::OnVehicleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this)
		return;

	if (HitComponent != GetMesh())
		return;

	if (!HasAuthority()) return;

	const FVector Forward = GetActorForwardVector();
	const FVector Right = GetActorRightVector();
	const FVector Normal = Hit.ImpactNormal;

	const float SideDot = FVector::DotProduct(Right, Normal);
	const float ImpactPower = NormalImpulse.Size();

	const bool bSideHit = FMath::Abs(SideDot) >= SideDotThreshold;

	const float GroundDot = FVector::DotProduct(Normal, FVector::UpVector);
	const bool bGroundHit = GroundDot >= GroundNormalThreshold;

	if (bGroundHit)
	{
		return;
	}

	if (ImpactPower >= StrongImpactThreshold)
	{
		const float Now = GetWorld()->GetTimeSeconds();
		
		if (Now - LastFrontImpactTime < FrontImpactCooldown)
			return;

		LastFrontImpactTime = Now;

		if (AV12_the_gamePawn* OtherPawn = Cast<AV12_the_gamePawn>(OtherActor))
		{
			if (GetUniqueID() < OtherPawn->GetUniqueID())
			{
				Multicast_PlayFrontImpact(Hit.ImpactPoint);
			}
		}
		else
		{
			Multicast_PlayFrontImpact(Hit.ImpactPoint);
		}

		UE_LOG(LogTemp, Warning,
			TEXT("[FRONT IMPACT SERVER] %s | Other=%s"),
			*GetName(),
			*OtherActor->GetName()
		);

		return;
		
		/*if (SideScrapeAudio->IsPlaying())
		{
			SideScrapeAudio->Stop();
		}

		if (SideScrapeEffect->IsActive())
		{
			SideScrapeEffect->Deactivate();
		}

		if (IsValid(FrontImpactSound))
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(),
				FrontImpactSound,
				Hit.ImpactPoint
			);
		}*/
	}

	const float SpeedKmh = GetSpeedKmh();

	if (SpeedKmh < MinScrapeSpeedKmh)
	{
		if (SideScrapeAudio->IsPlaying())
			SideScrapeAudio->Stop();

		if (SideScrapeEffect->IsActive())
			SideScrapeEffect->Deactivate();

		return;
	}

	if (bSideHit)
	{
		//audio
		if (!SideScrapeAudio->IsPlaying())
		{
			SideScrapeAudio->Play();
		}

		//effect
		SideScrapeEffect->SetWorldLocation(Hit.ImpactPoint);

		FVector Velocity = GetVelocity();
		Velocity.Z = 0.f;

		if (!Velocity.IsNearlyZero())
		{
			FVector SparkDir = FVector::VectorPlaneProject(
				-Velocity.GetSafeNormal(),
				Hit.ImpactNormal
			);

			SideScrapeEffect->SetWorldRotation(SparkDir.Rotation());
		}

		if (!SideScrapeEffect->IsActive())
		{
			SideScrapeEffect->Activate();
		}

		GetWorld()->GetTimerManager().ClearTimer(ScrapeStopTimer);
		GetWorld()->GetTimerManager().SetTimer(
			ScrapeStopTimer,
			this,
			&AV12_the_gamePawn::StopSideScrape,
			ScrapeStopDelay,
			false
		);
	}
}

void AV12_the_gamePawn::StopSideScrape()
{
	if (SideScrapeAudio->IsPlaying())
	{
		SideScrapeAudio->Stop();
	}

	if (SideScrapeEffect->IsActive())
	{
		SideScrapeEffect->Deactivate();
	}
}

void AV12_the_gamePawn::Multicast_PlayFrontImpact_Implementation(FVector ImpactPoint)
{
	if (HasAuthority())
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning,
		TEXT("[FRONT IMPACT MULTI] %s | NetMode=%d"),
		*GetName(),
		(int32)GetNetMode()
	);

	if (SideScrapeAudio->IsPlaying())
	{
		SideScrapeAudio->Stop();
	}

	if (SideScrapeEffect->IsActive())
	{
		SideScrapeEffect->Deactivate();
	}

	if (IsValid(FrontImpactSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			FrontImpactSound,
			ImpactPoint,
			1.f,
			1.f,
			0.f,
			ImpactAttenuation
		);
	}
}



void AV12_the_gamePawn::DoSteering(float SteeringValue)
{
	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void AV12_the_gamePawn::DoThrottle(float ThrottleValue)
{
	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);

	// reset the brake input
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AV12_the_gamePawn::DoBrake(float BrakeValue)
{
	// add the input
	ChaosVehicleMovement->SetBrakeInput(BrakeValue);

	// reset the throttle input
	ChaosVehicleMovement->SetThrottleInput(0.0f);
}

void AV12_the_gamePawn::DoBrakeStart()
{
	// call the Blueprint hook for the brake lights
	if (IsLocallyControlled())
	{
		Server_SetBrake(true);
	}
}

void AV12_the_gamePawn::DoBrakeStop()
{
	// call the Blueprint hook for the brake lights
	if (IsLocallyControlled())
	{
		Server_SetBrake(false);
	}

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AV12_the_gamePawn::DoHandbrakeStart()
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	if (IsLocallyControlled())
	{
		Server_SetBrake(true);
	}
}

void AV12_the_gamePawn::DoHandbrakeStop()
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	if (IsLocallyControlled())
	{
		Server_SetBrake(false);
	}
}

void AV12_the_gamePawn::DoLookAround(float YawDelta)
{
	// rotate the spring arm
	BackSpringArm->AddLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void AV12_the_gamePawn::DoToggleCamera()
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void AV12_the_gamePawn::DoResetVehicle()
{
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;

	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void AV12_the_gamePawn::OnRep_Brake()
{
	BrakeLights(bBrakeOn);
}

void AV12_the_gamePawn::Server_SetBrake_Implementation(bool bNewBrake)
{
	bBrakeOn = bNewBrake;
}

void AV12_the_gamePawn::FlippedCheck()
{
	// check the difference in angle between the mesh's up vector and world up
	const float UpDot = FVector::DotProduct(FVector::UpVector, GetMesh()->GetUpVector());

	if (UpDot < FlipCheckMinDot)
	{
		// is this the second time we've checked that the vehicle is still flipped?
		if (bPreviousFlipCheck)
		{
			// reset the vehicle to upright
			DoResetVehicle();
		}
		
		// set the flipped check flag so the next check resets the car
		bPreviousFlipCheck = true;

	} else {

		// we're upright. reset the flipped check flag
		bPreviousFlipCheck = false;
	}
}

float AV12_the_gamePawn::GetSpeedKmh() const
{
	const float SpeedCmPerSec = GetVelocity().Size();

	// cm/s → km/h
	return SpeedCmPerSec * 0.036f;
}

void AV12_the_gamePawn::setRaceStart(bool val)
{
	bRaceStart = val;
}

#undef LOCTEXT_NAMESPACE