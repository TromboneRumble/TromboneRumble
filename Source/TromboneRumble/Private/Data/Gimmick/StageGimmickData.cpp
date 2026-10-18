// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Data/Gimmick/StageGimmickData.h"
#include "Data/Gimmick/GimmickConfig.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "UObject/UObjectHash.h"
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
			Context.AddError(FText::FromString(FString::Printf(TEXT("기믹 목록 %d번: 기믹 종류가 비어 있습니다. 종류를 고르거나 항목을 지우세요"), Index + 1)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		// FindConfig returns the first match, so a second entry of the same type would never be read
		bool bAlreadySeen = false;
		SeenTypes.Add(Config->GetGimmickType(), &bAlreadySeen);
		if (bAlreadySeen)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("기믹 목록 %d번: %s 이(가) 이미 목록에 있습니다. 같은 종류는 첫 항목만 쓰입니다"), Index + 1, *UEnum::GetDisplayValueAsText(Config->GetGimmickType()).ToString())));
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
			AddError(FString::Printf(TEXT("기믹 시퀀스 %d: 시퀀스 순서가 비어 있습니다"), Index));
			continue;
		}

		TSet<EGimmickType> TypesInThis;
		for (const EGimmickType Type : Sequence.Order)
		{
			const FString TypeName = UEnum::GetDisplayValueAsText(Type).ToString();

			const UGimmickConfig* Config = FindConfig(Type);
			if (!Config)
			{
				AddError(FString::Printf(TEXT("기믹 시퀀스 %d: %s 이(가) 시퀀스 순서에 있지만 기믹 목록에 없습니다"), Index, *TypeName));
				continue;
			}
			if (!Config->RunsOnlyInSequence())
			{
				AddError(FString::Printf(TEXT("기믹 시퀀스 %d: %s 은(는) 자기 타이머로 도는 기믹이라 기믹 시퀀스에 넣을 수 없습니다"), Index, *TypeName));
			}

			// The same gimmick twice in one order is fine, but one gimmick cannot follow two orders
			bool bInThisAlready = false;
			TypesInThis.Add(Type, &bInThisAlready);
			if (!bInThisAlready && SequencedTypes.Contains(Type))
			{
				AddError(FString::Printf(TEXT("기믹 시퀀스 %d: %s 이(가) 다른 기믹 시퀀스에도 들어 있습니다"), Index, *TypeName));
			}
		}
		SequencedTypes.Append(TypesInThis);

		// FindSequence only looks at Order, so a gimmick only in FeverOrder would still start on its own
		if (Sequence.bChangeOrderInFever)
		{
			for (const EGimmickType Type : Sequence.FeverOrder)
			{
				if (!TypesInThis.Contains(Type))
				{
					AddError(FString::Printf(TEXT("기믹 시퀀스 %d: %s 이(가) 피버 타임 시 시퀀스 순서에 있지만 시퀀스 순서에는 없습니다"), Index, *UEnum::GetDisplayValueAsText(Type).ToString()));
				}
			}
		}
	}

	for (const UGimmickConfig* Config : Gimmicks)
	{
		if (Config && Config->RunsOnlyInSequence() && !SequencedTypes.Contains(Config->GetGimmickType()))
		{
			AddError(FString::Printf(TEXT("%s 은(는) 기믹 시퀀스에서만 발동하는데, 어느 기믹 시퀀스에도 없습니다"), *UEnum::GetDisplayValueAsText(Config->GetGimmickType()).ToString()));
		}
	}

	return Result;
}

TArray<FString> UStageGimmickData::GetNonSequenceGimmickTypes() const
{
	TSet<EGimmickType> Allowed;
	for (const UGimmickConfig* Config : Gimmicks)
	{
		if (Config && Config->RunsOnlyInSequence())
		{
			Allowed.Add(Config->GetGimmickType());
		}
	}

	TArray<FString> Names;
	const UEnum* Enum = StaticEnum<EGimmickType>();
	for (int32 Index = 0; Index < Enum->NumEnums() - 1; ++Index)
	{
		if (!Allowed.Contains(static_cast<EGimmickType>(Enum->GetValueByIndex(Index))))
		{
			Names.Add(Enum->GetNameStringByIndex(Index));
		}
	}
	return Names;
}

TArray<UClass*> UStageGimmickData::GetAddableGimmickClasses() const
{
	TSet<EGimmickType> UsedTypes;
	for (const UGimmickConfig* Config : Gimmicks)
	{
		if (Config)
		{
			UsedTypes.Add(Config->GetGimmickType());
		}
	}

	TArray<UClass*> ConfigClasses;
	GetDerivedClasses(UGimmickConfig::StaticClass(), ConfigClasses);

	// The picker also lets the children of an allowed class through, so the ice can come back when only the water drop is free
	TArray<UClass*> Addable;
	for (UClass* Class : ConfigClasses)
	{
		if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) continue;
		if (!UsedTypes.Contains(Class->GetDefaultObject<UGimmickConfig>()->GetGimmickType()))
		{
			Addable.Add(Class);
		}
	}
	return Addable;
}
#endif
