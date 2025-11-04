// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmNoteWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmNoteWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Note|Motion")
	void InitializeNote(const FVector2D& InStartNorm, const FVector2D& InEndNorm);

	UFUNCTION(BlueprintCallable, Category = "Note|Motion")
	void SetProgress(float InAlpha);

public:
	// A(스폰)~B(종료)까지의 정규화 좌표 [0..1] (뷰포트 비율)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Note|Motion")
	FVector2D StartNorm = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Note|Motion")
	FVector2D EndNorm = FVector2D::ZeroVector;


protected:
	virtual void NativeConstruct() override;

private:
	void UpdatePixelPosition(float Alpha);
};
