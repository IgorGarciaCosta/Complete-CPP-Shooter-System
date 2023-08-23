// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
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
			
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterChar = Cast<AShooterCharacter>(TryGetPawnOwner());
}
