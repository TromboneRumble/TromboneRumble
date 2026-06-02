#include "Framework/DefaultPlayerState.h"
#include "Subsystems/VoiceChatSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Characters/TromboneCharacterBase.h"
#include "Framework/InGameState.h"
#include "Framework/LobbyGameState.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Pawns/MatchPawn.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Utilities/DebugHelper.h"

ADefaultPlayerState::ADefaultPlayerState()
{
	bReplicates = true;
}

void ADefaultPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnRhythmGameStateChanged.AddDynamic(this, &ThisClass::HandleRhythmGameStateChanged);
		RhythmSubsystem->OnNoteDetected.AddDynamic(this, &ThisClass::HandleNoteDetected);
		RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::HandleOnInstrumentPicked);
	}

	// 멀티플레이 환경에서 GameState가 늦게 바인딩 될 수 있음
	GetWorldTimerManager().SetTimer(TimerHandle_BindGameState, this, &ThisClass::TryBindGameState, 0.5f, true);
	// 혹시 이미 들어와 있을 수 있으니 즉시 1회 실행
	TryBindGameState();

}

void ADefaultPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
	}
	
	Super::EndPlay(EndPlayReason);
}


void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquippedWeaponClass);
	DOREPLIFETIME(ThisClass, SkinColor);
	DOREPLIFETIME(ThisClass, VoiceSendVolume);
	DOREPLIFETIME(ThisClass, CustomizationData);
}

void ADefaultPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	
	OnPlayerNameChanged.Broadcast(GetPlayerName());
}

void ADefaultPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	//서버에서 값이 복제되어왔을때는 숫자만 동기화
	OnLocalScoreChanged.Broadcast(this, 0, EScoreType::None);
}

void ADefaultPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (ADefaultPlayerState* DefaultPS = Cast<ADefaultPlayerState>(PlayerState))
	{
		DefaultPS->EquippedWeaponClass = this->EquippedWeaponClass;
		DefaultPS->SkinColor = this->SkinColor;
		DefaultPS->VoiceSendVolume = this->VoiceSendVolume;
		DefaultPS->CustomizationData = this->CustomizationData;
	}
}

void ADefaultPlayerState::AddScore(int32 Amount, EScoreType ScoreType)
{
	if (Amount == 0) return;

	// 로컬 점수 선행 계산 후 즉시 갱신
	const float NewScore = GetScore() + static_cast<float>(Amount);
	SetScore(NewScore);
	switch (ScoreType) {
		case EScoreType::RhythmScore:
			{
			CurrentScoreData.TotalScore = NewScore;
			}
			break;
		case EScoreType::BuffedTromboneScore:
		{
			CurrentScoreData.TotalScore = NewScore;
			CurrentScoreData.TromboneComboBuffScore += Amount;
		}
			break;
		case EScoreType::BuffedViolinScore:
			{
			CurrentScoreData.TotalScore = NewScore;
			CurrentScoreData.ViolinBuffScore += Amount;
			}
			break;
		case EScoreType::InstrumentPickedUp:
			{
			CurrentScoreData.OtherScore += Amount;
			CurrentScoreData.InstrumentStealCount++;
			}
			break;
		case EScoreType::OnHit:
			{
			CurrentScoreData.AttackScore += Amount;
			CurrentScoreData.HitCount++;
			}
			break;
		case EScoreType::CymbalsHit:
			{
			CurrentScoreData.AttackScore += Amount;
			CurrentScoreData.CymbalsAttackScore += Amount;
			CurrentScoreData.HitCount++;
			}
			break;
		case EScoreType::SpotLight:
			{
				CurrentScoreData.OtherScore += Amount;
				CurrentScoreData.SpotlightPickupCount++;
			}
			break;
		case EScoreType::None:
			break;
		case EScoreType::Invalid:
			break;
		
	}
	OnLocalScoreChanged.Broadcast(this, Amount, ScoreType);

	// 서버 동기화
	// Replication은 Server->Client로 이루어지기 때문에 호출 필요
	if (!HasAuthority())
	{
		Server_AddScore(Amount, ScoreType);
	}
}

void ADefaultPlayerState::Server_AddScore_Implementation(int32 Amount, EScoreType ScoreType)
{
	AddScore(Amount, ScoreType);
}

void ADefaultPlayerState::TryBindGameState()
{
	if (AGameStateBase* CurrentGameState = GetWorld()->GetGameState())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_BindGameState);

		if (AInGameState* InGameState = Cast<AInGameState>(CurrentGameState))
		{
			InGameState->OnInGameStateChanged.AddUniqueDynamic(this, &ThisClass::HandleInGameStateChanged);
		}
	}
}

void ADefaultPlayerState::HandleRhythmGameStateChanged(ERhythmGameState NewState)
{
	if (NewState == ERhythmGameState::Start)
	{
		CurrentScoreData.Reset();
	}
}

void ADefaultPlayerState::HandleNoteDetected(ENoteResult NoteResult)
{
	switch (NoteResult)
	{
	case ENoteResult::Bad:
		{
		CurrentScoreData.MissCount++;
		}
		break;
	case ENoteResult::Good:
		{
		CurrentScoreData.GoodCount++;
		}
		break;
	case ENoteResult::Excellent:
		{
		CurrentScoreData.ExcellentCount++;
		}
		break;
	}
}

void ADefaultPlayerState::HandleInGameStateChanged(EInGameState InGameState)
{
	if (InGameState != EInGameState::End) return;
#if !(UE_BUILD_SHIPPING)
	APlayerController* PlayerController = GetPlayerController();
	if  (PlayerController && PlayerController->IsLocalController())
	{
		// 파일 경로 및 이름 설정 (Saved/Logs/ScoreExports/PlayerName_Timestamp.txt)
		FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
		FString FileName = FString::Printf(TEXT("ScoreLog_%s_%s.txt"), *GetPlayerName(), *Timestamp);
		FString SavePath = FPaths::ProjectSavedDir() / TEXT("Logs/ScoreExports/") / FileName;

	
		FString LogContent = FString::Printf(TEXT("=== Trombone Rumble Score Report ===\n"));
		LogContent += FString::Printf(TEXT("Player: %s\n"), *GetPlayerName());
		LogContent += FString::Printf(TEXT("Date: %s\n"), *FDateTime::Now().ToString());
		LogContent += TEXT("-------------------------------------------\n");
		LogContent += FString::Printf(TEXT("Total Score: %.2f\n"), CurrentScoreData.TotalScore);
		LogContent += FString::Printf(TEXT("Excellent Count: %d / Good Count: %d / Miss Count: %d\n"),
			CurrentScoreData.ExcellentCount, CurrentScoreData.GoodCount, CurrentScoreData.MissCount);
		LogContent += TEXT("-------------------------------------------\n");
		LogContent += FString::Printf(TEXT("Trombone Buff Score: %.2f\n"), CurrentScoreData.TromboneComboBuffScore);
		LogContent += FString::Printf(TEXT("Violin Buff Score: %.2f\n"), CurrentScoreData.ViolinBuffScore);
		LogContent += FString::Printf(TEXT("Cymbals Attack Score: %.2f\n"), CurrentScoreData.CymbalsAttackScore);
		LogContent += TEXT("-------------------------------------------\n");
		LogContent += FString::Printf(TEXT("Total Attack Score: %.2f (Hits: %d)\n"), CurrentScoreData.AttackScore, CurrentScoreData.HitCount);
		LogContent += FString::Printf(TEXT("Instrument Steals: %d\n"), CurrentScoreData.InstrumentStealCount);
		LogContent += FString::Printf(TEXT("Spotlight Pickups: %d\n"), CurrentScoreData.SpotlightPickupCount);
		LogContent += TEXT("===========================================");

		
		if (FFileHelper::SaveStringToFile(LogContent, *SavePath))
		{
			Debug::Print(TEXT("Score log created! Check Saved/Logs/ScoreExports/"));
		}
	}
#endif
}

void ADefaultPlayerState::HandleOnInstrumentPicked(EInstrumentType PrevType, EInstrumentType NewType)
{
	auto IsRealInstrument = [](EInstrumentType Type) -> bool
		{
			const uint8 V = static_cast<uint8>(Type);
			const uint8 BG = static_cast<uint8>(EInstrumentType::Background);
			const uint8 NONE = static_cast<uint8>(EInstrumentType::None);
			// Background(0) < 실제 악기들(1~3) < None(254)
			return (V > BG) && (V < NONE);
		};

	// 이전에 악기를 들고있다가 떨궜을때 콤보 초기화
	if (IsRealInstrument(PrevType))
	{
		CurrentCombo = 0;
	}
}


void ADefaultPlayerState::HandleCombo(ENoteResult InResult)
{
	if (InResult == ENoteResult::Bad || InResult == ENoteResult::None)
	{
		CurrentCombo = 0;
	}
	else
	{
		++CurrentCombo;
	}

	OnComboChanged.Broadcast(InResult, CurrentCombo);
}

void ADefaultPlayerState::OnRep_SkinColor()
{
	if (APawn* Pawn = GetPawn())
	{
		if (const ATromboneCharacterBase* TromboneCharacter = Cast<ATromboneCharacterBase>(Pawn))
		{
			TromboneCharacter->ApplySkinColor(SkinColor);
		}
		if (const AMatchPawn* LobbyPawn = Cast<AMatchPawn>(Pawn))
		{
			LobbyPawn->UpdateSkinFromPlayerState();
		}
	}
}

void ADefaultPlayerState::OnRep_VoiceSendVolume()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue;

		ULocalPlayer* LP = PC->GetLocalPlayer();
		if (!LP) continue;

		if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
		{
			VCS->ApplyVolumeToTalker(this);
		}
	}
}

void ADefaultPlayerState::Server_SetVoiceSendVolume_Implementation(float Volume)
{
	VoiceSendVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
	OnRep_VoiceSendVolume();
}

void ADefaultPlayerState::OnRep_CustomizationData()
{
	APawn* Pawn = GetPawn();
	if (!Pawn) return;

	UCustomizationComponent* Comp = nullptr;
	if (AMatchPawn* MP = Cast<AMatchPawn>(Pawn))
		Comp = MP->CustomizationComp;
	else if (ATromboneCharacterBase* TC = Cast<ATromboneCharacterBase>(Pawn))
		Comp = TC->CustomizationComp;

	if (Comp)
		Comp->LoadFromSaveData(CustomizationData);
}

void ADefaultPlayerState::Server_SetCustomization_Implementation(FCustomizationSaveData InData)
{
	CustomizationData = InData;
	OnRep_CustomizationData();
}

void ADefaultPlayerState::SetSkinColor(const FLinearColor& InSkinColor)
{
	SkinColor = InSkinColor;
	OnRep_SkinColor();
}

bool ADefaultPlayerState::IsHost() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		return false;
	}

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	const FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!CurrentSession)
	{
		return false;
	}

	if (GetUniqueId().IsValid() && CurrentSession->OwningUserId.IsValid())
	{
		return *GetUniqueId() == *CurrentSession->OwningUserId;
	}

	return false;
}
