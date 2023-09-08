// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieShooter_GameMode.h"

#include "AmmoCrate.h"
#include "Zombie.h"
#include "ZombieWaveShooterCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AZombieShooter_GameMode::AZombieShooter_GameMode()
	:Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	CurrentWave = 1;
	MaxWave = 3;

	bPlayerWins = false;
}

void AZombieShooter_GameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ZombieSpawnPoints_Class, ZombieSpawnPoints);

	SpawnEnemy();
	SpawnAmmo();
}

void AZombieShooter_GameMode::SpawnEnemy()
{
	ZombiesSpawned = SpawnAmount * CurrentWave;

	for(SIZE_T i {0}; i < ZombiesSpawned; i++)
	{
		const int VariablePoint = FMath::RandRange(0, ZombieSpawnPoints.Num() - 1);
		GetWorld()->SpawnActor<AZombie>(Zombie_Class, ZombieSpawnPoints[VariablePoint]->GetActorLocation(), ZombieSpawnPoints[VariablePoint]->GetActorRotation());
	}

	AZombieWaveShooterCharacter* Player = Cast<AZombieWaveShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	Player->GetZombiesCollision();
}

void AZombieShooter_GameMode::SpawnAmmo()
{
	if(!AmmoSpawnPoints.IsEmpty() && AmmoCrate_Class)
	{
		if(AActor* PreviousAmmoCrate = UGameplayStatics::GetActorOfClass(GetWorld(), AmmoCrate_Class))
		{
			PreviousAmmoCrate->Destroy();
		}

		int VariablePoint = FMath::RandRange(0, AmmoSpawnPoints.Num() - 1);
		GetWorld()->SpawnActor<AAmmoCrate>(AmmoCrate_Class, AmmoSpawnPoints[VariablePoint], FRotator::ZeroRotator);
	}
}

void AZombieShooter_GameMode::ZombieKilled()
{
	ZombiesSpawned--;

	if(ZombiesSpawned <= 0)
	{
		IncrementWaveLevel();
	}
}

void AZombieShooter_GameMode::IncrementWaveLevel()
{
	if(NextWave_UI_Class)
	{
		NextWave_UI = CreateWidget(GetWorld(), NextWave_UI_Class);
		NextWave_UI->AddToViewport();

		FTimerHandle UIHandle;
		GetWorldTimerManager().SetTimer(UIHandle, this, &AZombieShooter_GameMode::RemoveNextWaveUI, 4.0f);
	}

	if(CurrentWave >= MaxWave)
	{
		bPlayerWins = true;
		PlayerWins();
	}
	else
	{
		CurrentWave++;
		SpawnEnemy();
		SpawnAmmo();
	}
}

void AZombieShooter_GameMode::RemoveNextWaveUI()
{
	if(NextWave_UI->IsInViewport())
	{
		NextWave_UI->RemoveFromParent();
	}
}

void AZombieShooter_GameMode::PlayerWins()
{
	APlayerController* PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(FInputModeGameAndUI());

	if(YouWinLose_UI_Class)
	{
		YouWinLose_UI = CreateWidget(GetWorld(), YouWinLose_UI_Class);
		YouWinLose_UI->AddToViewport();
	}
}

void AZombieShooter_GameMode::PlayerLose()
{
	bPlayerWins = false;

	APlayerController* PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(FInputModeUIOnly());

	if (YouWinLose_UI_Class)
	{
		YouWinLose_UI = CreateWidget(GetWorld(), YouWinLose_UI_Class);
		YouWinLose_UI->AddToViewport();
	}
}

void AZombieShooter_GameMode::Restart()
{
	UGameplayStatics::OpenLevel(this, FName("BridgeMap"), false);
	APlayerController* PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetInputMode(FInputModeGameOnly());
}
