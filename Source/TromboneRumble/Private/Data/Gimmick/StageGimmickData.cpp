// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Data/Gimmick/StageGimmickData.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "Utilities/EnumHelper.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

const UGimmickConfig* UStageGimmickData::FindConfig(const EGimmickType GimmickType) const
{
	for (const UGimmickConfig* Config : Gimmicks)
	{
		if (Config && Config->GetGimmickType() == GimmickType)
		{
			return Config;
		}
	}
	return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UStageGimmickData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	TSet<EGimmickType> SeenTypes;
	for (int32 Index = 0; Index < Gimmicks.Num(); ++Index)
	{
		const UGimmickConfig* Config = Gimmicks[Index];
		if (!Config)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Gimmicks[%d] is empty"), Index)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		// FindConfig returns the first match, so a second entry of the same type would never be read
		bool bAlreadySeen = false;
		SeenTypes.Add(Config->GetGimmickType(), &bAlreadySeen);
		if (bAlreadySeen)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Gimmicks[%d] repeats the type %s"), Index, *EnumHelper::EnumToString(Config->GetGimmickType()))));
			Result = EDataValidationResult::Invalid;
		}

		const uint32 ErrorsBefore = Context.GetNumErrors();
		Config->ValidateConfig(Context);
		if (Context.GetNumErrors() > ErrorsBefore)
		{
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif
