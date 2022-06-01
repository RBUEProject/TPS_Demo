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
	TIPCharacterYaw(0.f),
	TIPCharacterYawLastFrame(0.f),
	RootYawOffset(0.f),
	pitch(0),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	YawDelta(0.f),
	RecoilWeight(1.f),
	bTurningInPlace(false)
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
		bCrouching = shooterCharacter->GetCrouching();

		bReloading = shooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
		
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

		if (bReloading)
		{
			OffsetState = EOffsetState::EOS_Reloading;
		}
		else if (bIsInAir)
		{
			OffsetState = EOffsetState::EOS_InAir;
		}
		else if (shooterCharacter->GetAiming())
		{
			OffsetState = EOffsetState::EOS_Aiming;
		}
		else
		{
			OffsetState = EOffsetState::EOS_Hip;
		}
	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::TurnInPlace()
{
	if(shooterCharacter == nullptr) return;

	pitch = shooterCharacter->GetBaseAimRotation().Pitch;


	if (speed > 0 || bIsInAir)
	{
		RootYawOffset = 0.f;

		TIPCharacterYaw = shooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurve = 0.f;
		RotationCurveLastFrame = 0.f;
	}
	else//原地旋转
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = shooterCharacter->GetActorRotation().Yaw;
		const float TIPYawDelta {TIPCharacterYaw - TIPCharacterYawLastFrame };//控制器转的角度

		//根和controller的偏移角度, root yaw offset , updated and clamped to [-180,180] 
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);
		//RootYawOffset > 0 : Turning left , < 0 : Turning right
		//GEngine->AddOnScreenDebugMessage(1,-1,FColor::Cyan,FString::Printf(TEXT("%f"),RootYawOffset));
		const float Turning{ GetCurveValue(TEXT("turning")) };//==1在旋转

		if (Turning > 0)
		{
			bTurningInPlace = true;
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
		else
		{
			bTurningInPlace = false;
		}
	}
	
	//设置RecoilWeight
	if (bTurningInPlace)
	{
		if (bReloading)
		{
			RecoilWeight = 1.f;//上下身混合权重
		}
		else
		{
			RecoilWeight = 0.f;
		}
	}
	else
	{
		if (bCrouching)
		{
			if (bReloading)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.1f;
			}
		}
		else
		{
			if (bAiming || bReloading)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.5f;
			}
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (shooterCharacter == nullptr) return;
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = shooterCharacter->GetActorRotation();

	const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };

	const double Target{ Delta.Yaw / DeltaTime };
	const double Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
	GEngine->AddOnScreenDebugMessage(1,-1,FColor::Cyan,FString::Printf(TEXT("%f"), YawDelta));
}