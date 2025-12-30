#include "gamemode/V12_MainGameMode.h"
#include "Player/V12PlayerState.h"
#include "V12_the_gamePlayerController.h"
#include "Game/V12GameInstance.h"

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

		A_PC.AddUnique(PC); // 중복 방지

		
		if (AV12PlayerState* PS = PC->GetPlayerState<AV12PlayerState>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Travel Success - Player Name: %s"), *PS->PlayerName);
		}

	}
}

void AV12_MainGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// GameInstance 가져오기
	UV12GameInstance* GI = GetGameInstance<UV12GameInstance>();
	if (IsValid(GI))
	{
		UE_LOG(LogTemp, Error, TEXT("PostSeamlessTravel: Player Count = %d"), GI->allPlayerCount);
		UE_LOG(LogTemp, Error, TEXT("PostSeamlessTravel: Player Count = %d"), GI->allPlayerCount);
		UE_LOG(LogTemp, Error, TEXT("PostSeamlessTravel: Player Count = %d"), GI->allPlayerCount);
		UE_LOG(LogTemp, Error, TEXT("PostSeamlessTravel: Player Count = %d"), GI->allPlayerCount);
		UE_LOG(LogTemp, Error, TEXT("PostSeamlessTravel: Player Count = %d"), GI->allPlayerCount);
	}

	StartCountdown();	
}

void AV12_MainGameMode::BeginPlay()
{
	Super::BeginPlay();

	/// 점수 올라가는지 확인만 하기 위한 기능
	GetWorldTimerManager().SetTimer(
		TestTimer,
		this,
		&AV12_MainGameMode::TestFunc,
		2.0f,
		true);


	/// 정확한 플레이어 갯수를 체크하고 시작하도록 변경
	// StartCountdown();
}

/// Seamless Travel 이용시 PostLogin은 사용하지 않는다!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
			//BP에서 색 지정 후, 복제를 통해 자동 적용
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
		UE_LOG(LogTemp, Error, TEXT("APC Count: %d"), A_PC.Num());
		UE_LOG(LogTemp, Error, TEXT("APC Count: %d"), A_PC.Num());
		UE_LOG(LogTemp, Error, TEXT("APC Count: %d"), A_PC.Num());


		if (GI->allPlayerCount == A_PC.Num())
		{
			StartCountdown();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!"));
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!"));
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!"));
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!"));
		UE_LOG(LogTemp, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!"));
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
	/// 끝나면 제거
	countdownCount--;
	if (countdownCount < 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimer);

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
