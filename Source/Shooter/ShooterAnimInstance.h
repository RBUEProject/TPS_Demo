// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 
 */
 UENUM(BlueprintType)
enum class EOffsetState : uint8 
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "MAX")
};


UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UShooterAnimInstance();
	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

protected:
	
	void TurnInPlace();

	void Lean(float DeltaTime);//�ƶ�ʱ��б

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* shooterCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))

	bool bIsInAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	//yawƫ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;
	//������¼ͣ����֮ǰ�ķ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	float TIPCharacterYaw;//idleʱ��yaw

	float TIPCharacterYawLastFrame;

	FRotator CharacterRotation;

	FRotator CharacterRotationLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;


	float RotationCurve;

	float RotationCurveLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float pitch;//ǹ�ڸ���׼�� aimoffset

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	double YawDelta;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crouching", meta = (AllowPrivateAccess = "true"))
	bool bCrouching;//�¶�Ϊtrue

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bTurningInPlace;
};
