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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JudgementRing|Params", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0", AllowPrivateAccess = "true"))
	float SizeAlpha = 0.0f;

	// 머티리얼 파라미터 이름
	UPROPERTY(EditDefaultsOnly, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	FName ParamName_SizeAlpha = "SizeAlpha";
};
