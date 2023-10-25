// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8 {
	EOSAiming UMETA(DIsplayName = "Aiming"),
	EOSHip UMETA(DIsplayName = "Hip"),
	EOSReloading UMETA(DIsplayName = "Reloading"),
	EOSInAir UMETA(DIsplayName = "InAir"),
};
/**
 *
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()


public:

	UFUNCTION(BlueprintCallable)
		void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

protected:
	void TurnInPlace();

	//calcs for leaning while running
	void Lean(float DeltaTime);


private:

	float RotationCurve;

	float RotationCurveLastFrame;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float Pitch = 0.f;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bReloading = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class AShooterCharacter* ShooterChar;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float Speed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float MovementOffsetYaw = 0.f;


	//get offsetyaw of the frame before we stopped moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float LastMovementOffsetYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bAiming = false;


	//turn in place (only updating when standing still)
	float TIPCharYaw = 0.f;

	float TIPCharYawLastFrame = 0.f;

	FRotator CharRotation;

	FRotator CharRotationLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float RootYawOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EOffsetState OffsetState = EOffsetState::EOSHip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float YawDelta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bCrouching = false;


};
