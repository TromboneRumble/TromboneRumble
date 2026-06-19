#include "Pawns/MatchPawn.h"
#include "UI/UserWidgets/MatchMenu/VoiceVolumeRowWidget.h"
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
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "Utilities/Defines.h"

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

	VoiceSliderComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("VoiceSliderComponent"));
	if (VoiceSliderComponent)
	{
		VoiceSliderComponent->SetupAttachment(CapsuleComponent);
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

	const int32 FaceIndex = SkeletalMeshComponent->GetMaterialIndex(TromboneMaterial::FaceSlotName);
	if (FaceIndex == INDEX_NONE) return;

	SkeletalMeshComponent->SetMaterial(FaceIndex, Target);
	FaceMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(FaceIndex);
	if (FaceMID)
	{
		if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
		{
			FaceMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, DPS->GetSkinColor());
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
			SkinMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, SkinColor);
		}
		if (FaceMID)
		{
			FaceMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, SkinColor);
		}
		// 몸통(costume)·안테나 follower 메시도 동일 색
		if (CustomizationComp)
		{
			CustomizationComp->ApplyPartsSkinColor(SkinColor);
		}
	}
}

void AMatchPawn::BeginPlay()
{
	// 머티리얼 슬롯은 인덱스 하드코딩 대신 슬롯 이름("skin"/"face")으로 조회
	SkinMID = UCustomizationComponent::EnsureSlotMID(SkeletalMeshComponent, TromboneMaterial::SkinSlotName);

	const int32 FaceIndex = SkeletalMeshComponent ? SkeletalMeshComponent->GetMaterialIndex(TromboneMaterial::FaceSlotName) : INDEX_NONE;
	if (FaceIndex != INDEX_NONE)
	{
		OriginalFaceMaterial = SkeletalMeshComponent->GetMaterial(FaceIndex);
		FaceMID = UCustomizationComponent::EnsureSlotMID(SkeletalMeshComponent, TromboneMaterial::FaceSlotName);
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

void AMatchPawn::Server_SetSpeaking_Implementation(bool bSpeaking)
{
	Multicast_SetSpeaking(bSpeaking);
}

void AMatchPawn::Multicast_SetSpeaking_Implementation(bool bSpeaking)
{
	// 표시 로직은 UI(MatchPawnSpeakerWidget)가 담당. 여기서는 PTT 상태만 VOIPTalker로 위임.
	if (VOIPTalker)
	{
		VOIPTalker->SetPushToTalkSpeaking(bSpeaking);
	}
}
