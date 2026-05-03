#include "UI/UserWidgets/Settings/VideoOptionPanel.h"
#include "Components/VerticalBox.h"
#include "GameFramework/GameUserSettings.h"
#include "SaveData/TromboneSaveGame.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "RHI.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleRowWidget.h"

void UVideoOptionPanel::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	BuildOptions();
}

void UVideoOptionPanel::HandleApplyButtonClicked()
{
    Super::HandleApplyButtonClicked();
    
    if (!SaveManagerSubsystem) return;

    FGraphicsSettingData NewSettings;

    if (CreatedWidgets.Contains(EGraphicsOptionType::OverallQuality))
       NewSettings.OverallQuality = CreatedWidgets[EGraphicsOptionType::OverallQuality]->GetCurrentIndex();
    
    if (CreatedWidgets.Contains(EGraphicsOptionType::ViewDistance))
       NewSettings.ViewDistance = CreatedWidgets[EGraphicsOptionType::ViewDistance]->GetCurrentIndex();

    if (CreatedWidgets.Contains(EGraphicsOptionType::AntiAliasing))
       NewSettings.AntiAliasing = CreatedWidgets[EGraphicsOptionType::AntiAliasing]->GetCurrentIndex();
    
    if (CreatedWidgets.Contains(EGraphicsOptionType::PostProcess))
       NewSettings.PostProcess = CreatedWidgets[EGraphicsOptionType::PostProcess]->GetCurrentIndex();

    if (CreatedWidgets.Contains(EGraphicsOptionType::Shadow))
       NewSettings.Shadow = CreatedWidgets[EGraphicsOptionType::Shadow]->GetCurrentIndex();

    if (CreatedWidgets.Contains(EGraphicsOptionType::GlobalIllumination))
       NewSettings.GlobalIllumination = CreatedWidgets[EGraphicsOptionType::GlobalIllumination]->GetCurrentIndex();

    if (CreatedWidgets.Contains(EGraphicsOptionType::Reflections))
       NewSettings.Reflections = CreatedWidgets[EGraphicsOptionType::Reflections]->GetCurrentIndex();
    
    if (CreatedWidgets.Contains(EGraphicsOptionType::Texture))
       NewSettings.Texture = CreatedWidgets[EGraphicsOptionType::Texture]->GetCurrentIndex();

    if (CreatedWidgets.Contains(EGraphicsOptionType::Effects))
       NewSettings.Effects = CreatedWidgets[EGraphicsOptionType::Effects]->GetCurrentIndex();

    if (CreatedWidgets.Contains(EGraphicsOptionType::Resolution))
    {
		FString ResString = CreatedWidgets[EGraphicsOptionType::Resolution]->GetOptionsArray()[CreatedWidgets[EGraphicsOptionType::Resolution]->GetCurrentIndex()].ToString();
		ResString = ResString.Replace(TEXT(" "), TEXT(""));

		FString Left, Right;
		if (ResString.Split(TEXT("x"), &Left, &Right))
		{
			NewSettings.Resolution = FIntPoint(FCString::Atoi(*Left), FCString::Atoi(*Right));
		}
    }

    if (CreatedWidgets.Contains(EGraphicsOptionType::VSync))
       NewSettings.bVSync = CreatedWidgets[EGraphicsOptionType::VSync]->GetCurrentIndex() == 1;
	
	if (CreatedWidgets.Contains(EGraphicsOptionType::WindowMode))
	{
		const int32 WindowModeIdx = CreatedWidgets[EGraphicsOptionType::WindowMode]->GetCurrentIndex();
    
		EWindowMode::Type NewWindowMode;
		switch (WindowModeIdx)
		{
			case 0: 
				NewWindowMode = EWindowMode::Fullscreen;
				break;
			case 1:
				NewWindowMode = EWindowMode::WindowedFullscreen;
				break;
			case 2:
				NewWindowMode = EWindowMode::Windowed;
				break;
			default:
				NewWindowMode = EWindowMode::WindowedFullscreen;
				break;
		}
    
		NewSettings.WindowMode = NewWindowMode;
	}

    SaveManagerSubsystem->ApplyAndSaveVideo(NewSettings);
	RefreshUI();
}

void UVideoOptionPanel::HandleResetButtonClicked()
{
	Super::HandleResetButtonClicked();
	
	ReapplySavedSettings();
	RefreshUI();
}

void UVideoOptionPanel::BuildOptions()
{
	if (!GraphicsOptionsDataTable || !OptionCycleWidgetClass || !VB_OptionContainer) return;
	
	if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
	{
		VideoSettings->LoadSettings();
	}

	VB_OptionContainer->ClearChildren();
	CreatedWidgets.Empty();

	static const FString ContextString(TEXT("Graphics Option Context"));
	TArray<FGraphicsOptionRow*> AllRows;
	GraphicsOptionsDataTable->GetAllRows<FGraphicsOptionRow>(ContextString, AllRows);

	for (const FGraphicsOptionRow* Row : AllRows)
	{
		if (!Row) continue;

		if (UOptionCycleRowWidget* NewWidget = CreateWidget<UOptionCycleRowWidget>(this, OptionCycleWidgetClass))
		{
			TArray<FText> Labels = Row->OptionLabels;
			const int32 StartIndex = Row->DefaultIndex;

			if (Row->OptionType == EGraphicsOptionType::Resolution)
			{
				TArray<FIntPoint> SupportedResolutions;

				if (UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedResolutions))
				{
					TArray<FText> ValidLabels;
					for (const FText& Label : Labels)
					{
						FString LabelStr = Label.ToString();
						FString Left, Right;

						if (LabelStr.Split(TEXT("x"), &Left, &Right))
						{
							const int32 Width = FCString::Atoi(*Left);
							const int32 Height = FCString::Atoi(*Right);
							FIntPoint TargetRes(Width, Height);

							if (SupportedResolutions.Contains(TargetRes))
							{
								ValidLabels.Add(Label);
							}
						}
					}
        
					Labels = ValidLabels;
				}
			}
			
			NewWidget->ForceInit(Row->DisplayName, Labels, StartIndex);
			VB_OptionContainer->AddChild(NewWidget);
			
			CreatedWidgets.Add(Row->OptionType, NewWidget);
			
			if (Row->OptionType == EGraphicsOptionType::OverallQuality)
			{
				NewWidget->OnRotatedWithDirection().AddDynamic(this, &UVideoOptionPanel::OnOverallQualityChanged);
			}
			else if (Row->OptionType == EGraphicsOptionType::WindowMode)
			{
				NewWidget->OnRotatedWithDirection().AddDynamic(this, &UVideoOptionPanel::OnWindowModeChanged);
			}
			else if (Row->OptionType != EGraphicsOptionType::Resolution &&
					 Row->OptionType != EGraphicsOptionType::VSync &&
					 Row->OptionType != EGraphicsOptionType::WindowMode)
			{
				NewWidget->OnRotatedWithDirection().AddDynamic(this, &UVideoOptionPanel::OnSubOptionChanged);
			}
		}
	}

	RefreshUI();
}

void UVideoOptionPanel::RefreshUI()
{
	if (!GEngine) return;
	
    const UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings();
    if (!VideoSettings || CreatedWidgets.Num() == 0) return;

	if (CreatedWidgets.Contains(EGraphicsOptionType::OverallQuality))
	{
		const int32 OverallLevel = VideoSettings->GetOverallScalabilityLevel();
    
		if (OverallLevel == -1)
		{
			const int32 CustomIndex = CreatedWidgets[EGraphicsOptionType::OverallQuality]->GetOptionsArray().Num() - 1;
			CreatedWidgets[EGraphicsOptionType::OverallQuality]->SetSelectedIndex(CustomIndex);
		}
		else
		{
			CreatedWidgets[EGraphicsOptionType::OverallQuality]->SetSelectedIndex(OverallLevel);
		}
	}
    if (CreatedWidgets.Contains(EGraphicsOptionType::ViewDistance))
        CreatedWidgets[EGraphicsOptionType::ViewDistance]->SetSelectedIndex(VideoSettings->GetViewDistanceQuality());

    if (CreatedWidgets.Contains(EGraphicsOptionType::AntiAliasing))
        CreatedWidgets[EGraphicsOptionType::AntiAliasing]->SetSelectedIndex(VideoSettings->GetAntiAliasingQuality());

    if (CreatedWidgets.Contains(EGraphicsOptionType::PostProcess))
        CreatedWidgets[EGraphicsOptionType::PostProcess]->SetSelectedIndex(VideoSettings->GetPostProcessingQuality());

    if (CreatedWidgets.Contains(EGraphicsOptionType::Shadow))
        CreatedWidgets[EGraphicsOptionType::Shadow]->SetSelectedIndex(VideoSettings->GetShadowQuality());

    if (CreatedWidgets.Contains(EGraphicsOptionType::GlobalIllumination))
        CreatedWidgets[EGraphicsOptionType::GlobalIllumination]->SetSelectedIndex(VideoSettings->GetGlobalIlluminationQuality());

    if (CreatedWidgets.Contains(EGraphicsOptionType::Reflections))
        CreatedWidgets[EGraphicsOptionType::Reflections]->SetSelectedIndex(VideoSettings->GetReflectionQuality());

    if (CreatedWidgets.Contains(EGraphicsOptionType::Texture))
        CreatedWidgets[EGraphicsOptionType::Texture]->SetSelectedIndex(VideoSettings->GetTextureQuality());

    if (CreatedWidgets.Contains(EGraphicsOptionType::Effects))
        CreatedWidgets[EGraphicsOptionType::Effects]->SetSelectedIndex(VideoSettings->GetVisualEffectQuality());

	if (CreatedWidgets.Contains(EGraphicsOptionType::Resolution))
	{
		const FIntPoint CurrentRes = VideoSettings->GetScreenResolution();
		const FString CurrentResStr = FString::Printf(TEXT("%dx%d"), CurrentRes.X, CurrentRes.Y);
    
		const TArray<FText>& ResOptions = CreatedWidgets[EGraphicsOptionType::Resolution]->GetOptionsArray();
		int32 TargetIdx = -1;

		for (int32 i = 0; i < ResOptions.Num(); ++i)
		{
			FString OptionStr = ResOptions[i].ToString().Replace(TEXT(" "), TEXT(""));
			if (OptionStr.Equals(CurrentResStr))
			{
				TargetIdx = i;
				break;
			}
		}
		
		CreatedWidgets[EGraphicsOptionType::Resolution]->SetSelectedIndex(TargetIdx != -1 ? TargetIdx : 0);
	}
	
	if (CreatedWidgets.Contains(EGraphicsOptionType::VSync))
	{
		const int32 VSyncIndex = VideoSettings->IsVSyncEnabled() ? 1 : 0;
		CreatedWidgets[EGraphicsOptionType::VSync]->SetSelectedIndex(VSyncIndex);
	}
	
	if (CreatedWidgets.Contains(EGraphicsOptionType::WindowMode))
	{
		const EWindowMode::Type CurrentMode = VideoSettings->GetFullscreenMode();
		int32 WindowModeIndex;
	
		switch (CurrentMode)
		{
			case EWindowMode::Fullscreen:
				WindowModeIndex = 0;
				break;
			case EWindowMode::WindowedFullscreen:
				WindowModeIndex = 1;
				break;
			case EWindowMode::Windowed:
				WindowModeIndex = 2;
				break;
			default:
				WindowModeIndex = 1;
				break;
		}
	
		CreatedWidgets[EGraphicsOptionType::WindowMode]->SetSelectedIndex(WindowModeIndex);
		OnWindowModeChanged(WindowModeIndex, ERotatorDirection::Right);
	}
}

void UVideoOptionPanel::ReapplySavedSettings()
{
	Super::ReapplySavedSettings();
	
	if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
	{
		VideoSettings->LoadSettings(true);
		VideoSettings->ApplySettings(false);
	}
}

void UVideoOptionPanel::OnOverallQualityChanged(const int32 Value, ERotatorDirection RotatorDir)
{
	const int32 CustomIndex = CreatedWidgets[EGraphicsOptionType::OverallQuality]->GetOptionsArray().Num() - 1;
	if (Value == CustomIndex) return;

	for (const auto& Elem : CreatedWidgets)
	{
		if (Elem.Key != EGraphicsOptionType::OverallQuality && 
			Elem.Key != EGraphicsOptionType::Resolution && 
			Elem.Key != EGraphicsOptionType::VSync &&
			Elem.Key != EGraphicsOptionType::WindowMode)
		{
			Elem.Value->SetSelectedIndex(Value);
		}
	}
}

void UVideoOptionPanel::OnSubOptionChanged(int32 Value, ERotatorDirection RotatorDir)
{
	if (CreatedWidgets.Contains(EGraphicsOptionType::OverallQuality))
	{
		const int32 CustomIndex = CreatedWidgets[EGraphicsOptionType::OverallQuality]->GetOptionsArray().Num() - 1;
		CreatedWidgets[EGraphicsOptionType::OverallQuality]->SetSelectedIndex(CustomIndex);
	}
}

void UVideoOptionPanel::OnWindowModeChanged(const int32 Value, ERotatorDirection RotatorDir)
{
	if (CreatedWidgets.Contains(EGraphicsOptionType::Resolution))
	{
		const bool bIsWindowed = (Value == 2);
        
		UOptionCycleRowWidget* ResWidget = CreatedWidgets[EGraphicsOptionType::Resolution];
		ResWidget->SetIsEnabled(bIsWindowed);
	}
}
