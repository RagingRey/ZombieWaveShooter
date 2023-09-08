// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoCrate.h"

#include "ZombieWaveShooterCharacter.h"

// Sets default values
AAmmoCrate::AAmmoCrate()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("Rotation Component"));

	this->RootComponent = MeshComponent;
	SphereComponent->SetupAttachment(MeshComponent);
}

// Called when the game starts or when spawned
void AAmmoCrate::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AAmmoCrate::OnPlayerHit);
}

void AAmmoCrate::OnPlayerHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(AZombieWaveShooterCharacter* Character = Cast<AZombieWaveShooterCharacter>(OtherActor))
	{
		Character->FillAmmo();
		UGameplayStatics::PlaySound2D(this, PickUpSound);
		Destroy();
	}
}

// Called every frame
void AAmmoCrate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}