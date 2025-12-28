// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RingHitBoxComponent.generated.h"


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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JudgementRing|Material", meta = (AllowPrivateAccess = "true"))
	float StartOuterRadius = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JudgementRing|Material", meta = (AllowPrivateAccess = "true"))
	float StartInnerRadius = 0.49f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JudgementRing|Material", meta = (AllowPrivateAccess = "true"))
	float EndOuterRadius = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JudgementRing|Material", meta = (AllowPrivateAccess = "true"))
	float EndInnerRadius = 0.09f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JudgementRing|Material", meta = (AllowPrivateAccess = "true"))
	float SizeAlpha = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JudgementRing|Material", meta = (AllowPrivateAccess = "true"))
	float FadePercent = 0.3f;

private:

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RingMID = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMaterialInterface> CachedParentMat = nullptr;

private:
	void EnsureMID();
	void ApplyMaterialParams();
		
};
