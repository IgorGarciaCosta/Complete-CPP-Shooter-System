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

}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (FollowCamera) {
		DefaultCameraFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = DefaultCameraFOV;
	}
}

void AShooterCharacter::MoveForward(float value)
{
	if (Controller != nullptr && value!=0.f) {
		//find which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};

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
	if (FireSound) {
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* MuzzleSocket = GetMesh()->GetSocketByName("muzzleSocket");

	if (MuzzleSocket) {
		const FTransform SocketTransf = MuzzleSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransf);
		}

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
	UAnimInstance* AnimInstange = GetMesh()->GetAnimInstance();
	if (AnimInstange && HipFireMontage) {
		AnimInstange->Montage_Play(HipFireMontage);
		AnimInstange->Montage_JumpToSection(FName("StartFire"));
	}
}

bool AShooterCharacter::GetBeamEndLoc(const FVector& MuzzleSocketLocation, FVector& OutBeamLoc)
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

	if (bScreenToWorld) {//check if the dproj was successful
		FHitResult ScreenTraceHit;
		const FVector Start = CrossHairWorldPosition;
		const FVector End = CrossHairWorldPosition + CrossHairWorldDirection * 50000.f;


		//set beam endpoint to linetrace endpoint
		OutBeamLoc = End;

		//trace outward from crosshair world loc
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHit.bBlockingHit) {//check if was there a trace hit

			//beamendpoint is now tracehit loc
			OutBeamLoc = ScreenTraceHit.Location;

		}

		//perform a second trace, this time from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart = MuzzleSocketLocation;
		const FVector WeaponTraceEnd = OutBeamLoc;
		//line trace from gun barrel to hit point
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
		if (WeaponTraceHit.bBlockingHit) {//obj between barel and beamEndPoint
			OutBeamLoc = WeaponTraceHit.Location;
		}

		return true;
	}

	return false;
}

void AShooterCharacter::AimingBtnPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingBtnReleased()
{
	bAiming = false;
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
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime,2.25f);
	}
	else {
		//shrink crisshairs quickly when fall
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);

	}

	CrosshairSpreadMult = .5f + CrosshairVelocityFactor + CrosshairInAirFactor;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CameraInterpZoom(DeltaTime);

	SetInterpRates();

	//calculates crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("FireBtn", IE_Pressed, this, &AShooterCharacter::FireWeapon);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);

	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("AimingBtn", IE_Pressed, this, &AShooterCharacter::AimingBtnPressed);
	PlayerInputComponent->BindAction("AimingBtn", IE_Released, this, &AShooterCharacter::AimingBtnReleased);

}

float AShooterCharacter::GetCrosshairSpreadMult() const
{
	return CrosshairSpreadMult;
}

