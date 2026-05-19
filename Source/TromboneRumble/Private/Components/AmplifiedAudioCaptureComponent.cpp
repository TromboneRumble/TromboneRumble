#include "Components/AmplifiedAudioCaptureComponent.h"

int32 UAmplifiedAudioCaptureComponent::OnGenerateAudio(float* OutAudio, int32 NumSamples)
{
	const int32 Generated = Super::OnGenerateAudio(OutAudio, NumSamples);

	if (bNoiseSuppressionEnabled.load() && Generated > 0)
	{
		// RMS 계산
		float SumSq = 0.0f;
		for (int32 i = 0; i < Generated; ++i)
		{
			SumSq += OutAudio[i] * OutAudio[i];
		}
		const float Rms = FMath::Sqrt(SumSq / static_cast<float>(Generated));

		// Attack/Release 엔벨로프로 게이트 게인 갱신
		// Attack(열림): 빠르게 — 소리 감지 즉시 게이트 개방
		// Release(닫힘): 느리게 — 짧은 침묵에 게이트가 닫혀버리는 것 방지
		if (Rms >= GateThreshold)
		{
			GateGain = FMath::Min(1.0f, GateGain + AttackStep);
		}
		else
		{
			GateGain = FMath::Max(0.0f, GateGain - ReleaseStep);
		}

		for (int32 i = 0; i < Generated; ++i)
		{
			OutAudio[i] *= GateGain;
		}
	}

	const float Gain = GainMultiplier.load();
	if (!FMath::IsNearlyEqual(Gain, 1.0f))
	{
		for (int32 i = 0; i < Generated; ++i)
		{
			OutAudio[i] *= Gain;
		}
	}
	return Generated;
}
