#include "Data/WwiseData.h"
#include "AkGameplayStatics.h"

namespace WwiseRTPC
{
	void SetVolume(FName RTPCName, float Value)
	{
		UAkGameplayStatics::SetRTPCValue(nullptr, Value * 100.f, 0, nullptr, RTPCName);
	}
}
