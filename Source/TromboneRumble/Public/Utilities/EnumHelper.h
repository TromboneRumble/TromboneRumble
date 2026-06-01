#pragma once

/**
 * Utility class providing static helper functions for Unreal Engine Enumerations (UEnum).
 */
class EnumHelper
{
public:
    /**
     * Converts an Enum value to its string representation.
     * @param EnumValue The enum value to convert.
     * @return The string name of the enum (e.g., "Violin"). Returns "InvalidEnum" if conversion fails.
     */
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

    /**
     * Casts an Enum value to its underlying integer type (int32).
     * @param EnumValue The enum value to cast.
     * @return The integer value representing the enum index.
     */
	template <typename TEnum>
	static int32 EnumToInt(TEnum EnumValue)
	{
		static_assert(TIsEnum<TEnum>::Value, "EnumToInt: TEnum must be an enum type.");

		return static_cast<int32>(EnumValue);
	}
    
    /** * Parses a string to find and return the corresponding Enum value.
     * @param StringValue The string input to parse (e.g., "Violin" or "EInstrumentType::Violin").
     * @param OutValue [Output] The mapped Enum value if successful.
     * @return True if a matching enum value was found, otherwise False.
     */
    template <typename TEnum>
    static bool StringToEnum(const FString& StringValue, TEnum& OutValue)
    {
		static_assert(TIsEnum<TEnum>::Value, "StringToEnum: TEnum must be an enum type.");

		const UEnum* EnumPtr = StaticEnum<TEnum>();
		if (!EnumPtr)
		{
			return false;
		}

		int64 EnumValue = EnumPtr->GetValueByName(FName(*StringValue));

		if (EnumValue == INDEX_NONE)
		{
			FString FullName = EnumPtr->GetName() + TEXT("::") + StringValue;
			EnumValue = EnumPtr->GetValueByName(FName(*FullName));
		}

		if (EnumValue != INDEX_NONE)
		{
			OutValue = static_cast<TEnum>(EnumValue);
			return true;
		}

		return false;
    }
	
	/**
	 * Converts an integer value back to an Enum value.
	 * @param IntValue The integer index to convert.
	 * @param OutValue [Output] The mapped Enum value if it exists.
	 * @return True if the integer is a valid index within the Enum, otherwise False.
	 */
	template <typename TEnum>
	static bool IntToEnum(int32 IntValue, TEnum& OutValue)
	{
		static_assert(TIsEnum<TEnum>::Value, "IntToEnum: TEnum must be an enum type.");

		const UEnum* EnumPtr = StaticEnum<TEnum>();
		if (!EnumPtr)
		{
			return false;
		}

		if (EnumPtr->IsValidEnumValue(static_cast<int64>(IntValue)))
		{
			OutValue = static_cast<TEnum>(IntValue);
			return true;
		}

		return false;
	}
};