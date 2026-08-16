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

private:
	void EnsureMID();
	void ApplyMaterialParams();

	// 현재 맵의 RhythmActor가 지정한 링 머티리얼. 없으면 nullptr
	UMaterialInterface* ResolvePerMapOverrideMaterial() const;

	// 반경 4종은 전부 머티리얼/MI 소유. 히트박스는 소멸 앵커로만 쓴다
	// 링이 히트박스를 벗어나는 진행도를 재서 소멸 지점과 맞는지 검증한다
	void UpdateNestSizeAlpha();

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

	// 링 외곽이 히트박스 안쪽에 맞물리는 SizeAlpha. 소멸 지점과 같아야 해서 검증에만 쓴다
	float NestSizeAlpha = 1.0f;

	// 같은 경고를 노트마다 다시 찍지 않기 위한 값
	float LastWarnedNestAlpha = -1.0f;

	// 머티리얼 파라미터 이름
	UPROPERTY(EditDefaultsOnly, Category = "NoteVisualizer|Params", meta = (AllowPrivateAccess = "true"))
	FName ParamName_SizeAlpha = "SizeAlpha";
};
