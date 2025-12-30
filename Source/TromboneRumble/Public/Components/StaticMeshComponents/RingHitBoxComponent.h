// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "RingHitBoxComponent.generated.h"


enum class ENoteResult : uint8;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API URingHitBoxComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	URingHitBoxComponent();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION()
	void OnNoteDetectedHandler(ENoteResult InNoteResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Material", meta = (AllowPrivateAccess = "true"))
	float StartOuterRadius = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Material", meta = (AllowPrivateAccess = "true"))
	float StartInnerRadius = 0.49f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Material", meta = (AllowPrivateAccess = "true"))
	float EndOuterRadius = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Material", meta = (AllowPrivateAccess = "true"))
	float EndInnerRadius = 0.09f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Material", meta = (AllowPrivateAccess = "true"))
	float SizeAlpha = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Material", meta = (AllowPrivateAccess = "true"))
	float FadePercent = 0.3f;

	// 노트 결과에 따라 Flash
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Flash", meta = (AllowPrivateAccess = "true"))
	FName ColorParamName = TEXT("CircleColor"); 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Flash",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "판정 플래시가 유지되는 시간"))
	float FlashDuration = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Flash", meta = (AllowPrivateAccess = "true"))
	FLinearColor BadFlashColor = FLinearColor(1.f, 0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Flash", meta = (AllowPrivateAccess = "true"))
	FLinearColor GoodFlashColor = FLinearColor(1.f, 1.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RingHitBox|Flash", meta = (AllowPrivateAccess = "true"))
	FLinearColor ExcellentFlashColor = FLinearColor(0.f, 1.f, 0.f, 1.f);


	UPROPERTY(EditDefaultsOnly, Category = "RingHitBox", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> RingMatOrigin = nullptr;

private:

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RingMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> CachedParentMat = nullptr;

	FLinearColor CachedBaseColor = FLinearColor::White;
	bool bHasBaseColor = false;

	FTimerHandle FlashTimerHandle;

private:
	void EnsureMID();
	void ApplyMaterialParams();

	void CacheBaseColorIfNeeded();
	void FlashToColor(const FLinearColor& InColor);
	void RestoreBaseColor();
		
};
