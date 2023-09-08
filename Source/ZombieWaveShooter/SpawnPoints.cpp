// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnPoints.h"

// Sets default values
ASpawnPoints::ASpawnPoints()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comopenent"));

	StaticMesh->SetSimulatePhysics(false);
	StaticMesh->SetCollisionObjectType(ECollisionChannel::ECC_OverlapAll_Deprecated);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetCollisionProfileName(TEXT("WorldStatic"));
}

// Called when the game starts or when spawned
void ASpawnPoints::BeginPlay()
{
	Super::BeginPlay();

	StaticMesh->SetVisibility(false);
}
