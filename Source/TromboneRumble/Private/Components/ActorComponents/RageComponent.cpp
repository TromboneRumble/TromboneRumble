// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/RageComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Compilation/MovieSceneCompilerRules.h"
#include "GameFramework/Character.h"
#include "Items/WeaponBase.h"
#include "Utilities/DebugHelper.h"

URageComponent::URageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void URageComponent::BeginPlay()
{
	Super::BeginPlay();
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh) return;

	UMaterialInterface* CurrentMat = Mesh->GetMaterial(2);
	if (!CurrentMat) return;

	DynamicMaterial = Cast<UMaterialInstanceDynamic>(CurrentMat);

	if (!DynamicMaterial)
	{
		DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(2);
	}
}

void URageComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (AActor* Owner = GetOwner())
	{
		if (UEquipmentComponent* EquipComp = Owner->FindComponentByClass<UEquipmentComponent>())
		{
			EquipComp->OnEquipmentChangedDelegate.AddDynamic(this, &ThisClass::HandleEquipmentChanged);
		}
		if (ADefaultTromboneCharacter* DefaultTromboneCharacter = Cast<ADefaultTromboneCharacter>(GetOwner()))
		{
			CharacterData = DefaultTromboneCharacter->GetCharacterDataAsset();
		}
	}
}


void URageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!GetOwner()->HasAuthority() || !isRageActive) return;

	if (bIsBuffActive && CharacterData)
	{
		BuffTimer += DeltaTime;

		float MaxTime = CharacterData->GetMaxBuffTime();
		FillAmount = FMath::Clamp(BuffTimer / MaxTime, 0.0f, 1.0f);
		OnRep_FillAmount();
		for (int32 i = 0; i < CharacterData->SpeedBuffMilestones.Num(); ++i)
		{
			const FRageBuffMileStone& Milestone = CharacterData->SpeedBuffMilestones[i];

			if (BuffTimer >= Milestone.TimeThreshold)
			{
				if (i > LastAppliedMilestoneIndex)
				{
					LastAppliedMilestoneIndex = i;
					UpdateSpeedEffect(Milestone.SpeedMultiplier);
				}
			}
		}
	}
}

void URageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(URageComponent, FillAmount);
}

void URageComponent::HandleEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (!GetOwner()->HasAuthority()) return;
	if (Slot==EEquipmentSlotType::Weapon)
	{
		FillAmount = 0.0f;
		OnRep_FillAmount();
		RemoveRageBuff();

		// 장비를 떨구거나, HeadButtComponent를 장비했을 경우
		if (NewItem == nullptr || (NewItem && DefaultWeaponClass && NewItem->IsA(DefaultWeaponClass)))
		{
			bIsBuffActive = true;
			BuffTimer = 0.0f;
			LastAppliedMilestoneIndex = -1;
		}
		else
		{
			bIsBuffActive = false;
		}
	}
}

void URageComponent::OnRep_FillAmount()
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("RageAmount"), FillAmount);
	}
}

void URageComponent::UpdateSpeedEffect(float Multiplier)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC || !RageBuffClass) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(RageBuffClass, 1.0f, Context);

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(SpeedMultiplierTag, Multiplier);

		if (RageBuffHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(RageBuffHandle);
		}

		RageBuffHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void URageComponent::RemoveRageBuff()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (ASC && RageBuffHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(RageBuffHandle);
		RageBuffHandle.Invalidate();
	}
}

bool URageComponent::IsAllEquipmentRemoved() const
{
	if (UEquipmentComponent* EquipComp = GetOwner()->FindComponentByClass<UEquipmentComponent>())
	{
		// 이 부분은 EquipmentComponent의 실제 구현(슬롯 리스트 등)에 따라 수정이 필요합니다.
		// 모든 슬롯이 비어있으면 true를 반환하도록 작성하세요.
		return true;
	}
	return false;
}

