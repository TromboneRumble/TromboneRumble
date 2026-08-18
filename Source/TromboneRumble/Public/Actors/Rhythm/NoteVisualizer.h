// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Poolable.h"
#include "Utilities/Defines.h"
#include "NoteVisualizer.generated.h"

class URingHitBoxComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

UCLASS(Abstract)
class TROMBONERUMBLE_API ANoteVisualizer : public AActor, public IPoolable
{
	GENERATED_BODY()
	
public:
	ANoteVisualizer();
	void Init(const FNoteHandle& InNoteHandle, const EInstrumentType& InType, const EInstrumentType& HeldType);
	
	// IPoolable interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnTakenFromPool();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnReturnToPool();
	// End of IPoolable interface

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	void SetAlpha(float InAlpha);

#if WITH_EDITOR
	//~ Begin 링 반경 규칙 (에디터 전용)
	// End 반경 2개는 Start 반경에서 따라 나온다. 외곽은 소멸 진행도에 히트박스 구멍에 딱 닿게,
	// 안쪽은 시작 두께를 그대로 유지하게. 계산과 기입은 에디터에서만 하고 MI에 저장된다.
	// 인게임은 저장된 값을 그대로 쓴다 — 아래는 계산식과 PIE 검증용이다

	// StartOuter가 히트박스 구멍 이하면 계산이 불가능해 false
	static bool ComputeEndRadii(float StartOuter, float StartInner, float AnchorInner,
		float InMissEndAlpha, float& OutEndOuter, float& OutEndInner);

	// 머티리얼 그래프에 있는 이름과 맞아야 한다
	static const FName ParamName_StartOuterRadius;
	static const FName ParamName_StartInnerRadius;
	static const FName ParamName_EndOuterRadius;
	static const FName ParamName_EndInnerRadius;

	FORCEINLINE float GetMissEndAlpha() const { return MissEndAlpha; }
	//~ End 링 반경 규칙
#endif

private:
	void EnsureMID();
	void ApplyMaterialParams();

	// 현재 맵의 RhythmActor가 지정한 링 머티리얼. 없으면 nullptr
	UMaterialInterface* ResolvePerMapOverrideMaterial() const;

#if WITH_EDITOR
	//~ Begin 링 반경 규칙 (에디터 전용)
	// MI에 저장된 End 반경이 규칙과 맞는지 보고 어긋나면 경고만 한다. 값은 고치지 않는다
	void ValidateEndRadii();
	//~ End 링 반경 규칙
#endif

	void BindChannel();
	void UnBindChannel();
	void ShowRing(bool bShow);
	void FindPlayerCharacterAndAttach();
	UFUNCTION()
	void HandleOnInstrumentPicked(EInstrumentType OldType, EInstrumentType NewType);


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> RingMesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MID;

	UPROPERTY(Transient)
	TWeakObjectPtr<URingHitBoxComponent> CachedRingHitBoxComponent = nullptr;

	UPROPERTY(Transient)
	FNoteHandle NoteHandle;

	FDelegateHandle ProgressHandle, DespawnHandle;

	EInstrumentType InstrumentType;

	// Parameters
	// 머티리얼에 넘기는 값. 1.0 = 판정선 도달, 그 위는 MissEndAlpha까지 같은 속도로 이어진다
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NoteVisualizer|Params", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.5", AllowPrivateAccess = "true"))
	float SizeAlpha = 0.0f;

	// 노트가 Destroyer에 잡혀 사라지는 진행도. BP_RhythmActor의 Destroyer 배치와 맞출 것
	UPROPERTY(EditDefaultsOnly, Category = "NoteVisualizer|Params", meta = (ClampMin = "1.0", AllowPrivateAccess = "true"))
	float MissEndAlpha = 1.12f;

#if WITH_EDITOR
	//~ Begin 링 반경 규칙 (에디터 전용)
	// 같은 경고를 노트마다 다시 찍지 않기 위한 값
	float LastWarnedStartOuter = -1.0f;
	//~ End 링 반경 규칙
#endif

	// 머티리얼 파라미터 이름
	UPROPERTY(EditDefaultsOnly, Category = "NoteVisualizer|Params", meta = (AllowPrivateAccess = "true"))
	FName ParamName_SizeAlpha = "SizeAlpha";
};
