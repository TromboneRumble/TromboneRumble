// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"  
#include "PuddleTrap.generated.h"

class UGameplayEffect;
class UBoxComponent;
class UDecalComponent;
class UPhysicalMaterial;
class ACharacter;

UCLASS(Abstract)
class TROMBONERUMBLE_API APuddleTrap : public AActor
{
	GENERATED_BODY()
	
public:	
	APuddleTrap();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDecalComponent> DecalComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Puddle|GAS")
	TSubclassOf<UGameplayEffect> PuddleSlowEffectClass;
	//~Components

	/** 0 → 최종 크기로 커지는 데 걸리는 시간(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Puddle")
	float GrowDuration = 0.5f;

	/** 스폰 후 FadeOut 시작까지 딜레이(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Puddle")
	float FadeDelay = 10.f;

	/** FadeOut에 걸리는 시간(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Puddle")
	float FadeDuration = 1.f;

private:
	float GrowElapsed = 0.f;
	bool bGrowing = false;

	FTimerHandle LifetimeTimerHandle;

	/** 이 웅덩이가 각 캐릭터에게 건 Slow GE 핸들 */
	UPROPERTY()
	TMap<TWeakObjectPtr<ACharacter>, struct FActiveGameplayEffectHandle> ActiveSlowEffects;

};
