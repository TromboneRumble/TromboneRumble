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

const FGimmickSequence* UStageGimmickData::FindSequence(const EGimmickType GimmickType) const
{
	return Sequences.FindByPredicate([GimmickType](const FGimmickSequence& Sequence) { return Sequence.Order.Contains(GimmickType); });
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

	const auto AddError = [&Context, &Result](const FString& Message)
	{
		Context.AddError(FText::FromString(Message));
		Result = EDataValidationResult::Invalid;
	};

	TSet<EGimmickType> SequencedTypes;
	for (int32 Index = 0; Index < Sequences.Num(); ++Index)
	{
		const FGimmickSequence& Sequence = Sequences[Index];
		if (Sequence.Order.IsEmpty())
		{
			AddError(FString::Printf(TEXT("순서 그룹 %d: 순서가 비어 있습니다"), Index));
			continue;
		}

		TSet<EGimmickType> TypesInThis;
		for (const EGimmickType Type : Sequence.Order)
		{
			const FString TypeName = UEnum::GetDisplayValueAsText(Type).ToString();

			const UGimmickConfig* Config = FindConfig(Type);
			if (!Config)
			{
				AddError(FString::Printf(TEXT("순서 그룹 %d: %s 이(가) 순서에 있지만 기믹 목록에 없습니다"), Index, *TypeName));
				continue;
			}
			if (!Config->RunsOnlyInSequence())
			{
				AddError(FString::Printf(TEXT("순서 그룹 %d: %s 은(는) 자기 타이머로 도는 기믹이라 순서 그룹에 넣을 수 없습니다"), Index, *TypeName));
			}

			// The same gimmick twice in one order is fine, but one gimmick cannot follow two orders
			bool bInThisAlready = false;
			TypesInThis.Add(Type, &bInThisAlready);
			if (!bInThisAlready && SequencedTypes.Contains(Type))
			{
				AddError(FString::Printf(TEXT("순서 그룹 %d: %s 이(가) 다른 순서 그룹에도 들어 있습니다"), Index, *TypeName));
			}
		}
		SequencedTypes.Append(TypesInThis);
	}

	for (const UGimmickConfig* Config : Gimmicks)
	{
		if (Config && Config->RunsOnlyInSequence() && !SequencedTypes.Contains(Config->GetGimmickType()))
		{
			AddError(FString::Printf(TEXT("%s 은(는) 순서 그룹에서만 발동하는데, 어느 순서 그룹에도 없습니다"), *UEnum::GetDisplayValueAsText(Config->GetGimmickType()).ToString()));
		}
	}

	return Result;
}
#endif
