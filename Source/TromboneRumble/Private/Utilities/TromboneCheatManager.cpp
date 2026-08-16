#include "Utilities/TromboneCheatManager.h"
#include "Actors/Gimmick/Garbage/GarbageSpawner.h"
#include "Actors/Gimmick/Spotlight/SpotlightManager.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Components/ActorComponents/XRayComponentBase.h"
#include "Components/ActorComponents/XRayTranslucentFadeComponent.h"
#include "Components/ActorComponents/XRaySilhouetteComponent.h"
#include "Components/ActorComponents/XRayWindowComponent.h"
#include "Utilities/XRayBenchmark.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/DefaultPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Subsystems/ResultSceneSubsystem.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"
#include "Utilities/EnumHelper.h"

void UTromboneCheatManager::Trombone_Help()
{
	FString DebugMsg;
	DebugMsg += TEXT("사용 가능한 명령어:\n");
	DebugMsg += TEXT("Trombone_SpawnInstrument [InstrumentType] - 스폰할 악기 타입을 입력하여 악기를 소환합니다. (예: Trombone_SpawnInstrument Violin)\n");
	DebugMsg += TEXT("Trombone_Spotlight - 스포트라이트를 소환합니다.\n");
	DebugMsg += TEXT("Trombone_Throw [Count] - 쓰레기를 소환합니다. Count는 소환할 쓰레기의 수입니다. (예: Trombone_Throw 10)\n");
	DebugMsg += TEXT("Trombone_Ragdoll - 래그돌을 실행합니다.\n");
	DebugMsg += TEXT("Trombone_Stun - 스턴을 실행합니다.\n");
	DebugMsg += TEXT("Trombone_ResetSettingData - 설정 데이터 초기화\n");
	DebugMsg += TEXT("Trombone_SetCustomization [AntennaKey] [FaceKey] [CostumeKey] - 커스터마이징 즉시 변경 및 복제 (None=기본값, 예: Trombone_SetCustomization None Face_02 None)\n");
	DebugMsg += TEXT("Trombone_XRayMode [silhouette|dither|window|off] - X-Ray 방식 전환. 인자 없으면 현재 붙어있는 컴포넌트 출력\n");
	DebugMsg += TEXT("Trombone_XRayBench [초] - X-Ray 3종의 프레임 비용을 순서대로 측정해 비교 (기본 10초씩. 가려진 자리에 서서 실행)\n");
	DebugMsg += TEXT("Trombone_XRayWindow [반경cm] [부드러움0~1] [가리는물체투명도0~1] - 원형 윈도우 런타임 조절. 인자 없으면 현재 값 출력\n");
	DebugMsg += TEXT("Trombone_XRayCropCapture [0|1] - 원형 윈도우의 캡처 시야를 원 주변으로 좁힐지. 인자 없으면 현재 값 출력\n");
	DebugMsg += TEXT("Trombone_AudioOffset [ms] - 리듬 BGM 오프셋(양수=일찍 시작). 인자 없으면 현재 값 출력\n");
	DebugMsg += TEXT("Trombone_RhythmSyncLog [0|1] - 리듬 싱크 실측 로그 + 음악 클럭 화면 표시. 인자 없으면 현재 값 출력\n");
	DebugMsg += TEXT("Trombone_ResultTest [인원수] [스테이지] - 더미 플레이어로 결과 씬 확인 (기본 4명 / OrchestraStage, 예: Trombone_ResultTest 3 SnowField)\n");
	DebugMsg += TEXT("--------------------------------\n");
	DebugMsg += TEXT("스폰 가능한 악기 타입 목록 :\n");
	DebugMsg += TEXT("Trombone, Violin, Cymbal\n");
	DebugMsg += TEXT("--------------------------------\n");
	DebugMsg += TEXT("대소문자는 상관없습니다.\n");

	PRINT_WITH_CURRENT_CONTEXT(DebugMsg);
}

void UTromboneCheatManager::Trombone_SpawnInstrument(const FString& TypeString)
{
	if (TypeString.IsEmpty())
	{
		const UEnum* EnumPtr = StaticEnum<EWeaponType>();
		FString AvailableTypes = TEXT("Available Types: ");
        
		for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i)
		{
			AvailableTypes += EnumPtr->GetNameStringByIndex(i);
			if (i < EnumPtr->NumEnums() - 2) AvailableTypes += TEXT(", ");
		}
        
		PRINT_WITH_CURRENT_CONTEXT(AvailableTypes);
		return;
	}
	
	EWeaponType Type;
	if (!EnumHelper::StringToEnum<EWeaponType>(TypeString, Type))
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("악기 타입이 잘못되었습니다: %s"), *TypeString));
		PRINT_WITH_CURRENT_CONTEXT(TEXT("가능한 악기 타입: Trombone, Violin, Cymbals"));
		return;
	}
	
	APawn* MyPawn = GetOuterAPlayerController()->GetPawn();
	if (!MyPawn) return;
	
	const UTromboneConfig* Config = UTromboneConfig::Get();
	if (Config->InstrumentClasses.Num() == 0)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("No weapon classes assigned in Project Settings. Assign weapon classes"));
		return;
	}

	if (!Config->InstrumentClasses.Contains(Type))
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Weapon class %s not assigned in CheatManager. talk to developer."), *TypeString));
		return;
	}

	FVector SpawnLocation = MyPawn->GetActorLocation() + FVector(0.f, 0.f, 200.f);
	FRotator SpawnRotation = MyPawn->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = MyPawn;
	SpawnParams.Instigator = MyPawn;

	if (GetWorld()->SpawnActor<AActor>(Config->InstrumentClasses[Type].LoadSynchronous(), SpawnLocation, SpawnRotation, SpawnParams))
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("%s 스폰"), *TypeString));
	}
}

void UTromboneCheatManager::Trombone_Spotlight()
{
	if (ASpotlightManager* Manager = Cast<ASpotlightManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASpotlightManager::StaticClass())))
	{
		Manager->Server_TriggerAllSpotlightSpawn();
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Triggered All Spotlights"));
	}
}

void UTromboneCheatManager::Trombone_Throw(const FString& Count)
{
	const int32 CountInt = FCString::Atoi(*Count);

	if (CountInt >= 100)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("렉 걸려요. 100번 이상은 하지마세요 ㅎㅎ"));
		return;
	}
	
	if (AGarbageSpawner* Spawner = Cast<AGarbageSpawner>(UGameplayStatics::GetActorOfClass(GetWorld(), AGarbageSpawner::StaticClass())))
	{
		Spawner->Server_SpawnGarbageForDebugging(CountInt);
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Garbage Spawned %s times"), *Count));
	}
}

void UTromboneCheatManager::Trombone_Ragdoll()
{
	APawn* MyPawn = GetOuterAPlayerController()->GetPawn();
	if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(MyPawn))
	{
		TromboneCharacter->Server_DebugRagdoll();
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Ragdoll executed"));
	}
}

void UTromboneCheatManager::Trombone_Stun()
{
	APawn* MyPawn = GetOuterAPlayerController()->GetPawn();
	if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(MyPawn))
	{
		TromboneCharacter->Server_DebugStun();
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Stun executed"));
	}
}

void UTromboneCheatManager::Trombone_AudioOffset(const FString& MsString)
{
	const UWorld* World = GetWorld();
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	USaveManagerSubsystem* Subsystem = GI ? GI->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	if (!Subsystem) return;

	FAudioSettingData Data = Subsystem->GetAudioSettings();
	if (MsString.IsEmpty())
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("리듬 오디오 오프셋: %d ms (양수 = BGM을 그만큼 일찍 시작)"), Data.RhythmAudioOffsetMs));
		return;
	}

	Data.RhythmAudioOffsetMs = FMath::Clamp(FCString::Atoi(*MsString), -500, 1000);
	Subsystem->ApplyAudio(Data, true);
	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("리듬 오디오 오프셋 %d ms 저장 - 다음 곡부터 적용"), Data.RhythmAudioOffsetMs));
}

void UTromboneCheatManager::Trombone_RhythmSyncLog(const FString& EnabledString)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Trombone.Rhythm.SyncLog"));
	if (!CVar) return;

	if (EnabledString.IsEmpty())
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("리듬 싱크 로그 - 현재 %s"), CVar->GetInt() != 0 ? TEXT("on") : TEXT("off")));
		return;
	}

	const bool bEnabled = EnabledString.ToBool() || EnabledString.Equals(TEXT("on"), ESearchCase::IgnoreCase);
	CVar->Set(bEnabled ? 1 : 0);
	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("리듬 싱크 로그 %s"), bEnabled ? TEXT("on") : TEXT("off")));
}

void UTromboneCheatManager::Trombone_ResetSettingData()
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (USaveManagerSubsystem* Subsystem = GI->GetSubsystem<USaveManagerSubsystem>())
			{
				Subsystem->ResetToDefaultSettings();
				PRINT_WITH_CURRENT_CONTEXT(TEXT("Tutorial data reset"));
			}
		}
	}
}


void UTromboneCheatManager::Trombone_SetCustomization(const FString& AntennaKey, const FString& FaceKey, const FString& CostumeKey)
{
	APlayerController* PC = GetOuterAPlayerController();
	if (!PC) return;

	UCustomizationComponent* Comp = PC->GetPawn()
		? PC->GetPawn()->FindComponentByClass<UCustomizationComponent>()
		: nullptr;
	if (!Comp)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("CustomizationComponent 없음 — MatchMenuMap에서 실행하세요"));
		return;
	}

	auto ToKey = [](const FString& S) -> FName
	{
		return (S.IsEmpty() || S.Equals(TEXT("None"), ESearchCase::IgnoreCase)) ? NAME_None : FName(*S);
	};

	FCustomizationSaveData Data;
	Data.AntennaKey = ToKey(AntennaKey);
	Data.FaceKey    = ToKey(FaceKey);
	Data.CostumeKey = ToKey(CostumeKey);

	Comp->LoadFromSaveData(Data);

	if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
		DPS->Server_SetCustomization(Data);

	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Customization set — Antenna:%s Face:%s Costume:%s"), *AntennaKey, *FaceKey, *CostumeKey));
}

void UTromboneCheatManager::Trombone_XRayMode(const FString& ModeString)
{
	const APlayerController* PC = GetOuterAPlayerController();
	if (!PC) return;

	// 벤치가 들고 있는 컴포넌트를 파괴하면 측정이 꼬인다
	if (XRayBenchmark && XRayBenchmark->IsRunning())
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("측정 중에는 X-Ray를 전환할 수 없습니다. 측정이 끝난 뒤 다시 시도하세요."));
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("조종 중인 폰이 없습니다 — 인게임에서 실행하세요"));
		return;
	}

	auto DescribeAttached = [Pawn]() -> FString
	{
		TArray<UXRayComponentBase*> Components;
		Pawn->GetComponents<UXRayComponentBase>(Components);
		if (Components.Num() == 0)
		{
			return TEXT("(없음)");
		}
		FString Result;
		for (const UXRayComponentBase* Component : Components)
		{
			if (!Result.IsEmpty()) Result += TEXT(", ");
			Result += Component->GetClass()->GetName();
			if (Component->IsEffectActive())
			{
				Result += TEXT("(활성)");
			}
			else
			{
				// 정지 상태와 "머티리얼이 없어 못 켜진 상태"를 구분해줘야 원인을 찾을 수 있다
				Result += Component->IsTrackingPaused() ? TEXT("(정지)") : TEXT("(머티리얼 없음)");
			}
		}
		return Result;
	};

	if (ModeString.IsEmpty())
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(
			TEXT("현재 X-Ray: %s  (전환: Trombone_XRayMode silhouette|dither|window|off)"), *DescribeAttached()));
		return;
	}

	TSubclassOf<UXRayComponentBase> NewClass = nullptr;
	if (ModeString.Equals(TEXT("silhouette"), ESearchCase::IgnoreCase))
	{
		NewClass = UXRaySilhouetteComponent::StaticClass();
	}
	else if (ModeString.Equals(TEXT("dither"), ESearchCase::IgnoreCase))
	{
		NewClass = UXRayTranslucentFadeComponent::StaticClass();
	}
	else if (ModeString.Equals(TEXT("window"), ESearchCase::IgnoreCase))
	{
		NewClass = UXRayWindowComponent::StaticClass();
	}
	else if (!ModeString.Equals(TEXT("off"), ESearchCase::IgnoreCase))
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("알 수 없는 모드입니다. silhouette | dither | window | off"));
		return;
	}

	// BP 컴포넌트는 파괴하지 않고 재우기만 한다 — 파괴하면 BP에 지정된 머티리얼/튜닝값을 잃는다
	static const FName CheatTempTag(TEXT("XRayCheatTemp"));
	UXRayComponentBase* Target = nullptr;

	TArray<UXRayComponentBase*> Existing;
	Pawn->GetComponents<UXRayComponentBase>(Existing);
	for (UXRayComponentBase* Component : Existing)
	{
		if (Component->ComponentHasTag(CheatTempTag))
		{
			Component->DestroyComponent();	// 이전 치트가 만든 임시 컴포넌트는 정리
			continue;
		}

		if (NewClass && Component->GetClass() == NewClass)
		{
			Target = Component;
			Target->SetTrackingPaused(false);	// 이미 켜져 있으면 아무 일도 하지 않는다
			continue;
		}

		Component->SetTrackingPaused(true);
	}

	if (NewClass && !Target)
	{
		// BP에 없는 방식은 임시로 만든다. 등록하면 BeginPlay가 돌아 스스로 켜진다
		// (머티리얼이 필요한 방식은 초기화에 실패해 잠든 채로 남는다)
		Target = NewObject<UXRayComponentBase>(Pawn, NewClass);
		Target->ComponentTags.Add(CheatTempTag);
		Target->RegisterComponent();
	}

	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("X-Ray → %s"), *DescribeAttached()));
}

void UTromboneCheatManager::Trombone_XRayBench(const FString& SecondsString)
{
	const APlayerController* PC = GetOuterAPlayerController();
	if (!PC) return;

	if (!XRayBenchmark)
	{
		XRayBenchmark = NewObject<UXRayBenchmark>(this);
	}

	const float Seconds = SecondsString.IsEmpty() ? 10.f : FCString::Atof(*SecondsString);
	XRayBenchmark->Start(PC->GetPawn(), Seconds);
}

void UTromboneCheatManager::Trombone_XRayWindow(const FString& RadiusString, const FString& SoftnessString, const FString& OccluderOpacityString)
{
	const APlayerController* PC = GetOuterAPlayerController();
	if (!PC) return;

	// 측정 중에 모양을 바꾸면 방식 간 비교가 깨진다
	if (XRayBenchmark && XRayBenchmark->IsRunning())
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("측정 중에는 윈도우를 조절할 수 없습니다. 측정이 끝난 뒤 다시 시도하세요."));
		return;
	}

	APawn* Pawn = PC->GetPawn();
	UXRayWindowComponent* Window = Pawn ? Pawn->FindComponentByClass<UXRayWindowComponent>() : nullptr;
	if (!Window)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("원형 윈도우 컴포넌트가 없습니다. Trombone_XRayMode window 로 먼저 켜세요."));
		return;
	}

	// 인자를 생략한 항목은 현재 값을 유지한다 (반경만 훑고 싶을 때가 많다)
	const float Radius = RadiusString.IsEmpty()
		? Window->GetWorldHoleRadius()
		: FCString::Atof(*RadiusString);
	const float Softness = SoftnessString.IsEmpty()
		? Window->GetEdgeSoftness()
		: FCString::Atof(*SoftnessString);
	const float OccluderOpacity = OccluderOpacityString.IsEmpty()
		? Window->GetOccluderOpacity()
		: FCString::Atof(*OccluderOpacityString);

	Window->SetWindowShape(Radius, Softness, OccluderOpacity);

	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(
		TEXT("X-Ray 윈도우 — WorldHoleRadius %.1f / EdgeSoftness %.2f / OccluderOpacity %.2f  (저장 안 됨. 마음에 들면 BP의 XRay|Window에 옮겨 적으세요)"),
		Window->GetWorldHoleRadius(), Window->GetEdgeSoftness(), Window->GetOccluderOpacity()));
}

void UTromboneCheatManager::Trombone_XRayCropCapture(const FString& EnabledString)
{
	const APlayerController* PC = GetOuterAPlayerController();
	if (!PC) return;

	// 측정 중에 캡처 방식을 바꾸면 구간별 비교가 깨진다
	if (XRayBenchmark && XRayBenchmark->IsRunning())
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("측정 중에는 바꿀 수 없습니다. 측정이 끝난 뒤 다시 시도하세요."));
		return;
	}

	APawn* Pawn = PC->GetPawn();
	UXRayWindowComponent* Window = Pawn ? Pawn->FindComponentByClass<UXRayWindowComponent>() : nullptr;
	if (!Window)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("원형 윈도우 컴포넌트가 없습니다. Trombone_XRayMode window 로 먼저 켜세요."));
		return;
	}

	if (EnabledString.IsEmpty())
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(
			TEXT("X-Ray 윈도우 캡처 크롭 — 현재 %s"), Window->IsCropCaptureEnabled() ? TEXT("on") : TEXT("off")));
		return;
	}

	const bool bEnabled = EnabledString.ToBool() || EnabledString.Equals(TEXT("on"), ESearchCase::IgnoreCase);
	Window->SetCropCaptureEnabled(bEnabled);

	// 렌더타겟 크기가 방식마다 달라서 재초기화가 필요하다
	Window->SetTrackingPaused(true);
	Window->SetTrackingPaused(false);

	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(
		TEXT("X-Ray 윈도우 캡처 크롭 %s (저장 안 됨. BP의 XRay|Window > bCropCaptureToWindow가 기본값)"),
		bEnabled ? TEXT("on — 원 주변만 캡처") : TEXT("off — 화면 전체 캡처")));
}

void UTromboneCheatManager::Trombone_ResultTest(const FString& PlayerCountString, const FString& StageString)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance())
	{
		return;
	}

	UResultSceneSubsystem* ResultSubsystem = World->GetGameInstance()->GetSubsystem<UResultSceneSubsystem>();
	if (!ResultSubsystem)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("ResultSceneSubsystem을 찾을 수 없습니다"));
		return;
	}

	const int32 PlayerCount = PlayerCountString.IsEmpty()
		? 4
		: FMath::Clamp(FCString::Atoi(*PlayerCountString), 1, 8);

	const FString Stage = StageString.IsEmpty() ? TEXT("OrchestraStage") : StageString;

	// 스테이지 이름이 틀리면 ResolveResultMapTag가 조용히 오케스트라로 넘어가므로 여기서 먼저 걸러낸다
	const FString CandidateStr = TromboneGamePlayTags::ResultPath + TEXT(".") + Stage;
	if (!FGameplayTag::RequestGameplayTag(FName(*CandidateStr), false).IsValid())
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("알 수 없는 스테이지: %s (OrchestraStage | SnowField | JazzBar)"), *Stage));
		return;
	}

	ResultSubsystem->SetDebugResultSceneData(PlayerCount, Stage);
	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("더미 결과 %d명 / %s — 결과 레벨로 이동합니다"), PlayerCount, *Stage));

	ResultSubsystem->OpenResultLevel(World);
}

void UTromboneCheatManager::Trombone_Dump_LevelStateSubsystem()
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (const UGameStateSubsystem* Subsystem = GI->GetSubsystem<UGameStateSubsystem>())
			{
				Subsystem->DumpSettings();
			}
		}
	}
}
