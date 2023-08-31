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
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

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
	AddControllerPitchInput(rate * BaseLookUpTurnRate * GetWorld()->GetDeltaSeconds());
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
			FVector BeamEndPoint = End;

			//trace outward from crosshair world loc
			GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
			
			if (ScreenTraceHit.bBlockingHit) {//check if was there a trace hit
				
				//beamendpoint is now tracehit loc
				BeamEndPoint = ScreenTraceHit.Location;
				
			}

			//perform a second trace, this time from the gun barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart = SocketTransf.GetLocation();
			const FVector WeaponTraceEnd = BeamEndPoint;
			//line trace from gun barrel to hit point
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
			if (WeaponTraceHit.bBlockingHit) {//obj between barel and beamEndPoint
				BeamEndPoint = WeaponTraceHit.Location;
			}

			//spawn particles after updated beamendpoint
			if (ImpactParticles) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, ScreenTraceHit.Location);
			}

			if (BeamParticles) {
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransf);
				if (Beam) {
					Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
				}
			}



		}

		//FHitResult FireHit;
		//const FVector Start = SocketTransf.GetLocation();
		//const FQuat Rotation = SocketTransf.GetRotation();
		//const FVector RotationAxis = Rotation.GetAxisX();
		//const FVector End = Start + RotationAxis * 50000.f;

		//FVector BeamEndPoint = End;

		//GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
		//if (FireHit.bBlockingHit) {
		//	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
		//	//DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f);
		//	BeamEndPoint = FireHit.Location;
		//	if (ImpactParticles) {
		//		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
		//	}
		//
		//}

		//if (BeamParticles) {
		//	UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransf);
		//	if (Beam) {
		//		Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
		//	}
		//}
	
	}
	UAnimInstance* AnimInstange = GetMesh()->GetAnimInstance();
	if (AnimInstange && HipFireMontage) {
		AnimInstange->Montage_Play(HipFireMontage);
		AnimInstange->Montage_JumpToSection(FName("StartFire"));
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

}

