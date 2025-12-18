// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Poolable.h"
#include "Utilities/Defines.h"
#include "JudgementRing.generated.h"

class ADefaultTromboneCharacter;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS(Abstract)
class TROMBONERUMBLE_API AJudgementRing : public AActor, public IPoolable
{
	GENERATED_BODY()
	
public:
	AJudgementRing();
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
	TWeakObjectPtr<ADefaultTromboneCharacter> CachedPlayerCharacter = nullptr;

	UPROPERTY(Transient)
	FNoteHandle NoteHandle;

	FDelegateHandle ProgressHandle, DespawnHandle;

	EInstrumentType InstrumentType;

	// Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	float StartOuterRadius = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	float StartInnerRadius = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	float EndOuterRadius = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	float EndInnerRadius = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JudgementRing|Params", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0", AllowPrivateAccess = "true"))
	float Alpha = 0.0f;

	// 머티리얼 파라미터 이름
	UPROPERTY(EditDefaultsOnly, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	FName ParamName_Alpha = "Alpha";

	UPROPERTY(EditDefaultsOnly, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	FName ParamName_StartOuter = "StartOuterRadius";

	UPROPERTY(EditDefaultsOnly, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	FName ParamName_StartInner = "StartInnerRadius";

	UPROPERTY(EditDefaultsOnly, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	FName ParamName_EndOuter = "EndOuterRadius";

	UPROPERTY(EditDefaultsOnly, Category = "JudgementRing|Params", meta = (AllowPrivateAccess = "true"))
	FName ParamName_EndInner = "EndInnerRadius";
};
