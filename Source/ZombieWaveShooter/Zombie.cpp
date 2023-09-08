// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie.h"

#include "NavigationSystem.h"
#include "ZombieShooter_GameMode.h"
#include "ZombieWaveShooterCharacter.h"
#include "AI/NavigationSystemBase.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AZombie::AZombie()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->bUseControllerRotationYaw = false;
	this->AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	this->GetCharacterMovement()->bOrientRotationToMovement = true;
	this->GetCharacterMovement()->MaxWalkSpeed = 60.0f;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>("Pawn Sensing");
	PawnSensing->HearingThreshold = 1388.0f;
	PawnSensing->SightRadius = 1440.0f;

	SphereArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Sphere Arrow Component"));
	SphereArrow->SetupAttachment(this->GetCapsuleComponent());
	SphereArrow->SetRelativeLocation(FVector(70.0f, 0.0f, 0.0f));

	Health = 80.0f;

	bSeenPlayer = false; 
	bIsAttacking = false;
	bDead = false;
}

// Called when the game starts or when spawned
void AZombie::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(this->GetController());

	AnimInstance = this->GetMesh()->GetAnimInstance();

	PawnSensing->OnSeePawn.AddDynamic(this, &AZombie::OnSeePawn);

	GetWorldTimerManager().SetTimer(Patrol_Handle, this, &AZombie::Patrol, 3.0f, true);

	Delegate_OnMontageNotifyBegin.BindUFunction(this, FName("OnMontageNotifyBegin"));
	AnimInstance->OnPlayMontageNotifyBegin.Add(Delegate_OnMontageNotifyBegin);
}

// Called every frame
void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Speed = this->GetCharacterMovement()->Velocity.Length();
}

// Called to bind functionality to input
void AZombie::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AZombie::Patrol()
{
	if(!bSeenPlayer && !bDead)
	{
		FNavLocation NavLocation;
		UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());

		/*if (NavigationSystem->GetRandomReachablePointInRadius(this->GetActorLocation(), 200.0f, NavLocation))
		{
			AIController->MoveTo(NavLocation.Location);
		}*/

		AIController->MoveTo(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	}
}

void AZombie::OnSeePawn(APawn* Pawn)
{
	if(!bDead && !AnimInstance->Montage_IsPlaying(AttackAnimation))
	{
		if (Pawn == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			bSeenPlayer = true;
			this->GetCharacterMovement()->MaxWalkSpeed = 250.0f;
			AIController->MoveTo(Pawn);

			const float Distance = this->GetDistanceTo(Pawn);
			Attack(Distance, Pawn);
		}
	}
}

void AZombie::Attack(float Distance, APawn* Pawn)
{
	if (!bIsAttacking && Distance < 200.0f)
	{
		bIsAttacking = true;
		AnimInstance->Montage_Play(AttackAnimation);

		if(Distance > 200.0f)
		{
			AnimInstance->Montage_Stop(1.0f, AttackAnimation);
		}

		FTimerDelegate Delegate;
		FTimerHandle Attack_Handle;

		Delegate.BindLambda([&]
			{
				bIsAttacking = false;
			});

		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, this->GetActorLocation(), this->GetActorRotation(), 1, 1, 0, AttackSoundAttenuation);
		GetWorldTimerManager().SetTimer(Attack_Handle, Delegate, 3.0f, false);
	}
}

void AZombie::OnMontageNotifyBegin()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OutActors;

	ActorsToIgnore.Add(this);

	if(UKismetSystemLibrary::SphereOverlapActors(this, SphereArrow->GetComponentLocation(), 50.0f, ObjectTypes, AZombieWaveShooterCharacter::StaticClass(), ActorsToIgnore, OutActors))
	{
		UKismetSystemLibrary::DrawDebugSphere(this, SphereArrow->GetComponentLocation(), 50.0f, 12, FLinearColor::Red, 7.0f);

		for(AActor* EachActor: OutActors)
		{
			AZombieWaveShooterCharacter* ZombieCharacter = Cast<AZombieWaveShooterCharacter>(EachActor);
			ZombieCharacter->TakeDamage(20.0f, FDamageEvent(), nullptr, this);
		}
	}
}

float AZombie::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                          AActor* DamageCauser)
{
	if(DamageAmount > 0.0f && Health > 0.0f)
	{
		Health -= DamageAmount;
		UE_LOG(LogTemp, Warning, TEXT("%f"), Health);
		AnimInstance->Montage_Play(HitAnimation);

		if (Health <= 0.0f)
			Die();
	}

	return DamageAmount;
}

void AZombie::Die()
{
	bDead = true;
	AZombieShooter_GameMode* GameMode = Cast<AZombieShooter_GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->ZombieKilled();
	this->GetCharacterMovement()->StopMovementImmediately();
	this->GetMesh()->PlayAnimation(DeathAnimation, false);
	this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->GetCapsuleComponent()->DestroyComponent();
}
