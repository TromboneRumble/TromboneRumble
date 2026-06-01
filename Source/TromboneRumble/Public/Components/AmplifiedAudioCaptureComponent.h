#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureComponent.h"
#include "AmplifiedAudioCaptureComponent.generated.h"

UCLASS(ClassGroup = Audio, meta = (BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UAmplifiedAudioCaptureComponent : public UAudioCaptureComponent
{
	GENERATED_BODY()

public:
	void SetGain(float InGain) { GainMultiplier.store(FMath::Max(0.0f, InGain)); }
	void SetNoiseSuppression(bool bEnabled) { bNoiseSuppressionEnabled.store(bEnabled); }

protected:
	virtual int32 OnGenerateAudio(float* OutAudio, int32 NumSamples) override;

private:
	std::atomic<float> GainMultiplier{1.0f};
	std::atomic<bool>  bNoiseSuppressionEnabled{true};

	// Noise Gate 상태 (오디오 스레드 전용 — atomic 불필요)
	float GateGain = 1.0f;
	static constexpr float GateThreshold = 0.015f;
	static constexpr float AttackStep    = 0.25f;    // 게이트 열림 속도 — 빠른 개방 (~4 버퍼 호출)
	static constexpr float ReleaseStep   = 0.005f;   // 게이트 닫힘 속도 — 느린 차단 (~200 버퍼 호출)
};
