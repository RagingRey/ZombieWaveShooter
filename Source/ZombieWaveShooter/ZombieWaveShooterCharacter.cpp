// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZombieWaveShooterCharacter.h"
#include "ZombieWaveShooterProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Zombie.h"
#include "ZombieShooter_GameMode.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


//////////////////////////////////////////////////////////////////////////
// AZombieWaveShooterCharacter

AZombieWaveShooterCharacter::AZombieWaveShooterCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->SetupAttachment(Mesh1P, FName("PistolSocket"));

	Health = 80.0f;
	bIsAiming = false;
	bDead = false;

	CurrentAmmo = ClipSize;
	TotalAmmo = 30;
	bIsReloading = false;
}

void AZombieWaveShooterCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	AnimInstance = Mesh1P->GetAnimInstance();

	GetZombiesCollision();

	Delegate_OnReloadMontageEnd.BindUFunction(this, FName("SetIsReloadingAfterMontage"));

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

//////////////////////////////////////////////////////////////////////////// Input

void AZombieWaveShooterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AZombieWaveShooterCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AZombieWaveShooterCharacter::Look);
	}

	//Aim
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AZombieWaveShooterCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AZombieWaveShooterCharacter::StopAiming);

	//Fire
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AZombieWaveShooterCharacter::Fire);

	//Reload
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AZombieWaveShooterCharacter::Reload);
}

void AZombieWaveShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Speed = this->GetCharacterMovement()->Velocity.Length();

	Regenerate(DeltaSeconds);

	if (Health > 50.0f && Overlay_Widget && Overlay_Widget->IsInViewport())
	{
		Overlay_Widget->RemoveFromParent();
	}
}

void AZombieWaveShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AZombieWaveShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AZombieWaveShooterCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AZombieWaveShooterCharacter::GetHasRifle()
{
	return bHasRifle;
}

void AZombieWaveShooterCharacter::Aim()
{
	bIsAiming = true;
	this->GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}

void AZombieWaveShooterCharacter::StopAiming()
{
	bIsAiming = false;
	this->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

void AZombieWaveShooterCharacter::Fire()
{
	if(CurrentAmmo > 0 && !bIsReloading)
	{
		FVector StartLocation = GunMesh->GetSocketLocation(FName("MuzzleFlash"));
		FRotator Rotation = GunMesh->GetSocketRotation(FName("MuzzleFlash"));

		Fire_CollisionQueryParams.bReturnPhysicalMaterial = true;
		Fire_CollisionQueryParams.AddIgnoredActor(this);

		if (bIsAiming)
		{
			FVector CamStart = FirstPersonCameraComponent->GetComponentLocation();
			FVector CamEnd = CamStart + FirstPersonCameraComponent->GetForwardVector() * 35000;
			FVector EndLocation = StartLocation + UKismetMathLibrary::FindLookAtRotation(StartLocation, CamEnd).Vector() * 3500.0f;

			LineTrace(OUT Fire_HitResult, StartLocation, EndLocation, Fire_CollisionObjectQueryParams, Fire_CollisionQueryParams);

			AnimInstance->Montage_Play(AimFireAnim);
		}
		else
		{
			FVector EndLocation = StartLocation + Rotation.Vector() * 3500.0f;

			LineTrace(OUT Fire_HitResult, StartLocation, EndLocation, Fire_CollisionObjectQueryParams, Fire_CollisionQueryParams);

			AnimInstance->Montage_Play(IdleFireAnim);
		}

		CurrentAmmo--;

		GunMesh->PlayAnimation(PistolFireAnim, false);
	}
	else
	{
		UGameplayStatics::PlaySound2D(this, EmptyPistolClip);
	}
}

void AZombieWaveShooterCharacter::LineTrace(FHitResult HitResult, FVector StartLocation, FVector EndLocation,
	FCollisionObjectQueryParams CollisionObjectQueryParams, FCollisionQueryParams CollisionQueryParams)
{
	if (GetWorld()->LineTraceSingleByObjectType(OUT HitResult, StartLocation, EndLocation, CollisionObjectQueryParams, CollisionQueryParams))
	{
		if (AZombie* Zombie = Cast<AZombie>(HitResult.GetActor()))
		{
			FString Name = HitResult.GetComponent()->GetName();
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Name)
			if (HitResult.PhysMaterial->SurfaceType == SurfaceType1)
			{
				Zombie->TakeDamage(50.0f, FDamageEvent(), nullptr, this);
			}
			else if (HitResult.PhysMaterial->SurfaceType == SurfaceType2)
			{
				Zombie->TakeDamage(20.0f, FDamageEvent(), nullptr, this);
			}

			UGameplayStatics::SpawnEmitterAtLocation(this, BloodParticle, HitResult.ImpactPoint, FRotator::ZeroRotator);
		}

		//UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), 20.0f, CamStart, HitResult, this->GetInstigatorController(), this, nullptr);
		DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Red, false, 3.0f, 0, 0.3f);
		DrawDebugLine(GetWorld(), HitResult.ImpactPoint, HitResult.TraceEnd, FColor::Green, false, 3.0f, 0, 0.3f);
	}
}

void AZombieWaveShooterCharacter::Reload()
{
	if(CurrentAmmo < ClipSize && TotalAmmo > 0)
	{
		bIsReloading = true;

		const int AmmoToAdd = ClipSize - CurrentAmmo;
		if(TotalAmmo > AmmoToAdd)
		{
			CurrentAmmo = FMath::Clamp((AmmoToAdd + CurrentAmmo), 0, ClipSize);
			TotalAmmo -= AmmoToAdd;
		}
		else
		{
			CurrentAmmo = FMath::Clamp((TotalAmmo + CurrentAmmo), 0, ClipSize);
			TotalAmmo -= TotalAmmo;
		}

		AnimInstance->Montage_Play(ReloadAnim);
		GunMesh->PlayAnimation(PistolReloadAnim, false);

		AnimInstance->OnMontageEnded.Add(Delegate_OnReloadMontageEnd);
	}
}

float AZombieWaveShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount > 0.0f && Health > 0.0f)
	{
		Health -= DamageAmount;
		UE_LOG(LogTemp, Warning, TEXT("%f"), Health);

		if(Health <= 50.0f && Overlay_Widget_Class)
		{
			if(!Overlay_Widget)
			{
				Overlay_Widget = CreateWidget(GetWorld(), Overlay_Widget_Class);
				Overlay_Widget->AddToViewport();
			}
		}

		if (Health <= 0.0f)
			Die();
	}

	return DamageAmount;
}

void AZombieWaveShooterCharacter::GetZombiesCollision()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ZombieActor_Class, ZombieActors);

	for (auto Zombie : ZombieActors)
	{
		Fire_CollisionQueryParams.AddIgnoredComponent(Cast<AZombie>(Zombie)->GetCapsuleComponent());
	}
}

void AZombieWaveShooterCharacter::Die()
{
	bDead = true;
	this->GetCharacterMovement()->DisableMovement();

	AZombieShooter_GameMode* GameMode = Cast<AZombieShooter_GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->PlayerLose();

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void AZombieWaveShooterCharacter::Regenerate(float DeltaTime)
{
	if(Health < 60.0f)
	{
		Health = FMath::Clamp((Health += DeltaTime * 1.5), 0, 100);
	}
}

void AZombieWaveShooterCharacter::FillAmmo()
{
	TotalAmmo += FMath::RandRange(30, 50);
}