// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenuWidget.h"
#include "CommonButtonBase.h"
#include "EasySessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/SpinBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UserWidgets/MainMenu/NoticePopupWidget.h"
#include "Utilities/DebugHelper.h"

void UMatchMenuWidget::Init(const TFunction<void()> OnMenuClosedCallback)
{
	OnMenuClosed = OnMenuClosedCallback;
}

void UMatchMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (LobbyCodeText)
	{
		LobbyCodeText->SetText(FText::GetEmpty());
	}
}

void UMatchMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	const FString LobbyMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);
	checkf(!LobbyMapPath.IsEmpty(), TEXT("Lobby map path not found. Please set it in GameMapDeveloperSettings."));
	CachedLobbyMapPath = LobbyMapPath;
	
	BindSubsystemCallbacks();
	InitButtons();
	
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	
	if (MaxPlayerSlider && MaxPlayerSpinBox)
	{
		MaxPlayerSlider->OnValueChanged.AddDynamic(this, &ThisClass::OnMaxPlayerSliderChanged);
		MaxPlayerSpinBox->OnValueChanged.AddDynamic(this, &ThisClass::OnMaxPlayerSpinBoxChanged);
		MaxPlayerSpinBox->SetValue(MaxPlayerSlider->GetValue());
	}
}

void UMatchMenuWidget::NativeDestruct()
{
	RemoveFromParent();
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	RemoveSubsystemCallbacks();

	Super::NativeDestruct();
}

void UMatchMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	if (OnMenuClosed) OnMenuClosed();
}

UWidget* UMatchMenuWidget::NativeGetDesiredFocusTarget() const
{
	return CB_Host;
}

void UMatchMenuWidget::InitButtons()
{
	if (CB_Host)
	{
		CB_Host->OnClicked().RemoveAll(this);
		CB_Host->OnClicked().AddUObject(this, &ThisClass::HostButtonClicked);
	}
	if (CB_Join)
	{
		CB_Join->OnClicked().RemoveAll(this);
		CB_Join->OnClicked().AddUObject(this, &ThisClass::JoinButtonClicked);
	}
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddLambda([this]
		{
			if (OnMenuClosed) OnMenuClosed();
		});
	}
}

void UMatchMenuWidget::BindSubsystemCallbacks()
{
	if (!SessionsSubsystem)
	{
		const UGameInstance* GameInstance = GetGameInstance();
		SessionsSubsystem = GameInstance->GetSubsystem<UEasySessionSubsystem>();
	}
	
	RemoveSubsystemCallbacks();
	
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnStartSessionSuccess.AddUObject(this, &ThisClass::OnStartSessionSuccess);
		SessionsSubsystem->OnStartSessionFailure.AddUObject(this, &ThisClass::OnStartSessionFailure);
		
		SessionsSubsystem->OnFindSessionsSuccess.AddUObject(this, &ThisClass::OnFindSessionsSuccess);
		SessionsSubsystem->OnFindSessionsFailure.AddUObject(this, &ThisClass::OnFindSessionsFailure);
		
		SessionsSubsystem->OnJoinSessionSuccess.AddUObject(this, &ThisClass::OnJoinSessionSuccess);
		SessionsSubsystem->OnJoinSessionFailure.AddUObject(this, &ThisClass::OnJoinSessionFailure);
		
		SessionsSubsystem->OnDestroySessionSuccess.AddUObject(this, &ThisClass::OnDestroySessionSuccess);
		SessionsSubsystem->OnDestroySessionFailure.AddUObject(this, &ThisClass::OnDestroySessionFailure);
	}
}

void UMatchMenuWidget::RemoveSubsystemCallbacks()
{
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnStartSessionSuccess.RemoveAll(this);
		SessionsSubsystem->OnStartSessionFailure.RemoveAll(this);
		
		SessionsSubsystem->OnFindSessionsSuccess.RemoveAll(this);
		SessionsSubsystem->OnFindSessionsFailure.RemoveAll(this);
		
		SessionsSubsystem->OnJoinSessionSuccess.RemoveAll(this);
		SessionsSubsystem->OnJoinSessionFailure.RemoveAll(this);
		
		SessionsSubsystem->OnDestroySessionSuccess.RemoveAll(this);
		SessionsSubsystem->OnDestroySessionFailure.RemoveAll(this);
	}
}

void UMatchMenuWidget::OnStartSessionSuccess()
{
	const FString LobbyPkg = FPackageName::ObjectPathToPackageName(CachedLobbyMapPath);
	const FString URL = LobbyPkg + TEXT("?listen");
	UGameplayStatics::OpenLevel(this, FName(*URL), true);
}

void UMatchMenuWidget::OnStartSessionFailure()
{
	SetUIEnabled(true);
	ShowNoticePopup(TEXT("세션 생성에 실패했습니다. 다시 시도해주세요."));
}

void UMatchMenuWidget::OnFindSessionsSuccess(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	if (bIsSearchingForHostValidation)
	{
		bIsSearchingForHostValidation = false;
		
		for (auto Result : SessionResults)
		{
			FString SettingsValue;
			Result.Session.SessionSettings.Get(GKey_Lobby_Code, SettingsValue);
	
			if (SettingsValue == PendingLobbyCode)
			{
				if (LobbyCodeText->GetText().IsEmpty()) 
				{
					const FString NewCode = GenerateRandomLobbyCode(FMath::Max(2, MaxLobbyCodeLength));
					StartHostValidation(NewCode);
				}
				else
				{
					SetUIEnabled(true);
					ShowNoticePopup(TEXT("이미 사용 중인 코드입니다."));
				}
				return;
			}
		}
		
		CreateSessionAfterValidation(PendingLobbyCode);
		return;
	}
	
	const FString& LobbyCode = LobbyCodeText->GetText().ToString();

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(GKey_Lobby_Code, SettingsValue);
		
		if (SettingsValue == LobbyCode)
		{
			Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
			Result.Session.SessionSettings.bUsesPresence = true;
			SessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	
	SetUIEnabled(true);
	ShowNoticePopup(FString::Printf(TEXT("'%s'에 해당하는 세션을 찾을 수 없습니다."), *LobbyCode));
}

void UMatchMenuWidget::OnFindSessionsFailure(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	if (bIsSearchingForHostValidation)
	{
		bIsSearchingForHostValidation = false;
		SetUIEnabled(true);
		ShowNoticePopup(TEXT("네트워크 상태가 불안정하여 중복 검사에 실패했습니다."));
		return;
	}
	
	SetUIEnabled(true);
	ShowNoticePopup(TEXT("세션 검색에 실패했습니다. 다시 시도해주세요."));
}

void UMatchMenuWidget::OnJoinSessionSuccess()
{
	SetUIEnabled(true);
}

void UMatchMenuWidget::OnJoinSessionFailure()
{
	SetUIEnabled(true);
	ShowNoticePopup(TEXT("세션 참가에 실패했습니다. 다시 시도해주세요."));
}

void UMatchMenuWidget::OnDestroySessionSuccess()
{
	PRINT_WITH_CURRENT_CONTEXT("Session destroyed successfully");
}

void UMatchMenuWidget::OnDestroySessionFailure()
{
	ShowNoticePopup(TEXT("세션 종료에 실패했습니다. 다시 시도해주세요."));
}

void UMatchMenuWidget::OnMaxPlayerSliderChanged(const float Value)
{
	if (MaxPlayerSpinBox)
	{
		MaxPlayerSpinBox->SetValue(FMath::RoundToInt(Value));
	}
}

void UMatchMenuWidget::OnMaxPlayerSpinBoxChanged(const float Value)
{
	if (MaxPlayerSlider)
	{
		MaxPlayerSlider->SetValue(Value);
	}
}

void UMatchMenuWidget::HostButtonClicked()
{
	FString LobbyCode;
	if (LobbyCodeText->GetText().IsEmpty())
	{
		LobbyCode = GenerateRandomLobbyCode(FMath::Max(2, MaxLobbyCodeLength));
	}
	else
	{
		LobbyCode = LobbyCodeText->GetText().ToString().ToUpper();
	}
	
	if (SessionsSubsystem)
	{
		SetUIEnabled(false);
		StartHostValidation(LobbyCode);
	}
}

void UMatchMenuWidget::JoinButtonClicked()
{
	if (LobbyCodeText->GetText().IsEmpty())
	{
		ShowNoticePopup(TEXT("로비 코드를 입력해주세요."));
		return;
	}
	
	if (SessionsSubsystem)
	{
		SetUIEnabled(false);

		FEasySearchSettings SearchSettings;
		SearchSettings.QuerySettings.Add(GKey_Lobby_Code.ToString(), LobbyCodeText->GetText().ToString().ToUpper());
		SessionsSubsystem->FindSessions(SearchSettings);
	}
}

void UMatchMenuWidget::StartHostValidation(const FString& Code)
{
	bIsSearchingForHostValidation = true;
	PendingLobbyCode = Code;
    
	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Validating Lobby Code: %s"), *Code));

	FEasySearchSettings SearchSettings;
	SearchSettings.QuerySettings.Add(GKey_Lobby_Code.ToString(), Code); 
	SessionsSubsystem->FindSessions(SearchSettings);
}

void UMatchMenuWidget::CreateSessionAfterValidation(const FString& ValidatedCode)
{
	if (SessionsSubsystem)
	{
		FEasySessionSettings Settings;
		Settings.NumPublicConnections = FMath::RoundToInt(MaxPlayerSlider->GetValue());
		Settings.CustomProperties.Add(GKey_Lobby_Code.ToString(), ValidatedCode);
		SessionsSubsystem->CreateSession(Settings);
	}
}

void UMatchMenuWidget::SetUIEnabled(const bool bEnabled)
{
	CB_Host->SetIsEnabled(bEnabled);
	CB_Join->SetIsEnabled(bEnabled);
	CB_Back->SetIsEnabled(bEnabled);
	LobbyCodeText->SetIsEnabled(bEnabled);
	MaxPlayerSlider->SetIsEnabled(bEnabled);
	MaxPlayerSpinBox->SetIsEnabled(bEnabled);
}

FString UMatchMenuWidget::GenerateRandomLobbyCode(const int32 Length) const
{
	const FString Chars = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	FString RandomCode;
	for (int32 i = 0; i < Length; ++i)
	{
		RandomCode += Chars[FMath::RandRange(0, Chars.Len() - 1)];
	}
	
	FPlatformApplicationMisc::ClipboardCopy(*RandomCode);
	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("로비 코드 %s가 생성되어 클립보드에 복사되었습니다."), *RandomCode));
	
	return RandomCode;
}

void UMatchMenuWidget::ShowNoticePopup(const FString& Content)
{
	if (NoticePopupWidgetClass)
	{
		UNoticePopupWidget* NoticePopup = CreateWidget<UNoticePopupWidget>(GetOwningPlayer(), NoticePopupWidgetClass);
		NoticePopup->OnInit(Content);
	}
}