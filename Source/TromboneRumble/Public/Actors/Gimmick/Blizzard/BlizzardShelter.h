// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlizzardShelter.generated.h"

enum class EBlizzardState : uint8;
class USphereComponent;
class UPointLightComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UAnimSequence;
class ASkeletalMeshActor;

/** ABlizzardShelter
 * 눈보라 안전지대. 천막마다 하나씩 배치한다.
 * SafeZone sphere 안에 있는 캐릭터는 눈보라 효과(밀림/슬로우/래그돌)를 받지 않는다.
 * ABlizzardGimmick 이 TActorIterator 로 자동 수집하므로 별도 등록이 필요 없다.
 *
 * 눈보라 상태에 따라 ShelterLight 가 밝아진다 (대피처를 시각적으로 안내).
 * 페이드 커브만 BP FadeTimeline 이 담당한다:
 *   HandleBlizzardState() 가 시작/목표 밝기를 잡고 OnLightFadeRequested() 로 타임라인 재생을 요청
 *   -> 타임라인 Update 가 UpdateLightFade(Alpha) 를 호출
 *
 * 천막 문(DoorActor)이 지정된 쉘터는 문이 열려 있을 때만 안전지대로 친다 (IsSheltering()).
 * 문 여닫기는 서버가 정하고 bDoorOpen 만 복제된다 — 레벨의 문 액터(ASkeletalMeshActor)는
 * 복제 액터가 아니라서 스스로 상태를 실어 나르지 못한다.
 */
UCLASS()
class TROMBONERUMBLE_API ABlizzardShelter : public AActor
{
	GENERATED_BODY()

public:
	ABlizzardShelter();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** WorldLoc(주로 캐릭터 중심)이 안전지대 sphere 내부에 있으면 true. 문 상태는 보지 않는다. */
	bool IsLocationInsideShelter(const FVector& WorldLoc) const;

	/** ABlizzardGimmick 이 상태 전이마다 호출한다. 목표를 잡고 BP 페이드를 요청한다. */
	void HandleBlizzardState(EBlizzardState NewState);

	//~ 천막 문
	/** 서버 전용. 문을 열고/닫는다. 값이 같으면 아무것도 하지 않는다. */
	void SetDoorOpen(bool bOpen);

	bool IsDoorOpen() const { return bDoorOpen; }

	/** 이 쉘터에 여닫을 문이 지정돼 있는지 */
	bool HasDoor() const { return !DoorActor.IsNull(); }

	/** 지금 이 쉘터가 눈보라를 막아주는지. 문이 없는 쉘터는 항상 안전(기존 맵 호환). */
	bool IsSheltering() const { return !HasDoor() || bDoorOpen; }
	//~

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
	//~ 천막 문. 둘 다 비어 있어도 동작해야 한다 (문 없는 쉘터 = 항상 안전).
	/** 레벨에 배치된 천막 문 액터 (SKM_Market_open). Transient 를 붙이면 레벨 저장 시 참조가 날아간다. */
	UPROPERTY(EditInstanceOnly, Category = "Config|Shelter|Door", meta = (AllowPrivateAccess = "true", DisplayName = "천막 문 액터"))
	TSoftObjectPtr<ASkeletalMeshActor> DoorActor;

	/** 문 열림 애니메이션 (SKM_Market_open_Anim). 닫을 때는 이걸 역재생한다. */
	UPROPERTY(EditAnywhere, Category = "Config|Shelter|Door", meta = (AllowPrivateAccess = "true", DisplayName = "문 열림 애니메이션"))
	TSoftObjectPtr<UAnimSequence> DoorOpenAnim;

	/** 문 액터에서 뽑아둔 메시. 월드 소유라 약참조. */
	TWeakObjectPtr<USkeletalMeshComponent> DoorMesh;

	UPROPERTY(ReplicatedUsing = OnRep_DoorOpen)
	bool bDoorOpen = false;

	UFUNCTION()
	void OnRep_DoorOpen();

	/** DoorActor 를 로드해 DoorMesh 를 채운다. 이미 채워져 있으면 그대로 둔다. */
	void ResolveDoorMesh();

	/** bDoorOpen 을 실제 문에 반영 (콜리전은 전 넷모드, 애니메이션은 데디 제외).
	 *  bAnimate=false 는 초기 동기화용 — 재생 없이 포즈만 맞춘다. */
	void ApplyDoorState(bool bAnimate);
	//~

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
