#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QuestData.generated.h"

/**
 * Conditions for quest success
 * @see FQuestData
 * @see EQuestConditionParamType
 * @see EBasicActionType
 * @see EInstrumentType
 * @see ENoteResult
 */
UENUM()
enum class EQuestConditionType : uint8
{
	/** Move, Run, Jump, Headbutt, Hit ... */
	BasicAction,
	
	/** Equip instrument */
	EquipInstrument,
	
	/** Hit note in rhythm games */
	GetNoteLevel,
	
	/** Hit spotlight */
	HitSpotlight,
};

UENUM()
enum class EQuestConditionParamType : uint8
{
	/** A single specific action. 
	 *	For example, in BasicAction, Move */
	Specific = 0,
	
	/** Any action. 
	 *	For example, in BasicAction, Move, Run, Jump ... all of them. */
	Any = 1,
};

USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
	GENERATED_BODY()

	/** 내부 확인용 코멘트 */
	UPROPERTY(EditAnywhere)
	FString Comment;
	
	/** String Table에서 참조할 ID */
	UPROPERTY(EditAnywhere)
	FString StringID;
	
	/** 아이콘 경로 */
	UPROPERTY(EditAnywhere)
	FString IconPath;

	UPROPERTY(EditAnywhere)
	EQuestConditionType QuestCondition;

	UPROPERTY(EditAnywhere)
	EQuestConditionParamType QuestConditionParam_0;

	UPROPERTY(EditAnywhere)
	FString QuestConditionParam_1; // 예: Move, Perfect

	/* 퀘스트 요구 횟수 (ex. 1, 5) */
	UPROPERTY(EditAnywhere)
	int32 QuestCondition_Count;
};

USTRUCT()
struct FActiveQuestData
{
	GENERATED_BODY()
	
	/** Quest ID to reference in the Quest Data Table */
	UPROPERTY()
	FString QuestID;
	
	UPROPERTY()
	bool bIsCompleted;
	
	UPROPERTY()
	EQuestConditionType QuestCondition;

	UPROPERTY()
	EQuestConditionParamType QuestConditionParam_0;

	UPROPERTY()
	FString QuestConditionParam_1; // 예: Move, Perfect

	/* 퀘스트 요구 횟수 (ex. 1, 5) */
	UPROPERTY()
	int32 QuestCondition_Count;
	
	FActiveQuestData();
};