// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetBase.h"
#include "UI/UserWidgets/Rhythm/ResultWidget/RhythmResultWidgetBase.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"

#include "Subsystems/RhythmSubsystem.h"

URhythmSpawnWidgetBase::URhythmSpawnWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, WidgetPool(*this)
{
}

void URhythmSpawnWidgetBase::PrepareNoteContainer(const EInstrumentType& InType)
{
	if (!NoteCanvas || !NoteWidgetClass)
	{
		return;
	}
	
	if (InstrumentContainers.Contains(InType))
	{
		return;
	}

	if (!WidgetTree)
	{
		return;
	}

	// 악기 타입별 Panel 컨테이너 생성
	UCanvasPanel* NewContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	if (!NewContainer)
	{
		return;
	}

	NewContainer->SetVisibility(ESlateVisibility::Collapsed);

	if (UCanvasPanelSlot* CanvasPanelSlot = NoteCanvas->AddChildToCanvas(NewContainer))
	{
		CanvasPanelSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CanvasPanelSlot->SetOffsets(FMargin(0.f));
		CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));        
		CanvasPanelSlot->SetPosition(FVector2D(0.f, 0.f));           
		CanvasPanelSlot->SetAutoSize(false);                         
	}

	InstrumentContainers.Add(InType, NewContainer);
}


URhythmNoteWidgetBase* URhythmSpawnWidgetBase::SpawnPooledRhythmNoteWidget(const EInstrumentType& InType)
{
	if (!NoteWidgetClass || !NoteCanvas)
	{
		return nullptr;
	}

	// 해당 타입의 컨테이너가 준비되어 있는지 보장
	PrepareNoteContainer(InType);

	UPanelWidget* Container = nullptr;
	if (TObjectPtr<UPanelWidget>* FoundContainer = InstrumentContainers.Find(InType))
	{
		Container = FoundContainer->Get();
	}

	if (!Container)
	{
		return nullptr;
	}

	URhythmNoteWidgetBase* Note = WidgetPool.GetOrCreateInstance<URhythmNoteWidgetBase>(NoteWidgetClass);
	if (!Note)
	{
		return nullptr;
	}

	// 부모가 다르면 자동으로 이전 부모에서 떼어내고 새 컨테이너에 붙인다.
	if (Note->GetParent() != Container)
	{
		Container->AddChild(Note);
	}

	Note->Init(InType);
	Note->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Note;
}

URhythmResultWidgetBase* URhythmSpawnWidgetBase::SpawnPooledRhythmResultWidget(const FVector2D& SpawnPos, ENoteResult InNoteResult)
{
	if (!NoteCanvas || !NoteResultWidgetClass)
	{
		return nullptr;
	}

	URhythmResultWidgetBase* ResultWidget = WidgetPool.GetOrCreateInstance<URhythmResultWidgetBase>(NoteResultWidgetClass);
	if (!ResultWidget)
	{
		return nullptr;
	}

	// 부모가 NoteCanvas가 아니면 붙인다.
	if (ResultWidget->GetParent() != NoteCanvas)
	{
		NoteCanvas->AddChild(ResultWidget);
	}

	// 위치 설정
	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(ResultWidget->Slot))
	{
		CanvasPanelSlot->SetPosition(SpawnPos);
	}

	ResultWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	return ResultWidget;
}

void URhythmSpawnWidgetBase::ReleasePooledRhythmNoteWidget(URhythmNoteWidgetBase* Widget)
{
	if (!Widget)
	{
		return;
	}

	// 어떤 악기든 상관없이 풀로 되돌리고, 풀에서 반환시 Init으로 다른 컨테이너에 붙혀줄 것
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	WidgetPool.Release(Widget);
}

void URhythmSpawnWidgetBase::ReleasePooledRhythmResultWidget(URhythmResultWidgetBase* Widget)
{
	if (!Widget)
	{
		return;
	}

	Widget->SetVisibility(ESlateVisibility::Collapsed);
	WidgetPool.Release(Widget);
}

void URhythmSpawnWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!IsDesignTime())
	{
		PrepareRhythmResultWidgets();
		if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::OnInstrumentChangedHandler);
		}
		SetRenderOpacity(0.f);
	}
}

void URhythmSpawnWidgetBase::NativeDestruct()
{
	if (!IsDesignTime())
	{
		if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnInstrumentPicked.RemoveDynamic(this, &ThisClass::OnInstrumentChangedHandler);
		}
	}
	Super::NativeDestruct();
}

void URhythmSpawnWidgetBase::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	WidgetPool.ReleaseAllSlateResources();
}

void URhythmSpawnWidgetBase::OnInstrumentChangedHandler(EInstrumentType PrevType, EInstrumentType NewType)
{
	auto IsRealInstrument = [](EInstrumentType Type) -> bool
		{
			const uint8 V = static_cast<uint8>(Type);
			const uint8 BG = static_cast<uint8>(EInstrumentType::Background);
			const uint8 NONE = static_cast<uint8>(EInstrumentType::None);
			// Background(0) < 실제 악기들(1~3) < None(254)
			return (V > BG) && (V < NONE);
		};

	// PrevType 이 실제 악기면 해당 패널 끄기
	if (IsRealInstrument(PrevType))
	{
		if (TObjectPtr<UPanelWidget>* FoundPrev = InstrumentContainers.Find(PrevType))
		{
			if (UPanelWidget* PrevPanel = FoundPrev->Get())
			{
				PrevPanel->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	// NewType 이 Background 또는 None 이면 아무 로직도 실행하지 않음
	if (!IsRealInstrument(NewType))
	{
		return;
	}

	// NewType 이 실제 악기면 해당 패널 켜기
	if (TObjectPtr<UPanelWidget>* FoundNew = InstrumentContainers.Find(NewType))
	{
		if (UPanelWidget* NewPanel = FoundNew->Get())
		{
			NewPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

void URhythmSpawnWidgetBase::PrepareRhythmResultWidgets()
{
	if (!NoteCanvas || !NoteResultWidgetClass)
	{
		return;
	}

	constexpr int32 PrewarmCount = 30;
	for (int32 i = 0; i < PrewarmCount; ++i)
	{

		if (URhythmNoteWidgetBase* Note = WidgetPool.GetOrCreateInstance<URhythmNoteWidgetBase>(NoteWidgetClass))
		{
			Note->SetVisibility(ESlateVisibility::Collapsed);
			WidgetPool.Release(Note);
		}


		if (URhythmResultWidgetBase* Result = WidgetPool.GetOrCreateInstance<URhythmResultWidgetBase>(NoteResultWidgetClass))
		{
			if (Result->GetParent() != NoteCanvas)
			{
				NoteCanvas->AddChild(Result);
			}

			Result->SetVisibility(ESlateVisibility::Collapsed);
			WidgetPool.Release(Result);
		}
	}
}
