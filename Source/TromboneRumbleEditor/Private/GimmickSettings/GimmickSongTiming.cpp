// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/GimmickSongTiming.h"
#include "GimmickSettings/GimmickSettingsCommon.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Data/RhythmSongDataRow.h"
#include "DeveloperSettings/GameMapDeveloperSettings.h"
#include "Dom/JsonObject.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Framework/TromboneGameInstance.h"
#include "GameMapsSettings.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Subsystems/RhythmSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "WwiseUnrealHelper.h"
#include "XmlFile.h"

namespace
{
	/** Depth first search for the first node with this tag whose attribute has this value. */
	const FXmlNode* FindNode(const FXmlNode* Node, const FString& Tag, const FString& Attribute, const FString& Value)
	{
		if (!Node) return nullptr;
		if (Node->GetTag() == Tag && Node->GetAttribute(Attribute) == Value) return Node;

		for (const FXmlNode* Child : Node->GetChildrenNodes())
		{
			if (const FXmlNode* Found = FindNode(Child, Tag, Attribute, Value)) return Found;
		}
		return nullptr;
	}

	/** @return ID of the object the event plays, or empty when the events work unit has no such event. */
	FString FindEventTargetId(const FXmlFile& EventsFile, const FString& EventName)
	{
		const FXmlNode* Event = FindNode(EventsFile.GetRootNode(), TEXT("Event"), TEXT("Name"), EventName);
		const FXmlNode* Reference = Event ? FindNode(Event, TEXT("Reference"), TEXT("Name"), TEXT("Target")) : nullptr;
		const FXmlNode* Target = Reference ? Reference->FindChildNode(TEXT("ObjectRef")) : nullptr;
		return Target ? Target->GetAttribute(TEXT("ID")) : FString();
	}

	/**
	 * @return Seconds from the start of the segment to the named cue, or a negative value when the segment has no such cue.
	 *         A cue without a TimeMs property sits at 0.
	 */
	float FindCueTime(const FXmlNode& Segment, const FString& CueName)
	{
		// Only the cue list of the segment itself. Its tracks are child nodes too
		const FXmlNode* Lists = Segment.FindChildNode(TEXT("ObjectLists"));
		const FXmlNode* Cues = Lists ? FindNode(Lists, TEXT("ObjectList"), TEXT("Name"), TEXT("Cues")) : nullptr;

		// Some segments have unnamed cues at the same times, so the cue is found by name
		const FXmlNode* Cue = Cues ? FindNode(Cues, TEXT("MusicCue"), TEXT("Name"), CueName) : nullptr;
		if (!Cue) return -1.f;

		const FXmlNode* Time = FindNode(Cue, TEXT("Property"), TEXT("Name"), TEXT("TimeMs"));
		return Time ? FCString::Atof(*Time->GetAttribute(TEXT("Value"))) / 1000.f : 0.f;
	}

	/** @return DurationMax of every event with an end in SoundbanksInfo.json, keyed by event name. */
	TMap<FString, float> ReadEventDurations(const FString& Path)
	{
		TMap<FString, float> Durations;

		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *Path)) return Durations;

		TSharedPtr<FJsonObject> Root;
		if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root.IsValid()) return Durations;

		const TSharedPtr<FJsonObject>* Info = nullptr;
		const TArray<TSharedPtr<FJsonValue>>* Banks = nullptr;
		if (!Root->TryGetObjectField(TEXT("SoundBanksInfo"), Info) || !(*Info)->TryGetArrayField(TEXT("SoundBanks"), Banks)) return Durations;

		for (const TSharedPtr<FJsonValue>& Bank : *Banks)
		{
			const TArray<TSharedPtr<FJsonValue>>* Events = nullptr;
			if (!Bank->AsObject()->TryGetArrayField(TEXT("Events"), Events)) continue;

			for (const TSharedPtr<FJsonValue>& Event : *Events)
			{
				const TSharedPtr<FJsonObject> Object = Event->AsObject();
				FString Name;
				FString Type;
				FString Max;
				if (!Object->TryGetStringField(TEXT("Name"), Name) || !Object->TryGetStringField(TEXT("DurationMax"), Max)) continue;

				// An infinite event never ends, so it gives no round length
				if (Object->TryGetStringField(TEXT("DurationType"), Type) && Type == TEXT("Infinite")) continue;

				Durations.Add(Name, FCString::Atof(*Max));
			}
		}
		return Durations;
	}

	/** @return Song table the game instance Blueprint points at, the same table the game reads. */
	const UDataTable* LoadSongTable()
	{
		const UClass* InstanceClass = GetDefault<UGameMapsSettings>()->GameInstanceClass.TryLoadClass<UTromboneGameInstance>();
		const UTromboneGameInstance* Defaults = InstanceClass ? InstanceClass->GetDefaultObject<UTromboneGameInstance>() : nullptr;
		return Defaults ? Defaults->RhythmSongDataTableSoft.LoadSynchronous() : nullptr;
	}
}

TArray<FGimmickSongTiming> GimmickSongTiming::CollectSongTimings(TArray<FString>& OutSkipped)
{
	TArray<FGimmickSongTiming> Songs;

	const UDataTable* SongTable = LoadSongTable();
	if (!SongTable)
	{
		OutSkipped.Add(TEXT("곡 테이블을 찾지 못했습니다. 게임 인스턴스의 RhythmSongDataTableSoft를 확인하세요"));
		return Songs;
	}

	const FString ProjectDir = WwiseUnrealHelper::GetWwiseProjectDirectoryPath();
	const FXmlFile EventsFile(ProjectDir / TEXT("Events/Default Work Unit.wwu"));
	const FXmlFile MusicFile(ProjectDir / TEXT("Interactive Music Hierarchy/Default Work Unit.wwu"));
	if (!EventsFile.IsValid() || !MusicFile.IsValid())
	{
		OutSkipped.Add(FString::Printf(TEXT("Wwise 프로젝트의 Work Unit 파일을 읽지 못했습니다 (%s)"), *ProjectDir));
		return Songs;
	}

	// The editor runs on Windows, so the Windows banks are the ones generated on this machine
	const TMap<FString, float> Durations = ReadEventDurations(WwiseUnrealHelper::GetSoundBankDirectory() / TEXT("Windows/SoundbanksInfo.json"));

	SongTable->ForeachRow<FRhythmSongDataRow>(TEXT("GimmickSongTiming"), [&](const FName& RowName, const FRhythmSongDataRow& Row)
	{
		const auto Skip = [&OutSkipped, &Row](const TCHAR* Reason) { OutSkipped.Add(FString::Printf(TEXT("%s: %s"), *Row.SongName, Reason)); };

		// The asset of a Wwise event has the name of the event
		const FString EventName = Row.BgmEvent.ToSoftObjectPath().GetAssetName();

		// A playlist would need the offset of each segment, and no song uses one but the tutorial
		const FString SegmentId = FindEventTargetId(EventsFile, EventName);
		const FXmlNode* Segment = SegmentId.IsEmpty() ? nullptr : FindNode(MusicFile.GetRootNode(), TEXT("MusicSegment"), TEXT("ID"), SegmentId);
		if (!Segment)
		{
			Skip(TEXT("BGM 이벤트가 음악 세그먼트 하나를 바로 재생하지 않습니다"));
			return;
		}

		const float Start = FindCueTime(*Segment, TromboneMusicCues::SpotlightStart);
		const float Fever = FindCueTime(*Segment, TromboneMusicCues::SpotlightFever);
		if (Start < 0.f || Fever < 0.f)
		{
			Skip(TEXT("Event_Spotlight_Start 또는 Event_Spotlight_Fever 큐가 없습니다"));
			return;
		}

		const float* Duration = Durations.Find(EventName);
		if (!Duration)
		{
			Skip(TEXT("SoundBank 정보에 길이가 없습니다. SoundBank를 생성했는지 확인하세요"));
			return;
		}

		Songs.Add({ Row.SongTag, Row.SongName, Fever - Start, *Duration - Start });
	});

	return Songs;
}

TArray<FGameplayTag> GimmickSongTiming::FindSongsOfLevel(const UWorld* World)
{
	if (!World) return {};

	const FString LevelPackage = GimmickSettings::GetLevelPackageName(*World);
	const FGameplayTag InGameCategory = FGameplayTag::RequestGameplayTag(FName(*TromboneGamePlayTags::InGamePath), false);

	for (const TPair<FGameplayTag, FSoftObjectPath>& Pair : GetDefault<UGameMapDeveloperSettings>()->GamePlayMap)
	{
		if (Pair.Key.MatchesTag(InGameCategory) && Pair.Value.GetLongPackageName() == LevelPackage)
		{
			return UTromboneFunctionLibrary::GetSongsForMap(Pair.Key);
		}
	}
	return {};
}
