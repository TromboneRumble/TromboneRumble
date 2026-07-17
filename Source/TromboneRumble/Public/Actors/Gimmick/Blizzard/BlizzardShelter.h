// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlizzardShelter.generated.h"

enum class EBlizzardState : uint8;
class USphereComponent;
class UPointLightComponent;

/** ABlizzardShelter
 * 눈보라 안전지대. 천막마다 하나씩 배치한다.
 * SafeZone sphere 안에 있는 캐릭터는 눈보라 효과(밀림/슬로우/래그돌)를 받지 않는다.
 * ABlizzardGimmick 이 TActorIterator 로 자동 수집하므로 별도 등록이 필요 없다.
 *
 * 눈보라 상태에 따라 ShelterLight 가 밝아진다 (대피처를 시각적으로 안내).
 * 페이드 커브만 BP FadeTimeline 이 담당한다:
 *   HandleBlizzardState() 가 시작/목표 밝기를 잡고 OnLightFadeRequested() 로 타임라인 재생을 요청
 *   -> 타임라인 Update 가 UpdateLightFade(Alpha) 를 호출
 */
UCLASS()
class TROMBONERUMBLE_API ABlizzardShelter : public AActor
{
	GENERATED_BODY()

public:
	ABlizzardShelter();

	/** WorldLoc(주로 캐릭터 중심)이 안전지대 sphere 내부에 있으면 true */
	bool IsLocationInsideShelter(const FVector& WorldLoc) const;

	/** ABlizzardGimmick 이 상태 전이마다 호출한다. 목표 밝기를 잡고 BP 페이드를 요청한다. */
	void HandleBlizzardState(EBlizzardState NewState);

	/** BP FadeTimeline 의 Update 에서 호출 (Alpha 0~1) */
	UFUNCTION(BlueprintCallable, Category = "Shelter")
	void UpdateLightFade(float Alpha);

protected:
	/** BP 에서 FadeTimeline 을 PlayFromStart 할 것 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
	void OnLightFadeRequested();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Shelter")
	TObjectPtr<USphereComponent> SafeZone;

	/** 런타임에 밝기를 바꾸므로 Mobility 를 Static 으로 두면 안 된다 (setter 가 무시됨). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Shelter")
	TObjectPtr<UPointLightComponent> ShelterLight;

private:
	//~ 상태별 목표 밝기.
	//  주의: 포인트라이트 체감 밝기는 배경 밝기에 상대적이다. Warning(노을=아직 밝음)은
	//  배경이 밝아 Active(어두운 눈보라)보다 훨씬 높은 값이라야 빛이 배경을 이기고 보인다.
	UPROPERTY(EditAnywhere, Category = "Config|Shelter", meta = (AllowPrivateAccess = "true", DisplayName = "평상시 밝기"))
	float IdleIntensity = 0.f;

	UPROPERTY(EditAnywhere, Category = "Config|Shelter", meta = (AllowPrivateAccess = "true", DisplayName = "예고 밝기"))
	float WarningIntensity = 18000.f;

	UPROPERTY(EditAnywhere, Category = "Config|Shelter", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 밝기"))
	float ActiveIntensity = 3500.f;
	//~

	//~ 현재 페이드 구간. 런타임 전용 (디버깅용으로만 노출)
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Config|Shelter", meta = (AllowPrivateAccess = "true"))
	float StartIntensity = 0.f;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Config|Shelter", meta = (AllowPrivateAccess = "true"))
	float TargetIntensity = 0.f;
	//~
};
