// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/CustomizationComponent.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Pawns/CustomizePawn.h"
#include "Pawns/MatchPawn.h"

const FName UCustomizationComponent::AntennaSocketName = TEXT("socket_antenna");
const FName UCustomizationComponent::CostumeSocketName = TEXT("socket_costume");

UCustomizationComponent::UCustomizationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UCustomizationComponent::InitializeComponent()
{
	Super::InitializeComponent();
	CachedDataTable = CustomizationDataTable.LoadSynchronous();
}

void UCustomizationComponent::BeginPlay()
{
	if (!CachedDataTable)
		CachedDataTable = CustomizationDataTable.LoadSynchronous();
	Super::BeginPlay();
}

// ── 공개 인터페이스 ──────────────────────────────────────────────────────────────

void UCustomizationComponent::LoadFromSaveData(const FCustomizationSaveData& Data)
{
	if (!CachedDataTable)
		CachedDataTable = CustomizationDataTable.LoadSynchronous();

	// 저장된 Key가 유효하면 그대로 사용, NAME_None이거나 DataTable에 없으면 Order 0 기본값으로 폴백
	auto Resolve = [this](ECustomizationSlotType Slot, FName SavedKey) -> FName
	{
		if (SavedKey != NAME_None && FindRowByKey(SavedKey))
			return SavedKey;
		return FindDefaultKeyForSlot(Slot);
	};

	ApplySlot(ECustomizationSlotType::Antenna, Resolve(ECustomizationSlotType::Antenna, Data.AntennaKey));
	ApplySlot(ECustomizationSlotType::Face,    Resolve(ECustomizationSlotType::Face,    Data.FaceKey));
	ApplySlot(ECustomizationSlotType::Costume, Resolve(ECustomizationSlotType::Costume, Data.CostumeKey));
}

FCustomizationSaveData UCustomizationComponent::GetCurrentSaveData() const
{
	FCustomizationSaveData Data;
	Data.AntennaKey = CurrentAntennaKey;
	Data.FaceKey    = CurrentFaceKey;
	Data.CostumeKey = CurrentCostumeKey;
	return Data;
}

void UCustomizationComponent::StepPart(ECustomizationSlotType Slot, int32 Direction)
{
	if (!CachedDataTable)
		CachedDataTable = CustomizationDataTable.LoadSynchronous();

	const TArray<FName> Keys = GetSortedKeysForSlot(Slot);
	if (Keys.IsEmpty()) return;

	FName CurrentKey = NAME_None;
	switch (Slot)
	{
	case ECustomizationSlotType::Antenna: CurrentKey = CurrentAntennaKey; break;
	case ECustomizationSlotType::Face:    CurrentKey = CurrentFaceKey;    break;
	case ECustomizationSlotType::Costume: CurrentKey = CurrentCostumeKey; break;
	default: return;
	}

	int32 Idx = Keys.IndexOfByKey(CurrentKey);
	if (Idx == INDEX_NONE) Idx = 0;

	// 음수 방향도 올바르게 처리
	const int32 NewIdx = ((Idx + Direction) % Keys.Num() + Keys.Num()) % Keys.Num();
	ApplySlot(Slot, Keys[NewIdx]);
}

void UCustomizationComponent::RandomizeAll()
{
	if (!CachedDataTable)
		CachedDataTable = CustomizationDataTable.LoadSynchronous();

	auto GetCurrentKey = [this](ECustomizationSlotType Slot) -> FName
	{
		switch (Slot)
		{
		case ECustomizationSlotType::Antenna: return CurrentAntennaKey;
		case ECustomizationSlotType::Face:    return CurrentFaceKey;
		case ECustomizationSlotType::Costume: return CurrentCostumeKey;
		default: return NAME_None;
		}
	};

	for (ECustomizationSlotType Slot : { ECustomizationSlotType::Antenna, ECustomizationSlotType::Face, ECustomizationSlotType::Costume })
	{
		TArray<FName> Keys = GetSortedKeysForSlot(Slot);
		if (Keys.IsEmpty()) continue;
		if (Keys.Num() == 1) { ApplySlot(Slot, Keys[0]); continue; }

		const FName CurrentKey = GetCurrentKey(Slot);
		const int32 CurrentIdx = Keys.IndexOfByKey(CurrentKey);

		if (CurrentIdx == INDEX_NONE)
		{
			ApplySlot(Slot, Keys[FMath::RandRange(0, Keys.Num() - 1)]);
			continue;
		}
		const int32 Offset = FMath::RandRange(1, Keys.Num() - 1);
		ApplySlot(Slot, Keys[(CurrentIdx + Offset) % Keys.Num()]);
	}
}

void UCustomizationComponent::SetPartByKey(ECustomizationSlotType Slot, FName Key)
{
	if (!CachedDataTable)
		CachedDataTable = CustomizationDataTable.LoadSynchronous();
	ApplySlot(Slot, Key);
}

bool UCustomizationComponent::IsDirtyFrom(const FCustomizationSaveData& Original) const
{
	// Original의 NAME_None도 실제 default key로 정규화해서 비교 (잘못된 dirty 판정 방지)
	auto Resolve = [this](ECustomizationSlotType Slot, FName SavedKey) -> FName
	{
		if (SavedKey != NAME_None && FindRowByKey(SavedKey))
			return SavedKey;
		return FindDefaultKeyForSlot(Slot);
	};

	return CurrentAntennaKey != Resolve(ECustomizationSlotType::Antenna, Original.AntennaKey)
		|| CurrentFaceKey    != Resolve(ECustomizationSlotType::Face,    Original.FaceKey)
		|| CurrentCostumeKey != Resolve(ECustomizationSlotType::Costume, Original.CostumeKey);
}

// ── 내부 적용 로직 ─────────────────────────────────────────────────────────────

void UCustomizationComponent::ApplySlot(ECustomizationSlotType Slot, FName Key)
{
	const FCustomizationPartRow* Row = FindRowByKey(Key);

	switch (Slot)
	{
	case ECustomizationSlotType::Antenna:
		CurrentAntennaKey = Key;
		ApplyAntenna(Row);
		break;
	case ECustomizationSlotType::Face:
		CurrentFaceKey = Key;
		ApplyFace(Row);
		break;
	case ECustomizationSlotType::Costume:
		CurrentCostumeKey = Key;
		ApplyCostume(Row);
		break;
	default:
		break;
	}
}

void UCustomizationComponent::ApplyAntenna(const FCustomizationPartRow* Row)
{
	if (AntennaComp)
	{
		AntennaComp->DestroyComponent();
		AntennaComp = nullptr;
	}

	if (!Row || Row->AssetPath.IsNull()) return;

	UStaticMesh* Mesh = Cast<UStaticMesh>(Row->AssetPath.TryLoad());
	if (!Mesh) return;

	AActor* Owner = GetOwner();
	// ACharacter::GetMesh()와 AMatchPawn::SkeletalMeshComponent 모두 처리
	USkeletalMeshComponent* SkelMesh = Owner ? Owner->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	if (!SkelMesh) return;

	AntennaComp = NewObject<UStaticMeshComponent>(Owner, TEXT("AntennaComp"));
	AntennaComp->SetStaticMesh(Mesh);
	AntennaComp->SetupAttachment(SkelMesh, AntennaSocketName);
	AntennaComp->RegisterComponent();

	// X-Ray 스텐실은 TromboneCharacterBase 로컬 캐릭터에만 적용
	if (ATromboneCharacterBase* TromboneChar = Cast<ATromboneCharacterBase>(Owner))
	{
		if (TromboneChar->IsLocallyControlled())
		{
			ATromboneCharacterBase::ApplyOccludedStencil(AntennaComp);
		}
	}
}

void UCustomizationComponent::ApplyFace(const FCustomizationPartRow* Row)
{
	UMaterialInterface* Mat = (!Row || Row->AssetPath.IsNull())
		? nullptr
		: Cast<UMaterialInterface>(Row->AssetPath.TryLoad());

	if (ATromboneCharacterBase* Char = Cast<ATromboneCharacterBase>(GetOwner()))
	{
		Char->ApplyFaceMaterial(Mat);
	}
	else if (AMatchPawn* Pawn = Cast<AMatchPawn>(GetOwner()))
	{
		Pawn->ApplyFaceMaterial(Mat);
	}
	else if (ACustomizePawn* CustomPawn = Cast<ACustomizePawn>(GetOwner()))
	{
		CustomPawn->ApplyFaceMaterial(Mat);
	}
}

void UCustomizationComponent::ApplyCostume(const FCustomizationPartRow* Row)
{
	if (CostumeComp)
	{
		CostumeComp->DestroyComponent();
		CostumeComp = nullptr;
	}

	if (!Row || Row->AssetPath.IsNull()) return;

	UStaticMesh* Mesh = Cast<UStaticMesh>(Row->AssetPath.TryLoad());
	if (!Mesh) return;

	AActor* Owner = GetOwner();
	USkeletalMeshComponent* SkelMesh = Owner ? Owner->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	if (!SkelMesh) return;

	CostumeComp = NewObject<UStaticMeshComponent>(Owner, TEXT("CostumeComp"));
	CostumeComp->SetStaticMesh(Mesh);
	CostumeComp->SetupAttachment(SkelMesh, CostumeSocketName);
	CostumeComp->RegisterComponent();

	if (ATromboneCharacterBase* TromboneChar = Cast<ATromboneCharacterBase>(Owner))
	{
		if (TromboneChar->IsLocallyControlled())
		{
			ATromboneCharacterBase::ApplyOccludedStencil(CostumeComp);
		}
	}
}

// ── DataTable 조회 헬퍼 ───────────────────────────────────────────────────────

TArray<FName> UCustomizationComponent::GetSortedKeysForSlot(ECustomizationSlotType Slot) const
{
	if (!CachedDataTable) return {};

	TArray<TPair<int32, FName>> Entries;
	CachedDataTable->ForeachRow<FCustomizationPartRow>(
		TEXT("GetSortedKeysForSlot"),
		[&](const FName& Key, const FCustomizationPartRow& Row)
		{
			if (Row.SlotType == Slot)
				Entries.Add({ Row.AssetOrder, Key });
		}
	);

	Entries.Sort([](const TPair<int32, FName>& A, const TPair<int32, FName>& B)
	{
		return A.Key < B.Key;
	});

	TArray<FName> Keys;
	Keys.Reserve(Entries.Num());
	for (const auto& Entry : Entries)
		Keys.Add(Entry.Value);

	return Keys;
}

const FCustomizationPartRow* UCustomizationComponent::FindRowByKey(FName Key) const
{
	if (!CachedDataTable || Key == NAME_None) return nullptr;
	return CachedDataTable->FindRow<FCustomizationPartRow>(Key, TEXT("FindRowByKey"), false);
}

FName UCustomizationComponent::FindDefaultKeyForSlot(ECustomizationSlotType Slot) const
{
	if (!CachedDataTable) return NAME_None;

	FName DefaultKey = NAME_None;
	CachedDataTable->ForeachRow<FCustomizationPartRow>(
		TEXT("FindDefaultKey"),
		[&](const FName& Key, const FCustomizationPartRow& Row)
		{
			if (Row.SlotType == Slot && Row.AssetOrder == 0)
				DefaultKey = Key;
		}
	);
	return DefaultKey;
}
