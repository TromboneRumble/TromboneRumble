// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Utilities/Defines.h"
#include "VoiceChatSubsystem.generated.h"

class APlayerState;
class UAmplifiedAudioCaptureComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceTalkModeChanged, EVoipMode, NewMode);

/**
 * Per-local-player voice chat state: talk mode (PTT/Auto), per-remote mute set, per-remote volume.
 * Persists across map transitions (e.g. MatchMenuMap → InGame).
 */
UCLASS()
class TROMBONERUMBLE_API UVoiceChatSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetTalkMode(EVoipMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Voice")
	EVoipMode GetTalkMode() const { return TalkMode; }

	/** Called by the player controller when PTT is pressed (or when Auto mode is entered). */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void BeginLocalTalk();

	/** Called when PTT is released (no-op in Auto mode). */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void EndLocalTalk();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetRemotePlayerMuted(APlayerState* PS, bool bMuted);

	UFUNCTION(BlueprintPure, Category = "Voice")
	bool IsRemotePlayerMuted(APlayerState* PS) const;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetRemotePlayerVolume(APlayerState* PS, float Volume);

	UFUNCTION(BlueprintPure, Category = "Voice")
	float GetRemotePlayerVolume(APlayerState* PS) const;

	UPROPERTY(BlueprintAssignable, Category = "Voice")
	FOnVoiceTalkModeChanged OnTalkModeChanged;

	/** OSS 보이스 인터페이스에 로컬 talker를 등록. 멱등적이므로 여러 번 호출해도 안전.
	 *  PC BeginPlay 등 맵 전환 시점에 호출해 PTT 첫 누름 전에 등록을 보장한다. */
	void EnsureLocalTalkerRegistered();

	/** 발신자 VoiceSendVolume × 청취자 로컬 조절값을 곱해 TromboneVOIPTalker에 적용.
	 *  PlayerState의 OnRep_VoiceSendVolume 및 RegisterTalker 시점에서 호출된다. */
	void ApplyVolumeToTalker(APlayerState* PS);

	/** 현재 연결된 마이크 장치 이름 목록 반환 (OS 보고 기준). */
	TArray<FString> GetAvailableMicDeviceNames();

	/** 마이크 설정 적용: 선택 장치 인덱스 저장, 전송 볼륨 OSS에 반영. */
	void ApplyMicrophoneSettings(int32 DeviceIndex, float VoiceSendVolume);

	/** 마이크 테스트 시작: 선택된 장치에서 캡처해 로컬 스피커로 재생. */
	void BeginMicTest(int32 DeviceIndex);

	/** 마이크 테스트 중단. */
	void EndMicTest();

	bool IsMicTesting() const { return bMicTestActive; }

	/** 마이크 테스트 중 로컬 재생 볼륨 실시간 조절 (0.0 ~ 1.0). 테스트 비활성 시 무시. */
	void SetMicTestVolume(float Volume);

	/** 잡음 제거 ON/OFF. 실제 VoIP(voice.MicNoiseGateThreshold CVar)와 마이크 테스트 경로 모두에 적용. */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetNoiseSuppression(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Voice")
	bool IsNoiseSuppressionEnabled() const { return bNoiseSuppressionEnabled; }

private:
	class APlayerController* GetOwningPlayerController() const;
	void ApplyNetworkedVoice(bool bActive);

	UPROPERTY()
	EVoipMode TalkMode = EVoipMode::PushToTalk;

	UPROPERTY()
	TSet<TWeakObjectPtr<APlayerState>> MutedPlayers;

	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerState>, float> VolumeMap;

	// UAmplifiedAudioCaptureComponent: 마이크 캡처 + gain 증폭 + 로컬 재생
	UPROPERTY()
	TObjectPtr<UAmplifiedAudioCaptureComponent> MicCaptureComp;

	bool bMicTestActive = false;
	bool bNoiseSuppressionEnabled = true;

	// voice.MicNoiseGateThreshold에 적용할 값 (linear amplitude 0~1 기준).
	// 캡처 PCM에 직접 적용되므로 말소리 피크(보통 0.05~0.3)보다 충분히 낮아야 음성이 안 잘림.
	static constexpr float NoiseSuppressionThreshold = 0.03f;
	// 잡음 제거 OFF에서도 완전 무음 구간까지 송신하지 않도록 유지하는 최소 게이트
	static constexpr float NoiseGateBaseThreshold = 0.01f;
	// voice.SilenceDetectionThreshold — 이 값 이하 구간은 아예 패킷으로 안 실림
	static constexpr float SilenceDetectionThreshold = 0.005f;

	/** 월드 파괴 시작 시 OSS의 원격 talker 버퍼를 비워 VoipListenerSynthComponent를
	 *  FScene::Release() 전에 해제시킨다. (상세: .cpp 구현부 주석) */
	void HandleWorldBeginTearDown(UWorld* World);

	FDelegateHandle WorldTearDownHandle;
};
