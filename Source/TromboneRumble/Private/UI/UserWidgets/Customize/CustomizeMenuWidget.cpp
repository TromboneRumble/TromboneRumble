#include "UI/UserWidgets/Customize/CustomizeMenuWidget.h"
#include "CommonButtonBase.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Framework/TromboneGameInstance.h"
#include "Pawns/CustomizePawn.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Popup/TwoButtonPopup.h"
#include "Utilities/Defines.h"
#include "Utilities/TromboneStatics.h"

void UCustomizeMenuWidget::Init()
{
	Super::Init();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ACustomizePawn* Pawn = Cast<ACustomizePawn>(PC->GetPawn()))
		{
			CustomizationComp = Pawn->CustomizationComp;
			OriginalSaveData = CustomizationComp->GetCurrentSaveData();
		}
	}

	if (CB_AntennaNext)  { CB_AntennaNext->OnClicked().RemoveAll(this);  CB_AntennaNext->OnClicked().AddUObject(this, &ThisClass::Handle_AntennaNext); }
	if (CB_AntennaPrev)  { CB_AntennaPrev->OnClicked().RemoveAll(this);  CB_AntennaPrev->OnClicked().AddUObject(this, &ThisClass::Handle_AntennaPrev); }
	if (CB_FaceNext)     { CB_FaceNext->OnClicked().RemoveAll(this);     CB_FaceNext->OnClicked().AddUObject(this, &ThisClass::Handle_FaceNext); }
	if (CB_FacePrev)     { CB_FacePrev->OnClicked().RemoveAll(this);     CB_FacePrev->OnClicked().AddUObject(this, &ThisClass::Handle_FacePrev); }
	if (CB_CostumeNext)  { CB_CostumeNext->OnClicked().RemoveAll(this);  CB_CostumeNext->OnClicked().AddUObject(this, &ThisClass::Handle_CostumeNext); }
	if (CB_CostumePrev)  { CB_CostumePrev->OnClicked().RemoveAll(this);  CB_CostumePrev->OnClicked().AddUObject(this, &ThisClass::Handle_CostumePrev); }
	if (CB_Randomize)    { CB_Randomize->OnClicked().RemoveAll(this);    CB_Randomize->OnClicked().AddUObject(this, &ThisClass::Handle_Randomize); }
	if (CB_Apply)        { CB_Apply->OnClicked().RemoveAll(this);        CB_Apply->OnClicked().AddUObject(this, &ThisClass::Handle_Apply); }
	if (CB_Back)         { CB_Back->OnClicked().RemoveAll(this);         CB_Back->OnClicked().AddUObject(this, &ThisClass::Handle_Back); }
}

void UCustomizeMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	if (!CustomizationComp)
		if (APlayerController* PC = GetOwningPlayer())
			if (ACustomizePawn* Pawn = Cast<ACustomizePawn>(PC->GetPawn()))
			{
				CustomizationComp = Pawn->CustomizationComp;
				OriginalSaveData = CustomizationComp->GetCurrentSaveData();
			}
}

void UCustomizeMenuWidget::Handle_AntennaNext()
{
	if (CustomizationComp) CustomizationComp->StepPart(ECustomizationSlotType::Antenna, +1);
}

void UCustomizeMenuWidget::Handle_AntennaPrev()
{
	if (CustomizationComp) CustomizationComp->StepPart(ECustomizationSlotType::Antenna, -1);
}

void UCustomizeMenuWidget::Handle_FaceNext()
{
	if (CustomizationComp) CustomizationComp->StepPart(ECustomizationSlotType::Face, +1);
}

void UCustomizeMenuWidget::Handle_FacePrev()
{
	if (CustomizationComp) CustomizationComp->StepPart(ECustomizationSlotType::Face, -1);
}

void UCustomizeMenuWidget::Handle_CostumeNext()
{
	if (CustomizationComp) CustomizationComp->StepPart(ECustomizationSlotType::Costume, +1);
}

void UCustomizeMenuWidget::Handle_CostumePrev()
{
	if (CustomizationComp) CustomizationComp->StepPart(ECustomizationSlotType::Costume, -1);
}

void UCustomizeMenuWidget::Handle_Randomize()
{
	if (CustomizationComp) CustomizationComp->RandomizeAll();
}

void UCustomizeMenuWidget::Handle_Apply()
{
	if (!CustomizationComp) return;
	if (USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
		SMS->SaveCustomization(CustomizationComp->GetCurrentSaveData());
	UTromboneStatics::OpenLevel(this, ELevelType::MainMenu);
}

void UCustomizeMenuWidget::Handle_Back()
{
	if (CustomizationComp && CustomizationComp->IsDirtyFrom(OriginalSaveData))
		ShowBackPopup();
	else
		UTromboneStatics::OpenLevel(this, ELevelType::MainMenu);
}

void UCustomizeMenuWidget::ShowBackPopup()
{
	const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance());
	if (!GI) return;

	FTwoButtonPopupParams Params;
	Params.Content			= GI->GetCommonUIText(TEXT("SettingPopup_AskConfirmation"));
	Params.LeftButtonText	= GI->GetCommonUIText(TEXT("Common_Apply"));
	Params.RightButtonText	= GI->GetCommonUIText(TEXT("Common_No"));
	Params.bCloseOnLeftButtonClick  = true;
	Params.bCloseOnRightButtonClick = true;
	Params.LeftCallback = [this]()
	{
		if (CustomizationComp)
			if (USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
				SMS->SaveCustomization(CustomizationComp->GetCurrentSaveData());
		UTromboneStatics::OpenLevel(this, ELevelType::MainMenu);
	};
	Params.RightCallback = [this]() { UTromboneStatics::OpenLevel(this, ELevelType::MainMenu); };

	if (UTwoButtonPopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
		Popup->Init(Params);
}
