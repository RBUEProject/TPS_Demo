// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
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
	}
}
