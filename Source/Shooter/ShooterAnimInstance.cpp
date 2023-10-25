// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterChar == nullptr) {
		ShooterChar = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterChar) {

		bCrouching = ShooterChar->GetCrouching();
		bReloading = ShooterChar->GetCombatState() == ECombatState::ECSReloading;

		//get lateral speed
		FVector Velocity = ShooterChar->GetVelocity();
		Velocity.Z = 0;
		Speed = Velocity.Size();

		//is char in the air?
		bIsInAir = ShooterChar->GetCharacterMovement()->IsFalling();

		//is accelerating?
		if (ShooterChar->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0) {
			bIsAccelerating = true;
		}
		else {
			bIsAccelerating = false;

		}

		FRotator AimRotation = ShooterChar->GetBaseAimRotation();
		//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Blue, FString::Printf(TEXT("Base AIm Rot: %f"), AimRotation.Yaw));
		FRotator MovementRotator = UKismetMathLibrary::MakeRotFromX(ShooterChar->GetVelocity());
		//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Red, FString::Printf(TEXT("MovementRotator: %f"), MovementRotator.Yaw));
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotator, AimRotation).Yaw;
		//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Red, FString::Printf(TEXT("MovementOffsetYaw: %f"), MovementOffsetYaw));

		if (ShooterChar->GetVelocity().Size() > 0.f) {
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		bAiming = ShooterChar->GetAIming();

		if (bReloading) {
			OffsetState = EOffsetState::EOSReloading;
		}

		else if (bIsInAir) {
			OffsetState = EOffsetState::EOSInAir;
		}

		else if (ShooterChar->GetAIming()) {
			OffsetState = EOffsetState::EOSAiming;
		}
		else {
			OffsetState = EOffsetState::EOSHip;
		}
	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterChar = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterChar == nullptr) return;

	Pitch = ShooterChar->GetBaseAimRotation().Pitch;


	if (Speed > 0 || bIsInAir) {
		//char moving, dont durn in place
		RootYawOffset = 0;
		TIPCharYaw = ShooterChar->GetActorRotation().Yaw;
		RotationCurveLastFrame = 0;
		RotationCurve = 0;
	}

	else {
		TIPCharYawLastFrame = TIPCharYaw;
		TIPCharYaw = ShooterChar->GetActorRotation().Yaw;

		const float TIPyawDelta = TIPCharYaw - TIPCharYawLastFrame;
		//RootYawOffset updated and clamped to [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPyawDelta);


		//1 if turning, 0 if  not
		const float Turning = GetCurveValue(TEXT("Turning"));
		if (Turning > 0) {
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));

			const float deltaRot = RotationCurve - RotationCurveLastFrame;

			//>0 turn left/ <0 turn right
			RootYawOffset > 0 ? RootYawOffset -= deltaRot : RootYawOffset += deltaRot;

			const float ABSRootYawOffset = FMath::Abs(RootYawOffset);
			if (ABSRootYawOffset > 90) {
				const float YawExcess = ABSRootYawOffset - 90;
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{

	if (ShooterChar == nullptr) return;

	CharRotationLastFrame = CharRotation;

	CharRotation = ShooterChar->GetActorRotation();

	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharRotation, CharRotationLastFrame);
	const float Target = (Delta.Yaw)/DeltaTime;
	const float Interp = FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f);

	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	//GEngine->AddOnScreenDebugMessage(2, -1, FColor::Cyan, FString::Printf(TEXT("YawDelta: %f"), YawDelta));
}
