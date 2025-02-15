// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimInstance.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "FirstGame/Weapon/Weapon.h"
#include "FirstGame/CharacterType/CombatState.h"

void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MyCharacter == nullptr) {
		MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
	}

	if (MyCharacter == nullptr) {
		return;
	}

	FVector Velocity = MyCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	//UE_LOG(LogTemp, Display, TEXT("%f"),Speed);

	bIsInAir = MyCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = MyCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped = MyCharacter->IsWeaponEquipped();

	EquippedWeapon = MyCharacter->GetEquippedWeapon();

	TurningInPlace = MyCharacter->GetTurningInPlace();

	bIsCrouch = MyCharacter->bIsCrouched;

	bAiming = MyCharacter->IsAiming();

	bRotateRootBone = MyCharacter->ShouldRotateRootBone();

	bElimmed = MyCharacter->IsElimmed();


	FRotator AimRotation = MyCharacter->GetBaseAimRotation();
	FRotator MovementRoatation = UKismetMathLibrary::MakeRotFromX(MyCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRoatation,AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;
	
	CharacterRotaionLastFrame = CharacterRotaion;
	CharacterRotaion = MyCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotaion, CharacterRotaionLastFrame);
	const float Target = Delta.Yaw;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = MyCharacter->GetAO_Yaw();
	AO_Pitch = MyCharacter->GetAO_Pitch();
	/*UE_LOG(LogTemp, Warning, TEXT("AO_YAW: %f, AO_PITCH: %f"), AO_Yaw, AO_Pitch);*/

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MyCharacter->GetMesh()) {
		if (MyCharacter->IsLocallyControlled()) {
			bLocallyControlled = true;
			FTransform RightHandTransform = MyCharacter->GetMesh()->GetSocketTransform(FName("RightHand"));
			FRotator LookAtRotaion = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), MyCharacter->GetHitTarget());
			LookAtRotaion = LookAtRotaion + FRotator(90.f, 100.f, 0.f);
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotaion, DeltaTime, 20.f);
		}

		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		/*FVector SocketPosition = LeftHandTransform.GetLocation();
		UE_LOG(LogTemp, Warning, TEXT("Socket Position: X=%f, Y=%f, Z=%f"), SocketPosition.X, SocketPosition.Y, SocketPosition.Z);*/
		MyCharacter->GetMesh()->TransformToBoneSpace(FName("LeftHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		//UE_LOG(LogTemp, Warning, TEXT("OutPosition: X=%f, Y=%f, Z=%f"), OutPosition.X, OutPosition.Y, OutPosition.Z);
		//UE_LOG(LogTemp, Warning, TEXT("OutRotation: Pitch=%f, Yaw=%f, Roll=%f"), OutRotation.Pitch, OutRotation.Yaw, OutRotation.Roll);


		
		


		/*FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Muzzle"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::Y));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MyCharacter->GetHitTarget(), FColor::Blue);*/

	}
	FTransform RootTransform = MyCharacter->GetMesh()->GetSocketTransform(FName("Hips"));
	//UE_LOG(LogTemp, Warning, TEXT("Root Bone Rotation: %s"), *RootTransform.GetRotation().Rotator().ToString());

	bUseFABRIK = MyCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffset = MyCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bTransformRightHand = MyCharacter->GetCombatState() != ECombatState::ECS_Reloading;
}
