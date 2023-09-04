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
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterChar = Cast<AShooterCharacter>(TryGetPawnOwner());
}
