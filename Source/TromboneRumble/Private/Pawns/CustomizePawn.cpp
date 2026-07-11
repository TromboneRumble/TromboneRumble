#include "Pawns/CustomizePawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "Utilities/Defines.h"

ACustomizePawn::ACustomizePawn()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionObjectType(ECC_Pawn);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	RootComponent = CapsuleComponent;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));

	CustomizationComp = CreateDefaultSubobject<UCustomizationComponent>(TEXT("CustomizationComponent"));
	// CustomizeMap은 스킨컬러 틴트 없이 파츠를 기본 머티리얼 그대로 표시
	CustomizationComp->SetApplyPartsSkinColor(false);

	AActor::SetReplicateMovement(false);
}

void ACustomizePawn::BeginPlay()
{
	// 스킨컬러 틴트를 적용하지 않음 — face 표정 교체 복원용 기본 머티리얼만 캐싱
	const int32 FaceIndex = SkeletalMeshComponent ? SkeletalMeshComponent->GetMaterialIndex(TromboneMaterial::FaceSlotName) : INDEX_NONE;
	if (FaceIndex != INDEX_NONE)
	{
		OriginalFaceMaterial = SkeletalMeshComponent->GetMaterial(FaceIndex);
	}

	if (CustomizationComp)
	{
		FCustomizationSaveData SaveData;
		if (USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
			SaveData = SMS->LoadCustomization();
		CustomizationComp->LoadFromSaveData(SaveData);
	}

	Super::BeginPlay();
}

void ACustomizePawn::ApplyFaceMaterial(UMaterialInterface* Material)
{
	// 스킨컬러 틴트 없이 표정 머티리얼만 교체 (기본 머티리얼 그대로)
	UMaterialInterface* Target = Material ? Material : OriginalFaceMaterial.Get();
	if (!Target || !SkeletalMeshComponent) return;

	const int32 FaceIndex = SkeletalMeshComponent->GetMaterialIndex(TromboneMaterial::FaceSlotName);
	if (FaceIndex == INDEX_NONE) return;

	SkeletalMeshComponent->SetMaterial(FaceIndex, Target);
}
