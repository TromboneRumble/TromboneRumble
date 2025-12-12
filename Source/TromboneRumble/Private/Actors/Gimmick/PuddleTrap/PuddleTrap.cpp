// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/PuddleTrap/PuddleTrap.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "Utilities/DebugHelper.h"

APuddleTrap::APuddleTrap()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);

	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComponent->SetCollisionObjectType(ECC_WorldStatic);
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	// 캐릭터 채널이 Pawn이 아닐 경우 수정 필요
	BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BoxComponent->SetGenerateOverlapEvents(true);

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(BoxComponent);

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	if (AkComponent)
	{
		AkComponent->OcclusionRefreshInterval = 0.f;
		AkComponent->SetupAttachment(RootComponent);
	}

	
	
}

void APuddleTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bGrowing && GrowDuration > 0.f)
	{
		GrowElapsed += DeltaTime;
		const float Alpha = FMath::Clamp(GrowElapsed / GrowDuration, 0.f, 1.f);
		const float NewScale = FMath::Lerp(0.f, 1.f, Alpha);
		SetActorScale3D(FVector(NewScale));

		if (Alpha >= 1.f)
		{
			bGrowing = false;
		}
	}
}


void APuddleTrap::BeginPlay()
{
	Super::BeginPlay();

	SetActorScale3D(FVector::ZeroVector);

	if (DecalComponent)
	{
		// FadeOut: 스폰 기준으로 Delay 후부터 Duration 동안 페이드
		DecalComponent->SetFadeOut(FadeDelay + GrowDuration, FadeDuration, false);
	}

	// 성장 시작
	bGrowing = (GrowDuration > 0.f);
	GrowElapsed = 0.f;

	if (BoxComponent)
	{
		BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBoxBeginOverlap);
		BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnBoxEndOverlap);
	}

	if (AkComponent && PuddleSpawnSFX)
	{
		FOnAkPostEventCallback Callback;
		AkComponent->PostAkEvent(
			PuddleSpawnSFX,
			0,
			Callback
		);
	}

	// Fade가 끝났을때 Destroy
	if (HasAuthority())
	{
		const float Lifetime = FadeDelay + FadeDuration + GrowDuration;
		GetWorldTimerManager().SetTimer(
			LifetimeTimerHandle,
			FTimerDelegate::CreateLambda([this]()
				{
					Destroy();
				}),
			Lifetime,
			false
		);
	}
}

void APuddleTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 서버에서 Actor가 파괴될 때, 아직 슬로우 GE 걸려있으면 제거
	if (HasAuthority())
	{
		for (auto& Pair : ActiveSlowEffects)
		{
			if (ACharacter* Character = Pair.Key.Get())
			{
				if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character))
				{
					if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
					{
						if (Pair.Value.IsValid())
						{
							ASC->RemoveActiveGameplayEffect(Pair.Value);
						}
					}
				}
			}
		}
		ActiveSlowEffects.Empty();
	}

	Super::EndPlay(EndPlayReason);
}

void APuddleTrap::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	
	if (!HasAuthority() || !OtherActor)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character);
	if (!ASCInterface)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	if (!PuddleSlowEffectClass)
	{
		return;
	}

	const UGameplayEffect* GE = PuddleSlowEffectClass->GetDefaultObject<UGameplayEffect>();
	if (!GE)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(GE, 1.f, Context);

	if (Handle.IsValid())
	{
		ActiveSlowEffects.Add(Character, Handle);
	}
	else
	{
		Debug::Print(TEXT("[PUDDLE][BeginOverlap] Handle NOT STORED (INVALID)"));
	}
}

void APuddleTrap::OnBoxEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{

	if (!HasAuthority() || !OtherActor)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	// ASC 체크
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character);
	if (!ASCInterface)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FActiveGameplayEffectHandle* FoundHandle = ActiveSlowEffects.Find(Character);

	if (!FoundHandle)
	{
		return;
	}


	// Valid 체크
	if (FoundHandle->IsValid())
	{
		ASC->RemoveActiveGameplayEffect(*FoundHandle);
	}
	else
	{
		Debug::Print(TEXT("[PUDDLE][EndOverlap] Handle NOT VALID — SKIP Remove"));
	}

	// 맵에서 제거
	ActiveSlowEffects.Remove(Character);
}
