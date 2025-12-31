#include "gamemode/V12_MainGameMode.h"
#include "Player/V12PlayerState.h"
#include "V12_the_gamePlayerController.h"
#include "Game/V12GameInstance.h"
#include "Game/V12_ARacingManager.h"
#include "Kismet/GameplayStatics.h"

AV12_MainGameMode::AV12_MainGameMode()
{
	bUseSeamlessTravel = true;
}

void AV12_MainGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(C);
	if (PC)
	{
		RestartPlayer(PC);

		A_PC.AddUnique(PC); // Ï§ëÎ≥µ Î∞©Ï?

		
		if (AV12PlayerState* PS = PC->GetPlayerState<AV12PlayerState>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Travel Success - Player Name: %s"), *PS->PlayerName);
		}


		//static int32 ColorIndex = 0;


		//if (AV12PlayerState* PS = PC->GetPlayerState<AV12PlayerState>())
		//{
		//	FString tempName = PS->PlayerName;

		//	if (PresetColors.Num() > 0)
		//	{
		//		//BP?êÏÑú ??ÏßÄ???? Î≥µÏ†úÎ•??µÌï¥ ?êÎèô ?ÅÏö©
		//		PS->VehicleColor = PresetColors[ColorIndex % PresetColors.Num()];
		//		ColorIndex++;
		//	}

		//	UE_LOG(LogTemp, Error, TEXT("Player Name is %s"), *tempName);
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Error, TEXT("Player Name is ERROR"));
		//}

	}
}

void AV12_MainGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// GameInstance Í∞Ä?∏Ïò§Í∏?
	UV12GameInstance* GI = GetGameInstance<UV12GameInstance>();
	if (IsValid(GI))
	{
		UE_LOG(LogTemp, Error, TEXT("PostSeamlessTravel: Player Count = %d"), GI->allPlayerCount);
	}

	StartCountdown();

}

void AV12_MainGameMode::BeginPlay()
{
	Super::BeginPlay();

	/// ?êÏàò ?¨ÎùºÍ∞Ä?îÏ? ?ïÏù∏Îß??òÍ∏∞ ?ÑÌïú Í∏∞Îä•
	GetWorldTimerManager().SetTimer(
		TestTimer,
		this,
		&AV12_MainGameMode::TestFunc,
		2.0f,
		true);

	//GetWorldTimerManager().SetTimer(
	//	TestTimer2,
	//	this,
	//	&AV12_MainGameMode::RaceManagerStart,
	//	10.0f,
	//	false);


	/// ?ïÌôï???åÎ†à?¥Ïñ¥ Í∞?àòÎ•?Ï≤¥ÌÅ¨?òÍ≥† ?úÏûë?òÎèÑÎ°?Î≥ÄÍ≤?
	// StartCountdown();
}

/// Seamless Travel ?¥Ïö©??PostLogin?Ä ?¨Ïö©?òÏ? ?äÎäî??!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void AV12_MainGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!IsValid(NewPlayer)) return;
	
	static int32 ColorIndex = 0;

	A_PC.Add(Cast<AV12_the_gamePlayerController>(NewPlayer));

    if (AV12PlayerState* PS = NewPlayer->GetPlayerState<AV12PlayerState>())
    {
		FString tempName = PS->PlayerName;

		if (PresetColors.Num() > 0)
		{
			//BP?êÏÑú ??ÏßÄ???? Î≥µÏ†úÎ•??µÌï¥ ?êÎèô ?ÅÏö©
			PS->VehicleColor = PresetColors[ColorIndex % PresetColors.Num()];
			ColorIndex++;
		}

		UE_LOG(LogTemp, Error, TEXT("Player Name is %s"), *tempName);
    }
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Player Name is ERROR"));
	}

	if (UV12GameInstance* GI = Cast<UV12GameInstance>(GetGameInstance()))
	{
		UE_LOG(LogTemp, Error, TEXT("Current Player Count: %d"), GI->allPlayerCount);
		UE_LOG(LogTemp, Error, TEXT("APC Count: %d"), A_PC.Num());


		if (GI->allPlayerCount == A_PC.Num())
		{
			StartCountdown();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!"));
	}

}

void AV12_MainGameMode::StartCountdown()
{
	countdownCount = 5;

	GetWorldTimerManager().SetTimer(
		CountdownTimer,
		this,
		&AV12_MainGameMode::CountdownFunc,
		1.0f,
		true);
}

void AV12_MainGameMode::CountdownFunc()
{
	for(auto a : A_PC)
	{
		if (!IsValid(a)) continue;
		AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(a);
		if (!IsValid(PC)) continue;
		PC->CountdownCheck(FText::AsNumber(countdownCount));
	}
	/// ?ùÎÇòÎ©??úÍ±∞
	countdownCount--;
	if (countdownCount < 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimer);
		
		/// RACE START!!!!!!!!!!!!!!
		RaceManagerStart();

		for (auto a : A_PC)
		{
			if (!IsValid(a)) continue;
			AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(a);
			if (!IsValid(PC)) continue;
			FText zeroBack = FText::GetEmpty();
			PC->CountdownCheck(zeroBack);
			PC->BeginRace();
		}
	}
}

void AV12_MainGameMode::TestFunc()
{
	for(auto a : A_PC)
	{
		if (!IsValid(a)) continue;
		AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(a);
		if (!IsValid(PC)) continue;
		AV12PlayerState* PS = PC->GetPlayerState<AV12PlayerState>();
		if (!IsValid(PS)) continue;
		PS->PlayerScore += 10;
		// UE_LOG(LogTemp, Warning, TEXT("Player %s Score: %d"), *PS->PlayerName, PS->PlayerScore);
	}
}

void AV12_MainGameMode::RaceManagerStart()
{
	// ?àÏù¥??Îß§Îãà?Ä ?°ÌÑ∞ Ï∞æÍ∏∞
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AV12_ARacingManager::StaticClass());
	AV12_ARacingManager* RacingManager = Cast<AV12_ARacingManager>(FoundActor);

	if (IsValid(RacingManager))
	{
		UE_LOG(LogTemp, Error, TEXT("Racing Manager Found!"));
		RacingManager->raceStart();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Racing Manager NOT Found in World!"));
	}
}
