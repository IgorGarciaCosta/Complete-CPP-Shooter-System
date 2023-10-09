// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EWeaponType :uint8 {
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;


	//adds impulse to weapon
	void ThrowWeapon();

protected:
	void StopFalling();

private:

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime = .7f;
	bool bFalling = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 Ammo = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 MagazineCapacity = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EWeaponType WeaponType = EWeaponType::EWT_SubmachineGun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EAmmoType AmmoType = EAmmoType::EAT_9mm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName ReloadMontageSection = FName(TEXT("ReloadSMG"));

public:

	FORCEINLINE int32 GetAmmo()const { return Ammo; };

	FORCEINLINE int32 GetMagazineCapacity()const { return MagazineCapacity; };

	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; };

	FORCEINLINE EAmmoType GetAmmoType()const { return AmmoType; };

	FORCEINLINE FName GetReloadMontageSection()const { return ReloadMontageSection; };


	void DecrementAmmo();
	

	void ReloadAmmo(int32 amount);
};
