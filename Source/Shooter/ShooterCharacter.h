// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward( float value);
	void MoveRight(float value);
	void TurnAtRate(float rate);
	void LookUpAtRate(float rate);

	//rotate controller based on mouse X movement
	void Turn(float Value);
	//rotate controller based on mouse Y movement
	void LookUp(float Value);

	void FireWeapon();

	bool GetBeamEndLoc(const FVector& MuzzleSocketLocation, FVector& OutBeamLoc);

	void AimingBtnPressed() ;
	void AimingBtnReleased();

	void CameraInterpZoom(float DeltaTime);

	void SetInterpRates();

	void CalculateCrosshairSpread(float DeltaTime);

	UFUNCTION()
		void FinishCrosshairBulletFire();
	void StartCrosshairBulletFire();

	void FireBtnPressed();

	void FireBtnReleased();

	void StartFireTimer();

	UFUNCTION()
		void AutoFireReset();

	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	void TraceForItems();

	class AWeapon* SpawnDefaultWeapon();

	void EquipWeapon( AWeapon* WeaponToEquip);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
		class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float BaseTurnRate = 45.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float BaseLookUpRate = 45.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float HipTurnRate = 90.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float HipLookUpRate = 90.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float AimingTurnRate = 20.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float AimingLookUpRate = 20.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MouseHipTurnRate = 1.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MouseHipLookUpRate = 1.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MouseAimingTurnRate = .2f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MouseAimingLookUpRate = .2f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class USoundCue* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* HipFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* BeamParticles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bAiming = false;

	float DefaultCameraFOV = 0.f;
	float ZoomedCameraFOV = 35.f;

	float CameraCurrentFOV = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float ZoomInterpSpeed = 30.f;


	//crosshair spread related variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CrosshairSpreadMult = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CrosshairVelocityFactor = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CrosshairInAirFactor = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CrosshairAimFactor = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CrosshairSHootingFactor = 0.f;

	float ShootTimeDuration = 0.05f;
	bool bFiringBullets = false;
	FTimerHandle CrosshairShootTimerHandle;

	bool bFireBtnPressed = false;

	bool bShouldFire = true;

	float AutomaticFireRate = 0.1f;

	FTimerHandle AutoFreTimerHandle;

	bool bSHouldTraceForItems = false;

	int8 OverlappedItemCount;

	class AItem* LeastHitItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class AWeapon* EquippedWeapon;


	//set this in BP for the default weapon class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeapon> DefaultWeaponClass;

public:
		FORCEINLINE USpringArmComponent* GetCameraBoom()const { return CameraBoom; }

		FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; };

		FORCEINLINE bool GetAIming() const { return bAiming; }

		FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; };

		void IncrementOverlappedItemCount(int8 Amount);

		UFUNCTION(BlueprintCallable)
			float GetCrosshairSpreadMult() const;
};

