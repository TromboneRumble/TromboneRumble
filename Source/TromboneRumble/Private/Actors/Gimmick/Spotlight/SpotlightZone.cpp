#include "Actors/Gimmick/Spotlight/SpotlightZone.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Data/Gimmick/SpotlightGimmickConfig.h"
#include "Components/SphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/DefaultPlayerState.h"
#include "NiagaraFunctionLibrary.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/ActorComponents/ClientToServerRelayComponent.h"

ASpotlightZone::ASpotlightZone()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComponent"));
	SpotLightComponent->SetupAttachment(RootComponent);
	SpotLightComponent->SetIntensity(100000.0f);
	SpotLightComponent->SetAttenuationRadius(1500.0f);
	SpotLightComponent->SetInnerConeAngle(12.5f);
	SpotLightComponent->SetOuterConeAngle(25.0f);
	SpotLightComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); 
	SpotLightComponent->SetVisibility(false);
	
	LightBeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightBeamMesh"));
	LightBeamMesh->SetupAttachment(RootComponent);
	LightBeamMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	LightBeamMesh->SetCastShadow(false);
	LightBeamMesh->SetVisibility(false);

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	if (AkComponent)
	{
		AkComponent->OcclusionRefreshInterval = 0.f;
		AkComponent->SetupAttachment(RootComponent);
	}
	

	bReplicates = true;
	AActor::SetReplicateMovement(false);
}

void ASpotlightZone::InitializeZone(const bool bIsFeverTime, const int32 InSpotlightBonusScore)
{
	if (!HasAuthority()) return;
	SpotlightBonusScore = InSpotlightBonusScore;
	bSpawnedInFever = bIsFeverTime;
	StartPhase(0);
}

void ASpotlightZone::HandleServerRPC(ACharacter* InstigatorCharacter)
{
	if (!HasAuthority() || !InstigatorCharacter) return;
	if (!TriggerVolume->IsOverlappingActor(InstigatorCharacter)) return;
	if (CurrentState != ESpotlightState::Active) return;
	if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(InstigatorCharacter))
	{
		TryAwardBonus(TromboneCharacter);

		// A success turns the light off early, so the zone jumps to its fading phase. Without one it goes at once
		TArray<FGimmickPhase> Phases;
		GetConfig().GetZonePhases(bSpawnedInFever, Phases);
		StartPhase(Phases.IndexOfByPredicate([](const FGimmickPhase& Phase) { return Phase.Phase == EGimmickTimelinePhase::Ending; }));
	}
}


void ASpotlightZone::BeginPlay()
{
	Super::BeginPlay();
	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleTriggerBeginOverlap);
		TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleTriggerEndOverlap);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URhythmSubsystem* RhythmSubsystem = GameInstance->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnNoteDetected.AddDynamic(this, &ThisClass::HandleOnNoteDetected);
		}
	}
}

void ASpotlightZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(LifecycleTimerHandle);
		LifecycleTimerHandle.Invalidate();
	}
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URhythmSubsystem* RhythmSubsystem = GameInstance->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnNoteDetected.RemoveDynamic(this, &ThisClass::HandleOnNoteDetected);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void ASpotlightZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentState);
	DOREPLIFETIME(ThisClass, bIsBonusAwarded);
	DOREPLIFETIME(ThisClass, SpotlightBonusScore);
}

void ASpotlightZone::HandleTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ADefaultTromboneCharacter* Character = Cast<ADefaultTromboneCharacter>(OtherActor))
	{
		// 이 클라 기준 로컬 플레이어만 기억
		if (Character->IsLocallyControlled())
		{
			bIsLocalPlayerOverlapping = true;
		}
	}
}

void ASpotlightZone::HandleTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ADefaultTromboneCharacter* Character = Cast<ADefaultTromboneCharacter>(OtherActor))
	{
		if (Character->IsLocallyControlled())
		{
			bIsLocalPlayerOverlapping = false;
		}
	}
}

void ASpotlightZone::HandleOnNoteDetected(ENoteResult NoteResult)
{

	if (NoteResult == ENoteResult::None ||
		NoteResult == ENoteResult::Bad ||
		NoteResult == ENoteResult::Invalid)
	{
		return;
	}

	if (!bIsLocalPlayerOverlapping || CurrentState != ESpotlightState::Active || bIsBonusAwarded)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) return;

	ADefaultTromboneCharacter* LocalCharacter = Cast<ADefaultTromboneCharacter>(PlayerController->GetPawn());
	if (!LocalCharacter) return;

	if (ADefaultPlayerState* PS = LocalCharacter->GetPlayerState<ADefaultPlayerState>())
	{
		PS->AddScore(SpotlightBonusScore, EScoreType::SpotLight);
		OnSpotlightBonusEarned.Broadcast();
	}

	if (HasAuthority())
	{
		if (PlayerController->IsLocalController())
		{
			HandleServerRPC(LocalCharacter);
		}
	}
	else
	{
		//서버의 응답이 오기 전에 중복 획득하는 것을 막기 위해 로컬에서 미리 획득 처리
		bIsBonusAwarded = true;
		if (UClientToServerRelayComponent* Relay = LocalCharacter->GetClientToServerRelayComponent())
		{
			Relay->Server_SendRPCRequest(this);
		}
	}
}

void ASpotlightZone::Multicast_PlaySpotlightTurnOnSFX_Implementation()
{
	if (SpotLightTurnOnSFX && AkComponent)
	{
		AkComponent->PostAkEvent(
			SpotLightTurnOnSFX,
			0,
			FOnAkPostEventCallback()
		);
	}
}

void ASpotlightZone::Multicast_PlaySpotlightSuccessEffect_Implementation(ADefaultTromboneCharacter* InPlayer)
{
	if (!SpotlightSuccessVFX || !IsValid(InPlayer))
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		SpotlightSuccessVFX,
		SpawnLocation,
		FRotator::ZeroRotator,
		FVector(1.f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	if (SpotlightSuccessSFX && AkComponent)
	{
		AkComponent->PostAkEvent(SpotlightSuccessSFX,0,FOnAkPostEventCallback());
	}
}



void ASpotlightZone::StartPhase(const int32 PhaseIndex)
{
	if (!HasAuthority()) return;

	// Read again for each phase, so a value edited during play counts from the next phase
	TArray<FGimmickPhase> Phases;
	GetConfig().GetZonePhases(bSpawnedInFever, Phases);
	if (!Phases.IsValidIndex(PhaseIndex))
	{
		Destroy();
		return;
	}

	switch (Phases[PhaseIndex].Phase)
	{
	case EGimmickTimelinePhase::Warning:
		SetState(ESpotlightState::Warning);
		break;
	case EGimmickTimelinePhase::Active:
		SetState(ESpotlightState::Active);
		break;
	default:
		SetState(ESpotlightState::Fading);
		break;
	}

	AGimmickBase::StartPhaseTimer(GetWorldTimerManager(), LifecycleTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::StartPhase, PhaseIndex + 1), Phases[PhaseIndex].Duration);
}

void ASpotlightZone::SetState(ESpotlightState NewState)
{
	if (!HasAuthority()) return;
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	OnRep_CurrentState();

	switch (CurrentState)
	{
		case ESpotlightState::Active:
			Multicast_PlaySpotlightTurnOnSFX();
			break;

		case ESpotlightState::Fading:
			TurnOffLight();
			break;

		default:
			break;
	}
}

bool ASpotlightZone::TryAwardBonus(ADefaultTromboneCharacter* InCharacter)
{
	if (!HasAuthority() || !InCharacter) return false;

	if (CurrentState == ESpotlightState::Active && !bIsBonusAwarded)
	{
		bIsBonusAwarded = true;

		Multicast_PlaySpotlightSuccessEffect(InCharacter);
		return true;
	}

	return false;
}

void ASpotlightZone::OnRep_CurrentState()
{
	switch (CurrentState)
	{
		case ESpotlightState::Warning:
			SpotLightComponent->SetVisibility(true);
			LightBeamMesh->SetVisibility(false);

			break;
		
		case ESpotlightState::Active:
			SpotLightComponent->SetVisibility(true);
			SpotLightComponent->SetLightColor(SpotlightActiveColor);
			LightBeamMesh->SetVisibility(true);
			break;

	case ESpotlightState::Fading:
			TurnOffLight();
		default:
			break;
	}
}

const USpotlightGimmickConfig& ASpotlightZone::GetConfig() const
{
	// The zone is not a gimmick itself, so it looks the entry up in the world. This works on clients too
	const USpotlightGimmickConfig* Config = Cast<USpotlightGimmickConfig>(AGimmickManager::FindConfigInWorld(GetWorld(), EGimmickType::Spotlight));
	return Config ? *Config : *GetDefault<USpotlightGimmickConfig>();
}
