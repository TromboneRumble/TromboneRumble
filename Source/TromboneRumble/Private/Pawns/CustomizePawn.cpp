#include "Pawns/CustomizePawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "Subsystems/SaveManagerSubsystem.h"

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

	AActor::SetReplicateMovement(false);
}

void ACustomizePawn::BeginPlay()
{
	if (UMaterialInterface* CurrentSkinMat = SkeletalMeshComponent->GetMaterial(SkinMaterialIndex))
	{
		SkinMID = Cast<UMaterialInstanceDynamic>(CurrentSkinMat);
		if (!SkinMID)
			SkinMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(SkinMaterialIndex);
	}
	if (UMaterialInterface* CurrentFaceMat = SkeletalMeshComponent->GetMaterial(FaceMaterialIndex))
	{
		OriginalFaceMaterial = CurrentFaceMat;
		FaceMID = Cast<UMaterialInstanceDynamic>(CurrentFaceMat);
		if (!FaceMID)
			FaceMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(FaceMaterialIndex);
	}

	if (CustomizationComp)
	{
		FCustomizationSaveData SaveData;
		if (USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
			SaveData = SMS->LoadCustomization();
		CustomizationComp->LoadFromSaveData(SaveData);
	}

	Super::BeginPlay();

	UpdateSkinFromPlayerState();
}

void ACustomizePawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	UpdateSkinFromPlayerState();
}

void ACustomizePawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	UpdateSkinFromPlayerState();
}

void ACustomizePawn::ApplyFaceMaterial(UMaterialInterface* Material)
{
	UMaterialInterface* Target = Material ? Material : OriginalFaceMaterial.Get();
	if (!Target || !SkeletalMeshComponent) return;

	SkeletalMeshComponent->SetMaterial(FaceMaterialIndex, Target);
	FaceMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(FaceMaterialIndex);
	if (FaceMID)
	{
		if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
			FaceMID->SetVectorParameterValue(TEXT("BaseColor"), DPS->GetSkinColor());
	}
}

void ACustomizePawn::UpdateSkinFromPlayerState() const
{
	if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
	{
		const FLinearColor SkinColor = DPS->GetSkinColor();
		if (SkinMID)
			SkinMID->SetVectorParameterValue(TEXT("BaseColor"), SkinColor);
		if (FaceMID)
			FaceMID->SetVectorParameterValue(TEXT("BaseColor"), SkinColor);
	}
}
