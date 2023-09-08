// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Zombie.h"
#include "Kismet/GameplayStatics.h"
#include "ZombieWaveShooterCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AZombieWaveShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;
	
public:
	AZombieWaveShooterCharacter();

protected:
	UPROPERTY(BlueprintReadOnly)
		float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsAiming;

	bool bDead;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TObjectPtr<USkeletalMeshComponent> GunMesh;

	UAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AActor> ZombieActor_Class;
	TArray<TObjectPtr<AActor>> ZombieActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<UParticleSystem> BloodParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<UUserWidget> Overlay_Widget_Class;
	TObjectPtr<UUserWidget> Overlay_Widget;

	//Gun Clip and ammo logic
	UPROPERTY(BlueprintReadOnly)
		int CurrentAmmo;

	UPROPERTY(BlueprintReadOnly)
		int TotalAmmo;

	int ClipSize = 8;
	bool bIsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimMontage* IdleFireAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimMontage* AimFireAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimationAsset* PistolFireAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimMontage* ReloadAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimationAsset* PistolReloadAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundBase* EmptyPistolClip;

	FScriptDelegate Delegate_OnReloadMontageEnd;

	FHitResult Fire_HitResult;
	FCollisionObjectQueryParams Fire_CollisionObjectQueryParams;
	FCollisionQueryParams Fire_CollisionQueryParams;

protected:
	virtual void BeginPlay() override;

	void Aim();
	void StopAiming();
	void Fire();
	void LineTrace(FHitResult HitResult, FVector StartLocation, FVector EndLocation, FCollisionObjectQueryParams CollisionObjectQueryParams, 
		FCollisionQueryParams CollisionQueryParams);
	void Reload();

	void Die();
	void Regenerate(float DeltaTime);

	UFUNCTION()
		void SetIsReloadingAfterMontage() { if (bIsReloading) { bIsReloading = false; } }

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	virtual void Tick(float DeltaSeconds) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void GetZombiesCollision();

	void FillAmmo();
};
