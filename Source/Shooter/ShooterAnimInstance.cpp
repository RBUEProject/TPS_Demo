// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
	speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	bAiming(false),
	CharacterYaw(0.f),
	CharacterYawLastFrame(0.f),
	RootYawOffset(0.f)
{

}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	shooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (shooterCharacter == nullptr)
	{
		shooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (shooterCharacter)
	{
		FVector Velocity {shooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		speed = Velocity.Size();

		bIsInAir = shooterCharacter->GetCharacterMovement()->IsFalling();

		if (shooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}
		//瞄准方向
		FRotator AimRotation = shooterCharacter->GetBaseAimRotation();
		//移动方向
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(shooterCharacter->GetVelocity());
	
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		if(shooterCharacter->GetVelocity().Size() > 0.f)
			LastMovementOffsetYaw = MovementOffsetYaw;
		//FString testMessage = FString::Printf(TEXT("Base Aim Rotation : %f ,Movement Rotation : %f , MovementOffsetYaw : %f"),  AimRotation.Yaw, MovementRotation.Yaw, MovementOffsetYaw);
		//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Blue, testMessage);

		bAiming = shooterCharacter->GetAiming();
	}
	TurnInPlace();
}

void UShooterAnimInstance::TurnInPlace()
{
	if(shooterCharacter == nullptr) return;

	if (speed > 0)
	{
		RootYawOffset = 0.f;

		CharacterYaw = shooterCharacter->GetActorRotation().Yaw;
		CharacterYawLastFrame = CharacterYaw;
		RotationCurve = 0.f;
		RotationCurveLastFrame = 0.f;
	}
	else
	{
		CharacterYawLastFrame = CharacterYaw;
		CharacterYaw = shooterCharacter->GetActorRotation().Yaw;
		const float YawDelta {CharacterYaw - CharacterYawLastFrame };//控制器转的角度

		//根和controller的偏移角度, root yaw offset , updated and clamped to [-180,180] 
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);// > 0 : Turning left , < 0 : Turning right
		GEngine->AddOnScreenDebugMessage(1,-1,FColor::Cyan,FString::Printf(TEXT("%f"),RootYawOffset));
		const float Turning{ GetCurveValue(TEXT("turning")) };//==1在旋转

		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation ;

			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset)};
			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess {ABSRootYawOffset - 90.f};
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}


	}
}
