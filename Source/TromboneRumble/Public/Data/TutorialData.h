#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TutorialData.generated.h"

UENUM()
enum class ETutorialSequenceType : uint8
{
	/** Shows a dialogue */
	Dialogue,
	
	/** Do a quest */
	Quest,
	
	/** Shows transition effect */
	Transition,
};

UENUM()
enum class ETromboneTransitionType : uint8
{
	None,
	FadeInOut,
	CurtainCall,
};

UENUM()
enum class ETutorialExtraDataType : uint8
{
	None,
	Image,
	Video,
};

USTRUCT(BlueprintType)
struct FTutorialData : public FTableRowBase
{
	GENERATED_BODY()

	/** Proceed with the tutorial in ID order */
	UPROPERTY(EditAnywhere)
	int32 TID;
	
	/** Internal comment */
	UPROPERTY(EditAnywhere)
	FString Comment;
	
	/** Type of the tutorial sequence */
	UPROPERTY(EditAnywhere)
	ETutorialSequenceType SequenceType;
	
	/** Dialogue ID to reference in the String Table */
	UPROPERTY(EditAnywhere)
	FString DialogueStringID;
	
	/** Quest ID to reference in the Quest Data Table */
	UPROPERTY(EditAnywhere)
	TArray<FString> QuestID;
	
	/** Transition effect type */
	UPROPERTY(EditAnywhere)
	ETromboneTransitionType TransitionType;
	
	/** Additional data to display */
	UPROPERTY(EditAnywhere)
	ETutorialExtraDataType ExtraDataType;
	
	/** Path to the extra data (korean)*/
	UPROPERTY(EditAnywhere)
	FString ExtraDataPath_ko;
		
	/** Path to the extra data (english)*/
	UPROPERTY(EditAnywhere)
	FString ExtraDataPath_en;
};
