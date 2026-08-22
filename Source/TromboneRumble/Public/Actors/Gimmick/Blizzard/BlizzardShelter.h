// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlizzardShelter.generated.h"

enum class EBlizzardState : uint8;
class USphereComponent;
class UPointLightComponent;
class USceneComponent;

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

	virtual void BeginPlay() override;

	/** WorldLoc(주로 캐릭터 중심)이 안전지대 sphere 내부에 있으면 true */
	bool IsLocationInsideShelter(const FVector& WorldLoc) const;

	/** ABlizzardGimmick 이 상태 전이마다 호출한다. 목표를 잡고 BP 페이드를 요청한다. */
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

	/** 런타임에 밝기를 바꾸므로 Mobility 를 Static 으로 두면 안 된다 (setter 가 무시됨).
	 *  평상시(Idle) 상태 = 배치된 이 라이트의 값 그대로 (BP 에서 Intensity 0 저작). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Shelter")
	TObjectPtr<UPointLightComponent> ShelterLight;

private:
	//~ 상태별 라이트 템플릿 (invisible; 읽기 전용). 클래스 기본값과 다른 값만 해당 상태에서 구동된다.
	//  값을 넣는 경로는 저장 버튼 하나뿐: ShelterLight 를 원하는 룩으로 조정 → "전조/눈보라/평상시 상태 저장" 클릭.
	//  (Details 직접 편집은 bEditableWhenInherited=false 로 잠겨 있다 — BlizzardShelter.cpp 생성자 참조)
	//  주의: 포인트라이트 체감 밝기는 배경 밝기에 상대적이다. Warning(노을=아직 밝음)은
	//  배경이 밝아 Active(어두운 눈보라)보다 훨씬 높은 값이라야 빛이 배경을 이기고 보인다.
	UPROPERTY(VisibleAnywhere, Category = "Config|Shelter", meta = (AllowPrivateAccess = "true", DisplayName = "예고 라이트"))
	TObjectPtr<UPointLightComponent> WarningLightTemplate;

	UPROPERTY(VisibleAnywhere, Category = "Config|Shelter", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 라이트"))
	TObjectPtr<UPointLightComponent> ActiveLightTemplate;

	/** 평상시 템플릿. 위 둘과 달리 ShelterLight 상태를 통째로 담는다 (미리보기가 전체 복사라서). */
	UPROPERTY(VisibleAnywhere, Category = "Config|Shelter", meta = (AllowPrivateAccess = "true", DisplayName = "평상시 라이트"))
	TObjectPtr<UPointLightComponent> NormalLightTemplate;
	//~

	//~ 런타임 보간 스냅샷 (Outer=TransientPackage). Idle=Normal, Warning/Active=평상시+템플릿 오버라이드.
	UPROPERTY(Transient) TObjectPtr<UPointLightComponent> StartSnapshot;
	UPROPERTY(Transient) TObjectPtr<UPointLightComponent> NormalSnapshot;
	UPROPERTY(Transient) TObjectPtr<UPointLightComponent> ResolvedWarning;
	UPROPERTY(Transient) TObjectPtr<UPointLightComponent> ResolvedActive;

	/** 현재 페이드가 향하는 상태. 생성자에서 Idle 로 초기화 (헤더에선 enum 이 전방선언이라 대입 불가). */
	EBlizzardState LightTargetState;

	/** 상태별 목표 스냅샷을 돌려준다. */
	UPointLightComponent* GetLightTargetFor(EBlizzardState State) const;
	//~

#if WITH_EDITOR
public:
	//~ 에디터 저작 버튼. 기믹의 저장/미리보기 버튼에서도 팬아웃된다.
	//  패널 순서는 선언 순서가 아니라 DisplayPriority 로 정해진다 (없으면 함수명 알파벳순).
	UFUNCTION(CallInEditor, Category = "Shelter|Editor|Save|Load", meta = (DisplayName = "전조 상태 저장", DisplayPriority = "1"))
	void SaveWarningFromWorld();
	UFUNCTION(CallInEditor, Category = "Shelter|Editor|Save|Load", meta = (DisplayName = "눈보라 상태 저장", DisplayPriority = "2"))
	void SaveActiveFromWorld();
	UFUNCTION(CallInEditor, Category = "Shelter|Editor|Save|Load", meta = (DisplayName = "평상시 상태 저장", DisplayPriority = "3"))
	void SaveNormalFromWorld();
	UFUNCTION(CallInEditor, Category = "Shelter|Editor|Save|Load", meta = (DisplayName = "전조 상태 미리보기", DisplayPriority = "4"))
	void LoadWarningToWorld();
	UFUNCTION(CallInEditor, Category = "Shelter|Editor|Save|Load", meta = (DisplayName = "눈보라 상태 미리보기", DisplayPriority = "5"))
	void LoadActiveToWorld();
	UFUNCTION(CallInEditor, Category = "Shelter|Editor|Save|Load", meta = (DisplayName = "평상시 상태 미리보기", DisplayPriority = "6"))
	void LoadNormalToWorld();
	UFUNCTION(CallInEditor, Category = "Shelter|Editor|Save|Load", meta = (DisplayName = "평상시 복원", DisplayPriority = "7"))
	void RestoreNormalToWorld();

	// 기믹 팬아웃 진입점 (버튼 아님 — 호출측이 트랜잭션을 소유).
	void EditorSaveState(EBlizzardState State);
	void EditorLoadState(EBlizzardState State);
	void EditorRestoreNormal();

private:
	void EditorEnsureNormalBackup();
	/** 상태에 대응하는 템플릿 고르기. 저장/미리보기가 같은 규칙을 쓰도록 한 곳에 모은다. */
	UPointLightComponent* EditorPickStateTemplate(EBlizzardState State) const;
#endif

#if WITH_EDITORONLY_DATA
	/** 미리보기 전 자동 캡처한 평상시 백업 (세션 1회). */
	UPROPERTY(Transient)
	TObjectPtr<UPointLightComponent> EditorNormalBackup;
#endif
};
