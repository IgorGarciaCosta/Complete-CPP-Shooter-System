// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Weapon.h"
#include "Ammo.h"



// Sets default values
AShooterCharacter::AShooterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;


	//controller only affets the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//configure char movement
	GetCharacterMovement()->bOrientRotationToMovement = false;//char moves in the input direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);//moves at this to rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>("HandSceneComponent");


	//constructing  scene comps
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>("WeaponInterpComp");
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>("InterpComp1");
	InterpComp1->SetupAttachment(GetFollowCamera());
	InterpComp2 = CreateDefaultSubobject<USceneComponent>("InterpComp2");
	InterpComp2->SetupAttachment(GetFollowCamera());
	InterpComp3 = CreateDefaultSubobject<USceneComponent>("InterpComp3");
	InterpComp3->SetupAttachment(GetFollowCamera());
	InterpComp4 = CreateDefaultSubobject<USceneComponent>("InterpComp4");
	InterpComp4->SetupAttachment(GetFollowCamera());
	InterpComp5 = CreateDefaultSubobject<USceneComponent>("InterpComp5");
	InterpComp5->SetupAttachment(GetFollowCamera());
	InterpComp6 = CreateDefaultSubobject<USceneComponent>("InterpComp6");
	InterpComp6->SetupAttachment(GetFollowCamera());

}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera) {
		DefaultCameraFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = DefaultCameraFOV;
	}

	EquipWeapon(SpawnDefaultWeapon());
	InitializeAmmoMap();

	//create fintepLoc structs for each interp loc. Add to array
	InitializeInterpLocations();
}

void AShooterCharacter::MoveForward(float value)
{
	if (Controller != nullptr && value != 0.f) {
		//find which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };

		AddMovementInput(Direction, value);
	}
}

void AShooterCharacter::MoveRight(float value)
{
	if (Controller != nullptr && value != 0.f) {
		//find which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };

		AddMovementInput(Direction, value);
	}
}

void AShooterCharacter::TurnAtRate(float rate)
{
	AddControllerYawInput(rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float rate)
{
	AddControllerPitchInput(rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor;
	if (bAiming) {
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else {
		TurnScaleFactor = MouseHipTurnRate;
	}

	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor;
	if (bAiming) {
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else {
		LookUpScaleFactor = MouseHipLookUpRate;
	}

	AddControllerYawInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (WeaponHasAmmo() && EquippedWeapon != nullptr && CombatState == ECombatState::ECSUnnocupied) {

		PlayFireSound();

		SendBullet();

		PlayGunfireMontage();

		EquippedWeapon->DecrementAmmo();
		StartFireTimer();
	}


}

bool AShooterCharacter::GetBeamEndLoc(const FVector& MuzzleSocketLocation, FVector& OutBeamLoc)
{
	//check for croshairs trace hit
	FHitResult CrossHairHitResult;
	bool bCrissHairHit = TraceUnderCrosshairs(CrossHairHitResult, OutBeamLoc);
	if (bCrissHairHit) {
		OutBeamLoc = CrossHairHitResult.Location;
	}
	else {//no crosshair trace hit
		//outbeamloc is the end location for the linetrace
	}

	//perform trace from gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector StartToEnd = OutBeamLoc - MuzzleSocketLocation;
	const FVector WeaponTraceEnd = MuzzleSocketLocation + StartToEnd * 1.25f;
	//line trace from gun barrel to hit point
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (WeaponTraceHit.bBlockingHit) {//obj between barel and beamEndPoint
		OutBeamLoc = WeaponTraceHit.Location;
		return true;
	}
	return false;


}

void AShooterCharacter::AimingBtnPressed()
{
	bAimingBtnPressed = true;
	if (CombatState != ECombatState::ECSReloading) {
		Aim();
	}
}

void AShooterCharacter::AimingBtnReleased()
{
	bAimingBtnPressed = false;
	StopAiming();
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if (bAiming) {//interp to zoomed fov
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV,
			ZoomedCameraFOV,
			DeltaTime,
			ZoomInterpSpeed);
	}
	else {//interp to default fov
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV,
			DefaultCameraFOV,
			DeltaTime,
			ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetInterpRates()
{
	if (bAiming) {
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else {
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSPeedRange = FVector2D(0.f, 600.f);
	FVector2D VelMultRange = FVector2D(0.f, 1.f);
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSPeedRange,
		VelMultRange,
		Velocity.Size()
	);

	if (GetCharacterMovement()->IsFalling()) {
		//spread he crisshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else {
		//shrink crisshairs quickly when fall
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);

	}

	if (bAiming) {
		//shrink crosshairs a small amount quickly
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.6f,
			DeltaTime,
			30.f
		);

	}

	else {
		//spread crosshairs back to normal quickly
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.0f,
			DeltaTime,
			30.f
		);

	}

	if (bFiringBullets) {
		CrosshairSHootingFactor = FMath::FInterpTo(CrosshairSHootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else {
		CrosshairSHootingFactor = FMath::FInterpTo(CrosshairSHootingFactor, 0.0f, DeltaTime, 60.f);

	}

	CrosshairSpreadMult = .5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairSHootingFactor;
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullets = false;

}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullets = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimerHandle, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);

}

void AShooterCharacter::FireBtnPressed()
{
	bFireBtnPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireBtnReleased()
{
	bFireBtnPressed = false;

}

void AShooterCharacter::StartFireTimer()
{
	SetCombatState(ECombatState::ECSFireTimerInProgress);
	GetWorldTimerManager().SetTimer(
		AutoFreTimerHandle,
		this,
		&AShooterCharacter::AutoFireReset,
		AutomaticFireRate);
}

void AShooterCharacter::AutoFireReset()
{
	SetCombatState(ECombatState::ECSUnnocupied);
	if (WeaponHasAmmo()) {
		if (bFireBtnPressed) {
			FireWeapon();
		}
	}

	else {
		//reload
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{

	//get currrent viewportSize
	FVector2D ViewPortSize;
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	//get crosshair loc in screen
	FVector2D CrosshairLoc(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);
	CrosshairLoc.Y -= 50.f;
	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;

	//get world pos and dir of the crosshasr
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLoc,
		CrossHairWorldPosition, CrossHairWorldDirection);

	if (bScreenToWorld) {
		//trave from crosshair world loc outward

		const FVector Start = CrossHairWorldPosition;
		const FVector End = Start + CrossHairWorldDirection * 50000.f;
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit) {
			OutHitLocation = OutHitResult.Location;
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "OutHitResult.bBlockingHit true");
			return true;

		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "OutHitResult.bBlockingHit false");

	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bSHouldTraceForItems) {
		FHitResult ItemTraceResult;
		FVector HitLoc;
		TraceUnderCrosshairs(ItemTraceResult, HitLoc);


		if (ItemTraceResult.bBlockingHit) {
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);

			if (TraceHitItem) {
				LeastHitItem = TraceHitItem;

				if (TraceHitItem->GetPickupWidget()) {
					//show item's pickup widget
					TraceHitItem->GetPickupWidget()->SetVisibility(true);
					TraceHitItem->EnableCustomDepth();
				}
			}
			else if (LeastHitItem) {
				LeastHitItem->GetPickupWidget()->SetVisibility(false);
				LeastHitItem->DisableCustomDepth();
			}


		}
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	//check tsubclassof var
	if (DefaultWeaponClass) {
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);

	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{


	if (WeaponToEquip) {


		//get hand socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

		if (IsValid(HandSocket)) {
			//attach
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon) {
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectBtnPressed()
{

	if (TraceHitItem) {
		TraceHitItem->StartItemCurve(this);

	}

}

void AShooterCharacter::SelectBtnReleased()
{
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AssountRifle, StartingARAmmo);

}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound) {
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* MuzzleSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("muzzleSocket");
	if (MuzzleSocket) {
		const FTransform SocketTransf = MuzzleSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		//muzzle flash
		if (MuzzleFlash) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransf);
		}

		//beam particles
		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLoc(SocketTransf.GetLocation(), BeamEnd);
		if (bBeamEnd) {
			if (ImpactParticles) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				BeamParticles,
				SocketTransf);
			if (Beam) {
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void AShooterCharacter::PlayGunfireMontage()
{
	//fire anim
	UAnimInstance* AnimInstange = GetMesh()->GetAnimInstance();
	if (AnimInstange && HipFireMontage) {
		AnimInstange->Montage_Play(HipFireMontage);
		AnimInstange->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECSUnnocupied) return;

	

	if (EquippedWeapon == nullptr) return;

	//do we have ammo of the correct type?
	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull()) {
		if (bAiming) {
			StopAiming();
		}

		SetCombatState(ECombatState::ECSReloading);

		//play reload montage
		FName MontageSection = EquippedWeapon->GetReloadMontageSection();

		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if (AnimInst && ReloadMontage) {
			AnimInst->Montage_Play(ReloadMontage);
			AnimInst->Montage_JumpToSection(MontageSection);
		}

		//reload
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();
	if (AmmoMap.Contains(AmmoType)) {
		return AmmoMap[AmmoType] > 0;
	}
	return false;
}

void AShooterCharacter::GrabClip()
{

	if (EquippedWeapon == nullptr || HandSceneComponent == nullptr) return;

	int32 ClipBoneIndex = EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName());
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{

	EquippedWeapon->SetMovingClip(false);

}

void AShooterCharacter::CrouchBtnPressed()
{

	if (!GetCharacterMovement()->IsFalling()) {
		bCrouching = !bCrouching;
	}
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching) {
		GetCharacterMovement()->MaxWalkSpeed = BasedMovementSpeed;
	}
}

void AShooterCharacter::pickUpAmmo(AAmmo* Ammo)
{
	//check if ammo map conain ammo type
	if (AmmoMap.Find(Ammo->GetAmmoType())) {
		//et amount of ammo in ammo map for a specific ammo type
		int32 AmmoCount = AmmoMap[Ammo->GetAmmoType()];
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType()) {
		//check if gun is empty
		if (EquippedWeapon->GetAmmo() == 0) {
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	//creating the struct
	FInterpLocation WeaponLocation{ WeaponInterpComp, 0, };
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);

	FInterpLocation InterpLoc2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);
	FInterpLocation InterpLoc3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLoc3);
	FInterpLocation InterpLoc4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLoc4);
	FInterpLocation InterpLoc5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLoc5);
	FInterpLocation InterpLoc6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLoc6);
}

int32 AShooterCharacter::GetInterpLocationLowestItemCountIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;//macro that holds the maximum value of an int32

	for (int32 i = 1; i < InterpLocations.Num();i++) {
		if (InterpLocations[i].ItemCount < LowestCount) {
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount>1) return;

	if (InterpLocations.Num() >= Index) {
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer,this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::StartEquiupSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);

}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CameraInterpZoom(DeltaTime);

	SetInterpRates();

	//calculates crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);

	TraceForItems();

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("FireBtn", IE_Pressed, this, &AShooterCharacter::FireBtnPressed);
	PlayerInputComponent->BindAction("FireBtn", IE_Released, this, &AShooterCharacter::FireBtnReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadWeapon);


	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectBtnPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectBtnReleased);


	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);

	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("AimingBtn", IE_Pressed, this, &AShooterCharacter::AimingBtnPressed);
	PlayerInputComponent->BindAction("AimingBtn", IE_Released, this, &AShooterCharacter::AimingBtnReleased);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchBtnPressed);

}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 index)
{
	if (index < InterpLocations.Num()) {
		return InterpLocations[index];
	}
	return FInterpLocation();//return an empty FInterpLocation
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0) {
		OverlappedItemCount = 0;
		bSHouldTraceForItems = false;
	}
	else {
		OverlappedItemCount += Amount;
		bSHouldTraceForItems = true;
	}
}

float AShooterCharacter::GetCrosshairSpreadMult() const
{
	return CrosshairSpreadMult;
}

//FVector AShooterCharacter::GetCameraInterpLoc()
//{
//	const FVector CameraWoldLoc = FollowCamera->GetComponentLocation();
//	const FVector CameraForward = FollowCamera->GetForwardVector();
//	return CameraWoldLoc + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
//}

void AShooterCharacter::GetPuckupItem(AItem* item)
{
	item->PlayEquipSound();

	auto weapon = Cast<AWeapon>(item);
	if (weapon) {
		SwapWeapon(weapon);
	}
	auto Ammo = Cast<AAmmo>(item);
	if (IsValid(Ammo)) {
		pickUpAmmo(Ammo);
	}
}

void AShooterCharacter::FinishReloading()
{

	SetCombatState(ECombatState::ECSUnnocupied);

	if (bAimingBtnPressed) {
		Aim();
	}

	//udate ammo map
	if (EquippedWeapon == nullptr) return;

	const auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType)) {
		int32 CarriedAmmo = AmmoMap[AmmoType];//amount of carries ammo of this type

		const int32 magEMptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (magEMptySpace > CarriedAmmo) {
			//reload magazinewith all ammo im carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
		}

		else {
			//fill magazine
			EquippedWeapon->ReloadAmmo(magEMptySpace);
			CarriedAmmo -= magEMptySpace;
		}
		AmmoMap.Add(AmmoType, CarriedAmmo);
	}
}

