// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon():
	ThrowWeaponTime(0.7f),
	bFalling(false),
	Ammo(0),
	WeaponType(EWeaponType::EWT_SubmachineGun),
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSection(FName(TEXT("ReloadSMG"))),
	MagazineCapacity(30),
	ClipBoneName(TEXT("smg_clip")),
	SlideDisplacement(0.f),
	SlideDisplacementTime(0.2f),
	bMovingSlide(false),
	MaxSlideDisplacement(4.f),
	MaxRecoilRotation(20.f),
	bAutomatic(true)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		FRotator MeshRotation {0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	UpdateSlideDisplacement();
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation {0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	const FVector MeshForward{ GetItemMesh()->GetForwardVector()};
	const FVector MeshRight{ GetItemMesh()->GetRightVector()};
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);
	float RandomRotation  { 30.f }; //FMath::FRandRange(0.f, 30.f);
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 2'000.f;
	GetItemMesh()->AddImpulse(ImpulseDirection);
	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

	EnableGlowMaterial();
}

void AWeapon::DecrementAmmo()
{
	if (Ammo - 1 <= 0)
	{
		Ammo = 0;
	}
	else
	{
		--Ammo;
	}
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("试图以超过弹匣容量的装填"));
	Ammo += Amount;
}

bool AWeapon::ClipIsFull()
{
	return Ammo >= MagazineCapacity;
}

void AWeapon::StartSlideTimer()
{
	bMovingSlide = true;
	GetWorldTimerManager().SetTimer(SlideTimer, this, &AWeapon::FinishMovingSlide, SlideDisplacementTime);
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_PickUp);
	StartPulseTimer();
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (BoneToHide != FName(""))
	{
		GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);
	}
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	const FString WeaponDataPath { TEXT("DataTable'/Game/_Game/DataTable/WeaponDataTable.WeaponDataTable'")};
	UDataTable* WeaponDataTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponDataPath));
	FWeaponDataTable* WeaponDataRow = nullptr;
	switch (WeaponType)
	{
	case EWeaponType::EWT_SubmachineGun:
		WeaponDataRow = WeaponDataTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"),TEXT(""));
		break;

	case EWeaponType::EWT_AssaultRifle:
		WeaponDataRow = WeaponDataTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
		break;
	case EWeaponType::EWT_Pistol:
		WeaponDataRow = WeaponDataTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
		break;
	}
	if (WeaponDataRow)
	{
		AmmoType = WeaponDataRow->AmmoType;
		Ammo = WeaponDataRow->WeaponAmmo;
		MagazineCapacity = WeaponDataRow->MagazineCapacity;
		SetPickUpSound(WeaponDataRow->PickUpSound);
		SetEquipSound(WeaponDataRow->EquipSound);
		GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
		SetItemName(WeaponDataRow->ItemName);
		SetIconItem(WeaponDataRow->InventoryIcon);
		SetAmmoIcon(WeaponDataRow->AmmoIcon);

		SetMaterialInstance(WeaponDataRow->MaterialInstance);
		PreviousMaterialIndex = GetMaterialIndex();
		GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
		SetMaterialIndex(WeaponDataRow->MaterialIndex);
		SetClipBoneName(WeaponDataRow->ClipBoneName);
		SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
		GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);
		CrosshairMiddle = WeaponDataRow->CrosshairMiddle;
		CrosshairLeft = WeaponDataRow->CrosshairLeft;
		CrosshairRight = WeaponDataRow->CrosshairRight;
		CrosshairTop = WeaponDataRow->CrosshairTop;
		CrosshairBottom = WeaponDataRow->CrosshairBottom;
		AutoFireRate = WeaponDataRow->AutoFireRate;
		MuzzleFlash = WeaponDataRow->MuzzleFlash;
		FireSound = WeaponDataRow->FireSound;
		BoneToHide = WeaponDataRow->BoneToHide;
		bAutomatic = WeaponDataRow->bAutomatic;
	}
	if (GetMaterialInstance())
	{
		SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
		GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
		GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
		EnableGlowMaterial();
	}
}

void AWeapon::FinishMovingSlide()
{
	bMovingSlide = false;
}

void AWeapon::UpdateSlideDisplacement()
{
	if (SlideDisplacementCurve && bMovingSlide)
	{
		const float ElapsedTime{ GetWorldTimerManager().GetTimerElapsed(SlideTimer) };
		const float CurveValue{ SlideDisplacementCurve->GetFloatValue(ElapsedTime) };
		SlideDisplacement = CurveValue * MaxSlideDisplacement * -1.f;
		RecoilRotation = CurveValue * MaxRecoilRotation * -1.f;
	}
}
