// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/CustomizationComponent.h"
#include "Actors/ResultScene/PodiumActor.h"
#include "Animation/AnimInstance.h"
#include "Characters/TromboneCharacterBase.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Framework/DefaultPlayerState.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Pawns/CustomizePawn.h"
#include "Pawns/MatchPawn.h"

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

	// owner가 LoadFromSaveData를 한 번도 호출하지 않은 경우(예: 빙의되지 않는 TutorialDummy)
	// 최소한 Order 0 기본 파츠를 부착해 머리만 보이는 현상을 방지한다.
	const bool bNothingApplied =
		CurrentAntennaKey == NAME_None &&
		CurrentFaceKey    == NAME_None &&
		CurrentCostumeKey == NAME_None;
	if (bNothingApplied)
	{
		LoadFromSaveData(FCustomizationSaveData());
	}
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
	ApplyFollowerPart(Row, AntennaComp, AntennaSkinMID, TEXT("AntennaComp"));
}

void UCustomizationComponent::ApplyFace(const FCustomizationPartRow* Row)
{
	UMaterialInterface* Mat = (!Row || Row->AssetPath.IsNull())
		? nullptr
		: Cast<UMaterialInterface>(Row->AssetPath.TryLoad());

	if (ADefaultTromboneCharacter* Char = Cast<ADefaultTromboneCharacter>(GetOwner()))
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
	else if (APodiumActor* Podium = Cast<APodiumActor>(GetOwner()))
	{
		Podium->ApplyFaceMaterial(Mat);
	}
}

void UCustomizationComponent::ApplyCostume(const FCustomizationPartRow* Row)
{
	ApplyFollowerPart(Row, CostumeComp, CostumeSkinMID, TEXT("CostumeComp"));
}

void UCustomizationComponent::ApplyFollowerPart(const FCustomizationPartRow* Row,
	TObjectPtr<USkeletalMeshComponent>& Comp,
	TObjectPtr<UMaterialInstanceDynamic>& SkinMID,
	const TCHAR* CompName)
{
	// 기존 follower 컴포넌트와 캐시된 MID 정리
	if (Comp)
	{
		if (AActor* OldOwner = GetOwner())
		{
			OldOwner->RemoveInstanceComponent(Comp);
		}
		Comp->DestroyComponent();
		Comp = nullptr;
	}
	SkinMID = nullptr;

	if (!Row || Row->AssetPath.IsNull()) return;

	USkeletalMesh* Mesh = Cast<USkeletalMesh>(Row->AssetPath.TryLoad());
	if (!Mesh) return;

	USkeletalMeshComponent* Leader = ResolveLeaderMesh();
	if (!Leader) return;

	AActor* Owner = GetOwner();
	Comp = NewObject<USkeletalMeshComponent>(Owner, CompName);
	Comp->SetSkeletalMeshAsset(Mesh);
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Comp->SetReceivesDecals(false);
	Comp->SetupAttachment(Leader);

	// 자체 본을 가진 파츠(모자 등)는 LeaderPose로 움직일 수 없다. follower는 애님도 물리도 평가하지 않기 때문.
	// AnimBP가 지정된 행만 자기 애님을 돌리고, 비어 있으면 기존 LeaderPose 경로 그대로.
	// 등록 전에 지정해야 OnRegister의 InitAnim에서 한 번만 초기화된다(InitAnim은 IsRegistered()를 요구).
	UClass* AnimClass = Row->PartAnimClass.IsNull() ? nullptr : Row->PartAnimClass.LoadSynchronous();
	if (AnimClass)
	{
		Comp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		Comp->SetAnimInstanceClass(AnimClass);
		Comp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	}

	Comp->RegisterComponent();
	Owner->AddInstanceComponent(Comp);

	if (AnimClass)
	{
		// CopyPoseFromMesh가 leader의 '이번 프레임' 포즈를 읽도록 틱 순서를 고정. 없으면 한 프레임 지연된다.
		Comp->AddTickPrerequisiteComponent(Leader);
	}
	else
	{
		// leader(머리)의 본 포즈(애니메이션·레그돌 결과)를 그대로 복사 → 자체 PhysicsAsset/AnimBP 불필요
		Comp->SetLeaderPoseComponent(Leader);
	}

	// skin 슬롯이 있으면 MID 확보 후 현재 피부색 적용 (머리/몸통/안테나 색 동기)
	// bApplyPartsSkinColor=false면 틴트 없이 기본 머티리얼 그대로 (CustomizeMap)
	if (bApplyPartsSkinColor)
	{
		SkinMID = EnsureSlotMID(Comp, TromboneMaterial::SkinSlotName);
		if (SkinMID)
		{
			SkinMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, GetOwnerSkinColor());
		}
	}
	
}

void UCustomizationComponent::ApplyPartsSkinColor(const FLinearColor& InColor) const
{
	if (!bApplyPartsSkinColor) return;

	if (CostumeSkinMID)
	{
		CostumeSkinMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, InColor);
	}
	if (AntennaSkinMID)
	{
		AntennaSkinMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, InColor);
	}
}

USkeletalMeshComponent* UCustomizationComponent::ResolveLeaderMesh() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	// follower도 USkeletalMeshComponent이므로 FindComponentByClass는 follower를 오인할 수 있음 → 타입별로 명시
	if (const ACharacter* Char = Cast<ACharacter>(Owner))
	{
		return Char->GetMesh();
	}
	if (const AMatchPawn* MatchPawn = Cast<AMatchPawn>(Owner))
	{
		return MatchPawn->GetMeshComponent();
	}
	if (const ACustomizePawn* CustomPawn = Cast<ACustomizePawn>(Owner))
	{
		return CustomPawn->GetMeshComponent();
	}
	if (const APodiumActor* Podium = Cast<APodiumActor>(Owner))
	{
		return Podium->GetMeshComponent();
	}
	return Owner->FindComponentByClass<USkeletalMeshComponent>();
}

FLinearColor UCustomizationComponent::GetOwnerSkinColor() const
{
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (const ADefaultPlayerState* DPS = OwnerPawn->GetPlayerState<ADefaultPlayerState>())
		{
			return DPS->GetSkinColor();
		}
	}
	// PodiumActor는 PlayerState가 없으므로 외부에서 주입된 피부색을 사용
	if (const APodiumActor* Podium = Cast<APodiumActor>(GetOwner()))
	{
		return Podium->GetSkinColor();
	}
	return FLinearColor::Black;
}

UMaterialInstanceDynamic* UCustomizationComponent::EnsureSlotMID(USkeletalMeshComponent* Mesh, FName SlotName)
{
	if (!Mesh) return nullptr;

	const int32 Index = Mesh->GetMaterialIndex(SlotName);
	if (Index == INDEX_NONE) return nullptr;

	if (UMaterialInstanceDynamic* Existing = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(Index)))
	{
		return Existing;
	}
	return Mesh->CreateAndSetMaterialInstanceDynamic(Index);
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
