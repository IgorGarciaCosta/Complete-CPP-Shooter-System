// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

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


private:

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterChar;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
	
};
