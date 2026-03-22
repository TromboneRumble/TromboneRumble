#include "Data/QuestData.h"

FActiveQuestData::FActiveQuestData()
{
	QuestID = "";
	bIsCompleted = false;
	QuestCondition = EQuestConditionType::BasicAction;
	QuestConditionParam_0 = EQuestConditionParamType::Any;
	QuestConditionParam_1 = "";
	QuestCondition_Count = 0;
}