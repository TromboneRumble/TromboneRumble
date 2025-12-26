// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"  
#include "PuddleTrap.generated.h"

class UAkAudioEvent;
class UAkComponent;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAkComponent> AkComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Puddle|GAS")
	TSubclassOf<UGameplayEffect> PuddleSlowEffectClass;
	//~Components

	/** 웅덩이가 생성된 후 최소 크기에서 최종 크기까지 커지는 데 걸리는 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Puddle|Config", meta = (DisplayName = "확장 소요 시간"))
	float GrowDuration = 0.5f;

	/** 웅덩이가 최대 크기를 유지하는 시간이며, 이 시간이 지나면 서서히 사라지기 시작합니다. (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Puddle|Config", meta = (DisplayName = "소멸 시작 대기 시간"))
	float FadeDelay = 10.f;

	/** 웅덩이가 완전히 투명해져서 사라질 때까지 걸리는 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Puddle|Config", meta = (DisplayName = "소멸 단계 지속 시간"))
	float FadeDuration = 1.f;

private:
	float GrowElapsed = 0.f;
	bool bGrowing = false;

	FTimerHandle LifetimeTimerHandle;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> PuddleSpawnSFX;

	/** 이 웅덩이가 각 캐릭터에게 건 Slow GE 핸들 */
	UPROPERTY()
	TMap<TWeakObjectPtr<ACharacter>, struct FActiveGameplayEffectHandle> ActiveSlowEffects;

};
