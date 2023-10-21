// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Math/UnrealMathUtility.h"

AWeapon::AWeapon()
{

	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//keep weapon upright
	if (GetItemState()==EItemState::EIS_Falling && bFalling) {
		const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation = FRotator(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward = GetItemMesh()->GetForwardVector();
	const FVector MeshRight = GetItemMesh()->GetRightVector();
	

	//direction in which we throw the weapon
	FVector ImpulseDirec = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandonRot = FMath::FRandRange(0, 30);
	ImpulseDirec = MeshRight.RotateAngleAxis(RandonRot, FVector(0.f, 0.f, 1.f));
	ImpulseDirec *= 2000.f;
	GetItemMesh()->AddImpulse(ImpulseDirec);

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);


}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::DecrementAmmo()
{
	if (Ammo - 1 <= 0) {
		Ammo = 0;
	}
	else {
		--Ammo;
	}
}

bool AWeapon::ClipIsFull()
{
	return Ammo>=MagazineCapacity;
}

void AWeapon::ReloadAmmo(int32 amount)
{
	checkf(Ammo+amount<=MagazineCapacity, TEXT("Attempted top reload with more than magazine capacity"));
	Ammo += amount;
}
