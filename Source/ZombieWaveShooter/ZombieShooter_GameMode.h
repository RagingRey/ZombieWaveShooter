// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmmoCrate.h"
#include "SpawnPoints.h"
#include "Zombie.h"
#include "GameFramework/GameMode.h"
#include "ZombieShooter_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEWAVESHOOTER_API AZombieShooter_GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AZombieShooter_GameMode();

protected:
	UPROPERTY(BlueprintReadOnly)
		int CurrentWave;

	UPROPERTY(BlueprintReadOnly)
		int MaxWave;

	UPROPERTY(EditAnywhere)
		TSubclassOf<ASpawnPoints> ZombieSpawnPoints_Class;
	TArray<TObjectPtr<AActor>> ZombieSpawnPoints;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AAmmoCrate> AmmoCrate_Class;
	UPROPERTY(EditAnywhere)
		TArray<FVector> AmmoSpawnPoints;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AZombie> Zombie_Class;

	UPROPERTY(BlueprintReadOnly)
		int ZombiesSpawned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int SpawnAmount = 4;

	UPROPERTY(BlueprintReadOnly)
		bool bPlayerWins;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<UUserWidget> NextWave_UI_Class;
	TObjectPtr<UUserWidget> NextWave_UI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<UUserWidget> YouWinLose_UI_Class;
	TObjectPtr<UUserWidget> YouWinLose_UI;

protected:
	virtual void BeginPlay() override;

	void SpawnEnemy();
	void SpawnAmmo();
	void IncrementWaveLevel();
	void RemoveNextWaveUI();

	void PlayerWins();

	UFUNCTION(BlueprintCallable)
		void Restart();

public:
	void ZombieKilled();
	void PlayerLose();
};
