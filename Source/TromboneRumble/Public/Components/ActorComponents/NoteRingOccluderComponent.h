// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NoteRingOccluderComponent.generated.h"

class AController;
class UMeshComponent;

/**
 * 발밑 노트 링(M_NoteVisualizer / M_NoteHitBox)이 내 캐릭터에게 가려지게 하는 표식.
 *
 * 두 링 머티리얼은 깊이 테스트를 꺼서 지형을 뚫고, CustomStencil == 252인 픽셀에서만 숨는다.
 * 이 컴포넌트가 오너 + 붙어 있는 액터(악기/무기)의 메시에 252를 찍는다.
 * 로컬 조종 캐릭터 전용이며 늦게 오는 possession도 스스로 따라간다.
 */
UCLASS(ClassGroup=(Rhythm))
class TROMBONERUMBLE_API UNoteRingOccluderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNoteRingOccluderComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleControllerChanged(APawn* InPawn, AController* OldController, AController* NewController);

	void RefreshTickEnabled();

	/** 오너 + 부착 액터의 메시에 스텐실을 찍고, 빠진 메시는 해제 */
	void RefreshStencilTargets();
	void ClearAllStencils();

	/** 링을 가려야 하는 메시인가 (위젯/반투명 메시 제외) */
	static bool IsOccluderMesh(const UMeshComponent* Mesh);

	// 지금 스텐실을 찍어둔 메시들. 다음 스캔에서 빠지면 해제한다
	TSet<TWeakObjectPtr<UMeshComponent>> StenciledMeshes;
};
