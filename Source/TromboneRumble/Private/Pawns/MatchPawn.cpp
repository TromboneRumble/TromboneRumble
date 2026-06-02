#include "Pawns/MatchPawn.h"
#include "UI/UserWidgets/Lobby/VoiceVolumeRowWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Components/ActorComponents/NameplateComponent.h"
#include "Components/ActorComponents/TromboneVOIPTalker.h"
#include "Engine/LocalPlayer.h"
#include "Framework/DefaultPlayerState.h"
#include "Framework/GameState/MatchMenuGameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Materials/MaterialInterface.h"
#include "Subsystems/SaveManagerSubsystem.h"

AMatchPawn::AMatchPawn()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionObjectType(ECC_Pawn);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	RootComponent = CapsuleComponent;
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	
	NameplateComponent = CreateDefaultSubobject<UNameplateComponent>(TEXT("NameplateComponent"));
	CustomizationComp = CreateDefaultSubobject<UCustomizationComponent>(TEXT("CustomizationComponent"));

	// Voice chat: 2D (omnidirectional) playback for lobby pawns.
	VOIPTalker = CreateDefaultSubobject<UTromboneVOIPTalker>(TEXT("VOIPTalker"));
	if (VOIPTalker)
	{
		VOIPTalker->bPositional = false;
	}

	SpeakerIndicatorComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SpeakerIndicatorComponent"));
	if (SpeakerIndicatorComponent)
	{
		SpeakerIndicatorComponent->SetupAttachment(SkeletalMeshComponent, FName("head"));
		SpeakerIndicatorComponent->SetWidgetSpace(EWidgetSpace::Screen);
		SpeakerIndicatorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SpeakerIndicatorComponent->bReceivesDecals = 0;
		SpeakerIndicatorComponent->SetCastShadow(false);
		SpeakerIndicatorComponent->SetVisibility(true);
		SpeakerIndicatorComponent->SetHiddenInGame(false);
	}

	VoiceSliderComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("VoiceSliderComponent"));
	if (VoiceSliderComponent)
	{
		VoiceSliderComponent->SetupAttachment(SkeletalMeshComponent, FName("head"));
		VoiceSliderComponent->SetWidgetSpace(EWidgetSpace::Screen);
		VoiceSliderComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		VoiceSliderComponent->bReceivesDecals = 0;
		VoiceSliderComponent->SetCastShadow(false);
	}

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetupAttachment(CapsuleComponent);
	}
#endif

	// Disable movement replication by default because we most likely want to relocate lobby pawns after spawning.
	AActor::SetReplicateMovement(false);
}

void AMatchPawn::ApplyFaceMaterial(UMaterialInterface* Material)
{
	UMaterialInterface* Target = Material ? Material : OriginalFaceMaterial.Get();
	if (!Target || !SkeletalMeshComponent) return;

	SkeletalMeshComponent->SetMaterial(FaceMaterialIndex, Target);
	FaceMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(FaceMaterialIndex);
	if (FaceMID)
	{
		if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
		{
			FaceMID->SetVectorParameterValue(TEXT("BaseColor"), DPS->GetSkinColor());
		}
	}
}

void AMatchPawn::UpdateSkinFromPlayerState() const
{
	if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
	{
		const FLinearColor SkinColor = DPS->GetSkinColor();
		if (SkinMID)
		{
			SkinMID->SetVectorParameterValue(TEXT("BaseColor"), SkinColor);
		}
		if (FaceMID)
		{
			FaceMID->SetVectorParameterValue(TEXT("BaseColor"), SkinColor);
		}
	}
}

void AMatchPawn::BeginPlay()
{
	// 실행 순서:
	// 1) MID 초기화  2) LoadFromSaveData (저장 데이터 적용)
	// 3) Super::BeginPlay() → ReceiveBeginPlay() (Blueprint BeginPlay) 실행
	//    개발자가 BP에서 SetPartByKey/StepPart를 호출하면 저장 데이터를 덮어써서 디버깅 가능
	if (UMaterialInterface* CurrentSkinMat = SkeletalMeshComponent->GetMaterial(SkinMaterialIndex))
	{
		SkinMID = Cast<UMaterialInstanceDynamic>(CurrentSkinMat);
		if (!SkinMID)
		{
			SkinMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(SkinMaterialIndex);
		}
	}
	if (UMaterialInterface* CurrentFaceMat = SkeletalMeshComponent->GetMaterial(FaceMaterialIndex))
	{
		OriginalFaceMaterial = CurrentFaceMat;
		FaceMID = Cast<UMaterialInstanceDynamic>(CurrentFaceMat);
		if (!FaceMID)
		{
			FaceMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(FaceMaterialIndex);
		}
	}

	if (CustomizationComp)
	{
		FCustomizationSaveData SaveData;
		if (IsLocallyControlled())
		{
			if (USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
				SaveData = SMS->LoadCustomization();
			CustomizationComp->LoadFromSaveData(SaveData);
			if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
				DPS->Server_SetCustomization(SaveData);
		}
		else
		{
			if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
				SaveData = DPS->GetCustomizationData();
			CustomizationComp->LoadFromSaveData(SaveData);
		}
	}

	Super::BeginPlay();

	TryRegisterVOIPTalker();

	// 멀티플레이 환경에서 UI 생성 및 Owner설정
	if (SpeakerIndicatorComponent)
	{
		if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = LocalPC->GetLocalPlayer())
			{
				SpeakerIndicatorComponent->SetOwnerPlayer(LocalPlayer);
			}
		}

		SpeakerIndicatorComponent->InitWidget();
		if (UUserWidget* UW = SpeakerIndicatorComponent->GetUserWidgetObject())
		{
			UW->SetVisibility(ESlateVisibility::HitTestInvisible);
			UW->SetRenderOpacity(0.0f);
		}
	}

	if (AMatchMenuGameState* GameState = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		GameState->HandleMatchPawnCreated(this);
	}

	// Apply a skin color in server side
	UpdateSkinFromPlayerState();
}

void AMatchPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RetryVOIPRegistrationHandle);

	if (AMatchMenuGameState* GameState = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		GameState->HandleMatchPawnPreDestroyed(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AMatchPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	TryRegisterVOIPTalker();

	// Apply a skin color in client side
	UpdateSkinFromPlayerState();

	if (IsLocallyControlled())
	{
		// BeginPlay 시점에 PlayerState가 null이었을 경우의 fallback
		if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
			if (USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
				DPS->Server_SetCustomization(SMS->LoadCustomization());
	}
	else if (CustomizationComp)
	{
		// 타 플레이어: OnRep_CustomizationData 미도착 시 즉시 적용
		if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
			CustomizationComp->LoadFromSaveData(DPS->GetCustomizationData());
	}
}

void AMatchPawn::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (!IsLocallyControlled()) return;
	USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	if (!SMS) return;

	FCustomizationSaveData SaveData = SMS->LoadCustomization();
	if (CustomizationComp)
		CustomizationComp->LoadFromSaveData(SaveData);
	if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
		DPS->Server_SetCustomization(SaveData);
}

void AMatchPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server쪽에서 OnRep_PlayerState가 호출되지 않기 때문에 PossessedBy에서 호출
	TryRegisterVOIPTalker();

	if (CustomizationComp)
		if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
			CustomizationComp->LoadFromSaveData(DPS->GetCustomizationData());
}

void AMatchPawn::TryRegisterVOIPTalker()
{
	if (!VOIPTalker)
	{
		return;
	}

	APlayerState* PS = GetPlayerState();
	if (!PS)
	{
		return;
	}
	
	VOIPTalker->OnTalkingStateChanged.RemoveDynamic(this, &ThisClass::HandleVoiceTalkingStateChanged);
	VOIPTalker->OnTalkingStateChanged.AddDynamic(this, &ThisClass::HandleVoiceTalkingStateChanged);
	VOIPTalker->RegisterTalker(PS);
	TryInitVoiceSlider();

	// RegisterRemoteTalker는 VoiceInterface가 아직 초기화되지 않으면 조용히 실패한다.
	// 로컬 플레이어 자신은 등록 불필요(no-op). 원격 플레이어 등록 실패 시 1초 후 재시도.
	if (!IsLocallyControlled() && !VOIPTalker->IsRemoteTalkerRegistered())
	{
		GetWorldTimerManager().SetTimer(RetryVOIPRegistrationHandle,
			this, &AMatchPawn::TryRegisterVOIPTalker, 1.0f, false);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(RetryVOIPRegistrationHandle);
	}
}

void AMatchPawn::TryInitVoiceSlider()
{
	if (bVoiceSliderInitialized || !VoiceSliderComponent) return;

	ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>();
	if (!DPS) return;

	if (const APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	{
		if (ULocalPlayer* LocalPlayer = LocalPC->GetLocalPlayer())
		{
			VoiceSliderComponent->SetOwnerPlayer(LocalPlayer);
		}
	}

	VoiceSliderComponent->InitWidget();
	if (UVoiceVolumeRowWidget* SliderWidget = Cast<UVoiceVolumeRowWidget>(VoiceSliderComponent->GetUserWidgetObject()))
	{
		SliderWidget->Init(DPS, IsLocallyControlled());
		bVoiceSliderInitialized = true;
	}
}

void AMatchPawn::HandleVoiceTalkingStateChanged(bool bIsTalking)
{
	// 해당 Pawn의 Owner가 PushToTalk 모드를 사용하고 있을 경우에는
	// V키에서 손을 떼야지만 UI가 사라짐
	// Auto Input모드일 경우에는 말을 하고 있는 경우에 UI 활성화
	if (bDesiredSpeakingByPTT && !bIsTalking)
	{
		return;
	}

	SetSpeakerIconVisible(bIsTalking);
}

void AMatchPawn::Server_SetSpeaking_Implementation(bool bSpeaking)
{
	Multicast_SetSpeaking(bSpeaking);
}

void AMatchPawn::Multicast_SetSpeaking_Implementation(bool bSpeaking)
{
	bDesiredSpeakingByPTT = bSpeaking;

	SetSpeakerIconVisible(bSpeaking);
}

void AMatchPawn::SetSpeakerIconVisible(bool bVisible)
{
	if (!SpeakerIndicatorComponent)
	{
		return;
	}

	UUserWidget* UW = SpeakerIndicatorComponent->GetUserWidgetObject();
	if (!UW)
	{
		SpeakerIndicatorComponent->InitWidget();
		UW = SpeakerIndicatorComponent->GetUserWidgetObject();
	}

	if (UW)
	{
		UW->SetVisibility(ESlateVisibility::HitTestInvisible);
		UW->SetRenderOpacity(bVisible ? 1.0f : 0.0f);
	}
}
