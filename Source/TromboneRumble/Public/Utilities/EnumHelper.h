#pragma once

class EnumHelper
{
public:
	template <typename TEnum>
	static FString EnumToString(TEnum EnumValue)
	{
		static_assert(TIsEnum<TEnum>::Value, "EnumToString: TEnum must be an enum type.");

		const UEnum* EnumPtr = StaticEnum<TEnum>();
		if (!EnumPtr)
		{
			return FString("InvalidEnum");
		}

		return EnumPtr->GetNameStringByValue(static_cast<int64>(EnumValue));
	}

	template <typename TEnum>
	static int32 EnumToInt(TEnum EnumValue)
	{
		static_assert(TIsEnum<TEnum>::Value, "EnumToInt: TEnum must be an enum type.");

		return static_cast<int32>(EnumValue);
	}
};
