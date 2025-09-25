// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PT_UIMain.generated.h"

UCLASS()
class TROMBONERUMBLE_API UPT_UIMain : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnClickHostGame();
	
	UFUNCTION()
	void OnClickJoinGame();

	UFUNCTION()
	void OnClickStartGame();
	
	UFUNCTION()
	void HandleSessionJoined(const FString& LobbyCode);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> HostGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> JoinGameButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableText> LobbyCodeInput;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> LobbyCodeText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> AuthText;
};
