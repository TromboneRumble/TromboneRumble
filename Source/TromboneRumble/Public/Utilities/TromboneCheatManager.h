#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "TromboneCheatManager.generated.h"

enum class EWeaponType : uint8;
class UXRayBenchmark;

/* Press '~' in-game to open the console and type commands below
 */
UCLASS()
class TROMBONERUMBLE_API UTromboneCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
/** Gameplay Console Commands */
public: 
	
	UFUNCTION(Exec)
	void Trombone_Help();
	
	UFUNCTION(Exec)
	void Trombone_SpawnInstrument(const FString& TypeString);
	
	UFUNCTION(Exec)
	void Trombone_Spotlight();
	
	UFUNCTION(Exec)
	void Trombone_Throw(const FString& Count);

	UFUNCTION(Exec)
	void Trombone_Ragdoll();

	UFUNCTION(Exec)
	void Trombone_Stun();

	// Drops every player from the lobby falling points again. Host console only, for repeated ragdoll measurement
	UFUNCTION(Exec)
	void Trombone_RagdollDrop();

	// Throws every standing player up in a ragdoll, like a headbutt hit. Host console only
	UFUNCTION(Exec)
	void Trombone_RagdollLaunch();

	// Runs every ragdoll test case in Project Settings > Game > Ragdoll Test. Any PIE window, lobby only
	UFUNCTION(Exec)
	void Trombone_RagdollTest();

	UFUNCTION(Exec)
	void Trombone_RagdollTestStop();
	
	UFUNCTION(Exec)
	void Trombone_ResetSettingData();

	UFUNCTION(Exec)
	void Trombone_SetCustomization(const FString& AntennaKey, const FString& FaceKey, const FString& CostumeKey);

	// 리듬 BGM 오프셋(ms). 양수면 BGM을 그만큼 일찍 시작. 인자 없으면 현재 값 출력
	UFUNCTION(Exec)
	void Trombone_AudioOffset(const FString& MsString);

	// 리듬 싱크 실측 로그 + 음악 클럭 화면 표시 on/off. 인자 없으면 현재 값 출력
	UFUNCTION(Exec)
	void Trombone_RhythmSyncLog(const FString& EnabledString);

	// X-Ray 방식 전환 (silhouette | dither | window | off). 인자 없으면 현재 붙어있는 컴포넌트 출력
	UFUNCTION(Exec)
	void Trombone_XRayMode(const FString& ModeString);

	// X-Ray 3종의 프레임 비용을 순서대로 측정해 비교표 출력
	UFUNCTION(Exec)
	void Trombone_XRayBench(const FString& SecondsString);

	// 원형 윈도우 크기/가장자리/가리는물체 투명도 런타임 조절. 인자 없으면 현재 값 출력
	// (저장되지 않음 — 마음에 드는 값은 BP에 옮겨 적을 것)
	UFUNCTION(Exec)
	void Trombone_XRayWindow(const FString& RadiusString, const FString& SoftnessString, const FString& OccluderOpacityString);

	// 원형 윈도우의 캡처 시야 크롭 on/off. 크롭 전후 비용을 Trombone_XRayBench로 비교할 때 쓴다
	UFUNCTION(Exec)
	void Trombone_XRayCropCapture(const FString& EnabledString);

	// 더미 플레이어를 채워 결과 씬으로 이동. 인원수 기본 4, 스테이지 기본 OrchestraStage
	UFUNCTION(Exec)
	void Trombone_ResultTest(const FString& PlayerCountString, const FString& StageString);

public:

	UFUNCTION(Exec)
	void Trombone_Dump_LevelStateSubsystem();

private:

	// Trombone_XRayBench가 쓰는 측정 오케스트레이터 (첫 실행 때 생성)
	UPROPERTY(Transient)
	TObjectPtr<UXRayBenchmark> XRayBenchmark;
};
