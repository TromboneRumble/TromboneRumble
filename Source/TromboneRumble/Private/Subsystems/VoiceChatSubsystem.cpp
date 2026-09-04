// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/VoiceChatSubsystem.h"
#include "Components/ActorComponents/TromboneVOIPTalker.h"
#include "Framework/DefaultPlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/VoiceInterface.h"
#include "Components/AmplifiedAudioCaptureComponent.h"
#include "HAL/IConsoleManager.h"
#include "Net/VoiceConfig.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "SaveData/TromboneSaveGame.h"

void UVoiceChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TalkMode = EVoipMode::PushToTalk;
	MutedPlayers.Reset();
	VolumeMap.Reset();

	// 저장된 VOIPSetting으로 TalkMode 초기화 (PC 미존재로 Begin/EndLocalTalk는 no-op)
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (const UGameInstance* GI = LP->GetGameInstance())
		{
			if (const USaveManagerSubsystem* SaveMgr = GI->GetSubsystem<USaveManagerSubsystem>())
			{
				TalkMode = SaveMgr->GetGameplaySettings().VOIPSetting;
			}
		}
	}

	// 침묵 감지 임계값 고정: 엔진 기본 0.08(linear amplitude)은 일반 말소리 피크보다 높아
	// 송신 단계에서 음성이 잘려 상대에게 작고 끊기게 들린다. ini [SystemSettings]와 이중 안전장치.
	UVOIPStatics::SetMicThreshold(SilenceDetectionThreshold);
	SetNoiseSuppression(bNoiseSuppressionEnabled);

	WorldTearDownHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(
		this, &UVoiceChatSubsystem::HandleWorldBeginTearDown);
}

void UVoiceChatSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldBeginTearDown.Remove(WorldTearDownHandle);
	EndMicTest();
	EndLocalTalk();
	Super::Deinitialize();
}

APlayerController* UVoiceChatSubsystem::GetOwningPlayerController() const
{
	const ULocalPlayer* LP = GetLocalPlayer();
	return LP ? LP->GetPlayerController(LP->GetWorld()) : nullptr;
}

void UVoiceChatSubsystem::ApplyNetworkedVoice(bool bActive)
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->ToggleSpeaking(bActive);
	}
}

void UVoiceChatSubsystem::EnsureLocalTalkerRegistered()
{
	const ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UWorld* World = LP->GetWorld();
	if (!World)
	{
		return;
	}

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (!OSS)
	{
		return;
	}

	IOnlineVoicePtr VoiceInt = OSS->GetVoiceInterface();
	if (!VoiceInt.IsValid())
	{
		UE_LOG(LogTemp, Verbose, TEXT("VoiceChatSubsystem: OSS '%s' has no voice interface (likely no active session yet)"),
			*OSS->GetSubsystemName().ToString());
		return;
	}

	const int32 LocalUserNum = LP->GetControllerId();

	// FOnlineVoiceImpl이 내부적으로 FLocalTalker::bIsRegistered로 중복 등록을 방지하므로
	// 여러 번 호출해도 안전. 맵 전환 시 OSS가 상태를 리셋할 수 있으므로 외부 플래그 없이
	// 매 BeginLocalTalk 및 PC BeginPlay마다 호출해 재등록을 보장한다.
	VoiceInt->RegisterLocalTalker(LocalUserNum);
}

void UVoiceChatSubsystem::SetTalkMode(EVoipMode NewMode)
{
	if (TalkMode == NewMode)
	{
		return;
	}

	TalkMode = NewMode;
	OnTalkModeChanged.Broadcast(TalkMode);

	if (TalkMode == EVoipMode::AutoVoice)
	{
		BeginLocalTalk();
	}
	else
	{
		EndLocalTalk();  // None, PushToTalk 모두 전송 중단
	}
}

void UVoiceChatSubsystem::BeginLocalTalk()
{
	if (bMicTestActive) return;
	// First-use registration: the OSS voice engine's OwningUserIndex must be set before
	// StartLocalVoiceProcessing can succeed. EasySessions + Steam login flow does not
	// trigger this automatically, so we do it lazily here.
	EnsureLocalTalkerRegistered();
	ApplyNetworkedVoice(true);
}

void UVoiceChatSubsystem::EndLocalTalk()
{
	ApplyNetworkedVoice(false);
}

void UVoiceChatSubsystem::SetRemotePlayerMuted(APlayerState* PS, bool bMuted)
{
	if (!PS)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	const FUniqueNetIdRepl& NetId = PS->GetUniqueId();
	if (!NetId.IsValid())
	{
		return;
	}

	if (bMuted)
	{
		MutedPlayers.Add(PS);
		PC->ServerMutePlayer(NetId);
	}
	else
	{
		MutedPlayers.Remove(PS);
		PC->ServerUnmutePlayer(NetId);
	}
}

bool UVoiceChatSubsystem::IsRemotePlayerMuted(APlayerState* PS) const
{
	return PS && MutedPlayers.Contains(PS);
}

void UVoiceChatSubsystem::SetRemotePlayerVolume(APlayerState* PS, float Volume)
{
	if (!PS)
	{
		return;
	}

	const float Clamped = FMath::Clamp(Volume, 0.0f, 2.0f);
	VolumeMap.Add(PS, Clamped);
	ApplyVolumeToTalker(PS);
}

float UVoiceChatSubsystem::GetRemotePlayerVolume(APlayerState* PS) const
{
	if (!PS)
	{
		return 1.0f;
	}
	if (const float* Found = VolumeMap.Find(PS))
	{
		return *Found;
	}
	return 1.0f;
}

void UVoiceChatSubsystem::ApplyVolumeToTalker(APlayerState* PS)
{
	if (!PS) return;

	float SendVolume = 1.0f;
	if (const ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(PS))
	{
		SendVolume = DPS->VoiceSendVolume;
	}

	const float ListenerOverride = GetRemotePlayerVolume(PS);
	constexpr float VoipBaselineBoost = 10.0f;
	const float FinalVolume = SendVolume * ListenerOverride * VoipBaselineBoost;

	const APawn* OwnerPawn = PS->GetPawn();
	if (!OwnerPawn) return;

	if (UTromboneVOIPTalker* Talker = OwnerPawn->FindComponentByClass<UTromboneVOIPTalker>())
	{
		Talker->SetVolumeMultiplier(FinalVolume);
	}
}

TArray<FString> UVoiceChatSubsystem::GetAvailableMicDeviceNames()
{
	TArray<Audio::FCaptureDeviceInfo> Devices;
	Audio::FAudioCapture TempCapture;
	TempCapture.GetCaptureDevicesAvailable(Devices);

	TArray<FString> Names;
	for (const Audio::FCaptureDeviceInfo& Device : Devices)
	{
		Names.Add(Device.DeviceName);
	}
	return Names;
}

void UVoiceChatSubsystem::ApplyMicrophoneSettings(int32 DeviceIndex, float VoiceSendVolume)
{
	// 실제 캡처 게인에 적용: VoiceCaptureWindows::ProcessData()가 이 CVar를 읽어
	// 인코딩 전 PCM 샘플에 곱한다. (복제되는 VoiceSendVolume은 청취자 재생 배수 전용)
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicInputGain")))
	{
		CVar->Set(FMath::Clamp(VoiceSendVolume, 0.0f, 2.0f), ECVF_SetByGameSetting);
	}

	const APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (ADefaultPlayerState* PS = PC->GetPlayerState<ADefaultPlayerState>())
	{
		PS->Server_SetVoiceSendVolume(VoiceSendVolume);
	}
}

void UVoiceChatSubsystem::BeginMicTest(int32 DeviceIndex)
{
	if (bMicTestActive)
	{
		EndMicTest();
	}

	const ULocalPlayer* LP = GetLocalPlayer();
	UWorld* World = LP ? LP->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	// 임시 액터를 스폰해 UAudioCaptureComponent를 소유시킴.
	// UAudioCaptureComponent는 USynthComponent를 상속하므로
	// 마이크 캡처 + 로컬 스피커 재생을 자동으로 처리한다.
	// 현재 UAudioCaptureComponent는 OS 기본 캡처 장치를 사용하므로
	// DeviceIndex 파라미터는 드롭다운 저장용으로만 사용된다.
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags = RF_Transient;
	AActor* TempActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
	if (!TempActor)
	{
		return;
	}

	MicCaptureComp = NewObject<UAmplifiedAudioCaptureComponent>(TempActor, UAmplifiedAudioCaptureComponent::StaticClass());
	TempActor->AddOwnedComponent(MicCaptureComp);
	MicCaptureComp->RegisterComponent();
	MicCaptureComp->Activate(true);
	MicCaptureComp->SetNoiseSuppression(bNoiseSuppressionEnabled);

	ApplyNetworkedVoice(false);
	bMicTestActive = true;
}

void UVoiceChatSubsystem::EndMicTest()
{
	if (!bMicTestActive)
	{
		return;
	}

	if (MicCaptureComp)
	{
		AActor* Owner = MicCaptureComp->GetOwner();
		MicCaptureComp = nullptr;
		if (Owner) Owner->Destroy();
	}

	bMicTestActive = false;

	if (TalkMode == EVoipMode::AutoVoice)
	{
		BeginLocalTalk();
	}
}

void UVoiceChatSubsystem::SetMicTestVolume(float Volume)
{
	// Volume 파라미터는 0~2000 범위의 퍼센트값. 100 = 1.0x (기본)
	if (bMicTestActive && MicCaptureComp)
	{
		MicCaptureComp->SetGain(Volume / 100.0f);
	}
}

void UVoiceChatSubsystem::SetNoiseSuppression(bool bEnabled)
{
	bNoiseSuppressionEnabled = bEnabled;

	// 실제 VoIP 경로: VoiceCaptureWindows::ProcessData()가 이 CVar를 읽어 noise gate를 적용.
	// OFF에서도 0이 아닌 최소 게이트를 유지해 무음 구간의 배경 잡음 송신(대역폭 낭비)을 막는다.
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicNoiseGateThreshold")))
	{
		CVar->Set(bEnabled ? NoiseSuppressionThreshold : NoiseGateBaseThreshold, ECVF_SetByGameSetting);
	}

	// 마이크 테스트 경로: AmplifiedAudioCaptureComponent에도 반영
	if (MicCaptureComp)
	{
		MicCaptureComp->SetNoiseSuppression(bEnabled);
	}
}

void UVoiceChatSubsystem::HandleWorldBeginTearDown(UWorld* World)
{
	const ULocalPlayer* LP = GetLocalPlayer();
	if (!World || !LP || World != LP->GetWorld() || !World->IsGameWorld())
	{
		return;
	}

	// VoipListenerSynthComponent(CreateVoiceSynthComponent)는 Outer 없이 /Engine/Transient에
	// 생성되고 소유 액터가 없어 UWorld::CleanupWorld가 UnregisterComponent를 해주지 않는다.
	// RemoteTalkerBuffers에 항목이 남아있는 동안은 FVoiceEngineImpl의 FVoiceSerializeHelper가
	// GC 참조를 잡고 있고, 엔진의 유일한 정리 경로(OnPostLoadMap -> FRemoteTalkerDataImpl::Reset)는
	// 새 맵 로드 이후 — 즉 옛 월드의 FScene::Release() 이후에나 실행된다.
	// 그 결과 해제된 FScene을 가리키는 오디오 컴포넌트가 트래블 이후까지 살아남아,
	// 새 월드의 첫 음성 패킷이 도착할 때 오디오 워커 스레드가 죽는다.
	//
	// 여기서 버퍼를 비우면 reachability 분석 시점에 컴포넌트가 unreachable이 되어,
	// 같은 full-purge GC 패스의 BeginDestroy 단계에서 스스로 unregister된다
	// (GC는 모든 unreachable의 BeginDestroy를 FinishDestroy보다 먼저 끝내도록 보장).
	//
	// 주의: RemoveAllRemoteTalkers는 Reset()이 아니라 Cleanup()을 호출하므로 직접 unregister하지
	// 않는다. 같은 FSeamlessTravelHandler::Tick 안에서 뒤따르는 full-purge GC에 의존한다.
	//
	// Online::GetVoiceInterface(World)를 쓰는 이유: IOnlineSubsystem::Get()과 달리 PIE 인스턴스
	// 매핑을 존중하므로, PIE에서 클라 1의 teardown이 클라 2의 talker를 지우지 않는다.
	if (IOnlineVoicePtr VoiceInt = Online::GetVoiceInterface(World))
	{
		VoiceInt->RemoveAllRemoteTalkers();
	}
}
