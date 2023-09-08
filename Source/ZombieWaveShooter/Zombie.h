// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "Zombie.generated.h"

UCLASS()
class ZOMBIEWAVESHOOTER_API AZombie : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZombie();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float Speed;

	bool bSeenPlayer;
	bool bIsAttacking;
	bool bDead;

	FTimerHandle Patrol_Handle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AI)
		UPawnSensingComponent* PawnSensing;
	TObjectPtr<AAIController> AIController;

	TObjectPtr<UAnimInstance> AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<UAnimMontage> AttackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<UAnimMontage> HitAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<UAnimationAsset> DeathAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<UArrowComponent> SphereArrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<USoundBase> AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<USoundAttenuation> AttackSoundAttenuation;

	FScriptDelegate Delegate_OnMontageNotifyBegin;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Patrol();

	UFUNCTION()
		void OnSeePawn(APawn* Pawn);

	void Attack(float Distance, APawn* Pawn);

	UFUNCTION()
		void OnMontageNotifyBegin();

	void Die();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
