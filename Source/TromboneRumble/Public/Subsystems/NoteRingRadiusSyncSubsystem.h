// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "NoteRingRadiusSyncSubsystem.generated.h"

class UMaterialInstanceConstant;

/**
 * 노트 링 MI의 End 반경을 대신 채워 넣는다 (에디터 전용).
 *
 * MI에서 Start 반경만 고치면 End 2개는 규칙대로 다시 계산돼 에셋에 기입된다.
 * End를 손으로 고쳐도 곧바로 계산값으로 되돌아가므로 사실상 읽기 전용이다.
 * 규칙과 계산식은 ANoteVisualizer::ComputeEndRadii에 있고, 인게임은 여기서 저장한 값을 그대로 쓴다.
 */
UCLASS()
class TROMBONERUMBLE_API UNoteRingRadiusSyncSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#if WITH_EDITOR
private:

	/** 프로퍼티가 바뀔 때마다 불린다. 노트 MI거나 계산 근거면 다시 기입한다 */
	void HandleObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event);

	/** 부모를 거슬러 올라가 설정에 적힌 노트 베이스 머티리얼이 나오는지 본다 */
	bool IsNoteRingInstance(const UMaterialInstanceConstant* MI) const;

	/** 히트박스 밴드 반경 2개를 설정에 적힌 캐릭터 BP CDO에서 읽는다 */
	bool ResolveRule(float& OutAnchorInner, float& OutAnchorOuter) const;

	/** MI 하나에 End 반경을 써넣는다. 이미 맞으면 아무것도 안 하고 false */
	bool SyncOne(UMaterialInstanceConstant* NoteMI) const;

	/** 베이스 머티리얼을 쓰는 MI를 전부 다시 기입한다. 계산 근거가 바뀌었을 때만 */
	void SyncAll() const;

	FDelegateHandle PropertyChangedHandle;

	/** 우리가 쓴 값이 콜백을 다시 타는 것을 막는다 */
	bool bSyncing = false;
#endif
};
