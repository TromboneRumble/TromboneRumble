#include "Pawns/MatchPawn.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "Components/ActorComponents/NameplateComponent.h"
#include "Components/ActorComponents/TromboneVOIPTalker.h"
#include "Engine/LocalPlayer.h"
#include "Framework/DefaultPlayerState.h"
#include "Framework/GameState/MatchMenuGameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

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
		FaceMID = Cast<UMaterialInstanceDynamic>(CurrentFaceMat);
		if (!FaceMID)
		{
			FaceMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(FaceMaterialIndex);
		}
	}
	
	// Apply a skin color in server side
	UpdateSkinFromPlayerState();
}

void AMatchPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
}

void AMatchPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server쪽에서 OnRep_PlayerState가 호출되지 않기 때문에 PossessedBy에서 호출
	TryRegisterVOIPTalker();
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
