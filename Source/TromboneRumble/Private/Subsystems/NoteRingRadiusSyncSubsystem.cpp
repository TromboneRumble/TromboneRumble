// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/NoteRingRadiusSyncSubsystem.h"

#if WITH_EDITOR
#include "Actors/Rhythm/NoteVisualizer.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/StaticMeshComponents/RingHitBoxComponent.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "MaterialEditor/DEditorScalarParameterValue.h"
#include "MaterialEditor/MaterialEditorInstanceConstant.h"
#include "Materials/MaterialInstanceConstant.h"
#include "UObject/UObjectIterator.h"

namespace
{
	/** 계산 근거가 되는 UPROPERTY 이름. 리플렉션으로 찾으므로 Initialize에서 존재를 확인한다 */
	const FName PropName_AnchorInnerRadius(TEXT("EndInnerRadius"));
	const FName PropName_AnchorOuterRadius(TEXT("EndOuterRadius"));

	/** 이보다 차이가 작으면 이미 맞은 것으로 보고 쓰지 않는다. 매 편집마다 dirty가 되는 것을 막는다 */
	constexpr float WriteEpsilon = 1.e-6f;

	/**
	 * 열려 있는 MI 에디터 창의 값도 같이 고친다.
	 * 창은 프록시(UMaterialEditorInstanceConstant)를 편집하고 그 내용을 MI에 통째로 덮어쓰므로
	 * (PreviewMaterial.cpp:899-923), 프록시를 안 고치면 다음 편집 때 우리 값이 지워진다
	 */
	void PushToOpenEditor(const UMaterialInstanceConstant* NoteMI, float EndOuter, float EndInner)
	{
		for (TObjectIterator<UMaterialEditorInstanceConstant> It; It; ++It)
		{
			UMaterialEditorInstanceConstant* Proxy = *It;
			if (!Proxy || Proxy->SourceInstance != NoteMI)
			{
				continue;
			}

			for (FEditorParameterGroup& Group : Proxy->ParameterGroups)
			{
				for (UDEditorParameterValue* Param : Group.Parameters)
				{
					UDEditorScalarParameterValue* Scalar = Cast<UDEditorScalarParameterValue>(Param);
					if (!Scalar)
					{
						continue;
					}

					if (Scalar->ParameterInfo.Name == ANoteVisualizer::ParamName_EndOuterRadius)
					{
						Scalar->ParameterValue = EndOuter;
						Scalar->bOverride = true;
					}
					else if (Scalar->ParameterInfo.Name == ANoteVisualizer::ParamName_EndInnerRadius)
					{
						Scalar->ParameterValue = EndInner;
						Scalar->bOverride = true;
					}
				}
			}
		}
	}
}
#endif

bool UNoteRingRadiusSyncSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if WITH_EDITOR
	// 에디터에서 사람이 MI를 고칠 때만 필요하다. 쿠커/커맨드릿에는 편집 이벤트가 없다
	return GIsEditor && !IsRunningCommandlet();
#else
	return false;
#endif
}

void UNoteRingRadiusSyncSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	PropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(
		this, &UNoteRingRadiusSyncSubsystem::HandleObjectPropertyChanged);

	// 이름이 바뀌면 조용히 멈추므로 시작할 때 한 번 확인한다
	for (const FName& PropName : { PropName_AnchorInnerRadius, PropName_AnchorOuterRadius })
	{
		if (!FindFProperty<FProperty>(URingHitBoxComponent::StaticClass(), PropName))
		{
			UE_LOG(LogTemp, Error, TEXT("[NoteRingSync] URingHitBoxComponent에 %s가 없다. 이름이 바뀌었으면 여기도 고칠 것"),
				*PropName.ToString());
		}
	}
#endif
}

void UNoteRingRadiusSyncSubsystem::Deinitialize()
{
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PropertyChangedHandle);
	PropertyChangedHandle.Reset();
#endif

	Super::Deinitialize();
}

#if WITH_EDITOR
void UNoteRingRadiusSyncSubsystem::HandleObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	if (!Object || bSyncing)
	{
		return;
	}

	// 노트 MI를 직접 고친 경우. 슬라이더를 끄는 중에도 바로 반영해준다
	if (UMaterialInstanceConstant* NoteMI = Cast<UMaterialInstanceConstant>(Object))
	{
		if (IsNoteRingInstance(NoteMI))
		{
			bSyncing = true;
			SyncOne(NoteMI);
			bSyncing = false;
		}
		return;
	}

	// 계산 근거를 고친 경우. 모든 노트 MI가 한꺼번에 낡는다
	// 에셋을 로드하므로 슬라이더를 놓은 뒤에만 돈다
	if (Event.ChangeType == EPropertyChangeType::Interactive)
	{
		return;
	}

	const FName ChangedName = Event.GetPropertyName();
	const bool bAnchorChanged = Object->IsA<URingHitBoxComponent>()
		&& (ChangedName == PropName_AnchorInnerRadius || ChangedName == PropName_AnchorOuterRadius);

	if (bAnchorChanged)
	{
		bSyncing = true;
		SyncAll();
		bSyncing = false;
	}
}

bool UNoteRingRadiusSyncSubsystem::IsNoteRingInstance(const UMaterialInstanceConstant* MI) const
{
	const UTromboneConfig* Config = UTromboneConfig::Get();
	if (!MI || !Config)
	{
		return false;
	}

	const FSoftObjectPath BasePath = Config->NoteRingBaseMaterial.ToSoftObjectPath();
	if (BasePath.IsNull())
	{
		return false;
	}

	// 경로로 비교해서 베이스 머티리얼을 일부러 로드하지 않는다
	for (const UMaterialInterface* Current = MI->Parent; Current; )
	{
		if (FSoftObjectPath(Current) == BasePath)
		{
			return true;
		}
		const UMaterialInstance* AsInstance = Cast<UMaterialInstance>(Current);
		Current = AsInstance ? AsInstance->Parent.Get() : nullptr;
	}
	return false;
}

bool UNoteRingRadiusSyncSubsystem::ResolveRule(float& OutAnchorInner, float& OutAnchorOuter) const
{
	const UTromboneConfig* Config = UTromboneConfig::Get();
	if (!Config)
	{
		return false;
	}

	const UClass* CharacterClass = Config->NoteRingAnchorCharacterClass.LoadSynchronous();
	if (!CharacterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[NoteRingSync] Trombone Config의 Editor|NoteRing 항목이 비어 있어 End 반경을 계산할 수 없다"));
		return false;
	}

	// 설정에 엉뚱한 클래스가 들어와도 터지지 않게 캐스팅으로 확인한다
	const ADefaultTromboneCharacter* CharacterCDO = Cast<ADefaultTromboneCharacter>(CharacterClass->GetDefaultObject());
	if (!CharacterCDO)
	{
		UE_LOG(LogTemp, Warning, TEXT("[NoteRingSync] Editor|NoteRing에 지정된 클래스가 기대한 타입이 아니다"));
		return false;
	}

	// 히트박스는 네이티브 CreateDefaultSubobject라 CDO에 그대로 붙어 있다
	const URingHitBoxComponent* HitBox = CharacterCDO->FindComponentByClass<URingHitBoxComponent>();
	if (!HitBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("[NoteRingSync] %s에 RingHitBoxComponent가 없다"), *CharacterClass->GetName());
		return false;
	}

	OutAnchorInner = HitBox->GetEndInnerRadius();
	OutAnchorOuter = HitBox->GetEndOuterRadius();
	return true;
}

bool UNoteRingRadiusSyncSubsystem::SyncOne(UMaterialInstanceConstant* NoteMI) const
{
	float AnchorInner = 0.f;
	float AnchorOuter = 0.f;
	if (!NoteMI || !ResolveRule(AnchorInner, AnchorOuter))
	{
		return false;
	}

	float StartOuter = 0.f;
	float StartInner = 0.f;
	if (!NoteMI->GetScalarParameterValue(ANoteVisualizer::ParamName_StartOuterRadius, StartOuter)
		|| !NoteMI->GetScalarParameterValue(ANoteVisualizer::ParamName_StartInnerRadius, StartInner))
	{
		return false;
	}

	float WantOuter = 0.f;
	float WantInner = 0.f;
	if (!ANoteVisualizer::ComputeEndRadii(StartOuter, StartInner, AnchorInner, AnchorOuter, WantOuter, WantInner))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[NoteRingSync] %s: StartOuterRadius(%.5f)가 히트박스 밴드(%.5f~%.5f) 중앙까지 줄어들 수 없다"),
			*NoteMI->GetName(), StartOuter, AnchorInner, AnchorOuter);
		return false;
	}

	float CurrentOuter = 0.f;
	float CurrentInner = 0.f;
	NoteMI->GetScalarParameterValue(ANoteVisualizer::ParamName_EndOuterRadius, CurrentOuter);
	NoteMI->GetScalarParameterValue(ANoteVisualizer::ParamName_EndInnerRadius, CurrentInner);
	if (FMath::IsNearlyEqual(CurrentOuter, WantOuter, WriteEpsilon)
		&& FMath::IsNearlyEqual(CurrentInner, WantInner, WriteEpsilon))
	{
		return false;
	}

	NoteMI->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(ANoteVisualizer::ParamName_EndOuterRadius), WantOuter);
	NoteMI->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(ANoteVisualizer::ParamName_EndInnerRadius), WantInner);
	NoteMI->MarkPackageDirty();

	PushToOpenEditor(NoteMI, WantOuter, WantInner);

	UE_LOG(LogTemp, Log, TEXT("[NoteRingSync] %s: EndOuterRadius %.5f, EndInnerRadius %.5f (Start %.5f / %.5f)"),
		*NoteMI->GetName(), WantOuter, WantInner, StartOuter, StartInner);
	return true;
}

void UNoteRingRadiusSyncSubsystem::SyncAll() const
{
	const UTromboneConfig* Config = UTromboneConfig::Get();
	if (!Config)
	{
		return;
	}

	const FSoftObjectPath BasePath = Config->NoteRingBaseMaterial.ToSoftObjectPath();
	if (BasePath.IsNull())
	{
		return;
	}

	const IAssetRegistry& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	// 베이스를 직접 참조하는 패키지만 본다. 노트 MI는 전부 베이스의 자식이라 여기 들어온다
	TArray<FName> ReferencerPackages;
	AssetRegistry.GetReferencers(FName(*BasePath.GetLongPackageName()), ReferencerPackages);

	const FTopLevelAssetPath InstanceClassPath = UMaterialInstanceConstant::StaticClass()->GetClassPathName();
	int32 SyncedCount = 0;

	for (const FName& PackageName : ReferencerPackages)
	{
		TArray<FAssetData> AssetsInPackage;
		AssetRegistry.GetAssetsByPackageName(PackageName, AssetsInPackage);

		for (const FAssetData& AssetData : AssetsInPackage)
		{
			// 레벨 같은 무거운 패키지를 로드하지 않도록 클래스로 먼저 거른다
			if (AssetData.AssetClassPath != InstanceClassPath)
			{
				continue;
			}

			UMaterialInstanceConstant* NoteMI = Cast<UMaterialInstanceConstant>(AssetData.GetAsset());
			if (IsNoteRingInstance(NoteMI) && SyncOne(NoteMI))
			{
				++SyncedCount;
			}
		}
	}

	if (SyncedCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[NoteRingSync] 계산 근거가 바뀌어 노트 MI %d개의 End 반경을 다시 잡았다. 저장할 것"),
			SyncedCount);
	}
}
#endif
