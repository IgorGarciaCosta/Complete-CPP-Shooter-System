// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
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
		int32 Ammo = 0;

public:

	FORCEINLINE int32 GetAmmo()const { return Ammo; };

	void DecrementAmmo();
	
};
