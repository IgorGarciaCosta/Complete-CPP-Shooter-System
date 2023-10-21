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
	}
	TurnInPlace();
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterChar = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterChar == nullptr) return;

	Pitch = ShooterChar->GetBaseAimRotation().Pitch;


	if (Speed > 0) {
		//char moving, dont durn in place
		RootYawOffset = 0;
		CharYaw = ShooterChar->GetActorRotation().Yaw;
		RotationCurveLastFrame = 0;
		RotationCurve = 0;
	}

	else {
		CharYawLastFrame = CharYaw;
		CharYaw = ShooterChar->GetActorRotation().Yaw;

		const float yawDelta = CharYaw - CharYawLastFrame;
		//RootYawOffset updated and clamped to [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - yawDelta);


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
