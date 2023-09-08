// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnPoints.generated.h"

UCLASS()
class ZOMBIEWAVESHOOTER_API ASpawnPoints : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnPoints();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TObjectPtr<UStaticMeshComponent> StaticMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
