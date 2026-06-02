#include "Components/ActorComponents/NameplateComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Framework/DefaultPlayerState.h"
#include "UI/UserWidgets/MatchMenu/PlayerNameplateWidget.h"
#include "Utilities/DebugHelper.h"

UNameplateComponent::UNameplateComponent()
{
	bWantsInitializeComponent = true;
	NameplateWidgetClass = TSubclassOf<UUserWidget>();
	NameplateDrawSize = FVector2D(150.0f, 50.0f);
	NameplateOffset = FVector(0.0f, 0.0f, 100.0f);

	WidgetComponent = nullptr;
}

void UNameplateComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (!GetOwner()->IsA<APawn>())
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Owning actor is invalid! The component must be attached to a Pawn or Character."));
		return;
	}
	
	if (!NameplateWidgetClass.Get())
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("NameplateWidgetClass is empty!"));
		return;
	}

	WaitInitialReplication();
}

void UNameplateComponent::WaitInitialReplication()
{
	APawn* PlayerPawn = GetOwner<APawn>();
	APlayerState* PlayerState = PlayerPawn ? PlayerPawn->GetPlayerState() : nullptr;

	if (PlayerState)
	{
		CreateNameplate();
	}
	else
	{
		if (IsValid(this))
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::WaitInitialReplication);
		}
	}
}

void UNameplateComponent::CreateNameplate()
{
	if (WidgetComponent)
	{
		WidgetComponent->DestroyComponent();
	}

	WidgetComponent = NewObject<UWidgetComponent>(GetOwner());
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetWidgetClass(NameplateWidgetClass);
	WidgetComponent->SetBlendMode(EWidgetBlendMode::Masked);
	WidgetComponent->SetCastShadow(false);
	WidgetComponent->SetDrawSize(NameplateDrawSize);
	WidgetComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	WidgetComponent->SetRelativeLocation(NameplateOffset);
	if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	{
		if (ULocalPlayer* LocalPlayer = LocalPC->GetLocalPlayer())
		{
			WidgetComponent->SetOwnerPlayer(LocalPlayer);
		}
	}
	WidgetComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	WidgetComponent->RegisterComponent();

	WaitForNameplateWidget();
}
void UNameplateComponent::WaitForNameplateWidget()
{
	UUserWidget* NameplateWidget = WidgetComponent ? WidgetComponent->GetUserWidgetObject() : nullptr;
	if (NameplateWidget)
	{
		APawn* OwningPawn = GetOwner<APawn>();
		ADefaultPlayerState* PlayerState = OwningPawn ? Cast<ADefaultPlayerState>(OwningPawn->GetPlayerState()) : nullptr;

		if (PlayerState && !PlayerState->GetPlayerName().IsEmpty())
		{
			if (UPlayerNameplateWidget* Widget = Cast<UPlayerNameplateWidget>(NameplateWidget))
			{
				Widget->InitPlayerWidget(PlayerState);
			}

			OnNameplateCreatedEvent.Broadcast(NameplateWidget);
			return;
		}
	}
	if (IsValid(this) && IsValid(GetOwner()))
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::WaitForNameplateWidget);
	}
}

void UNameplateComponent::UninitializeComponent()
{
	if (WidgetComponent)
	{
		WidgetComponent->DestroyComponent();
	}

	Super::UninitializeComponent();
}