// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlizzardShelter.generated.h"

class USphereComponent;

/** ABlizzardShelter
 * 눈보라 안전지대. 천막마다 하나씩 배치한다.
 * SafeZone sphere 안에 있는 캐릭터는 눈보라 효과(밀림/슬로우/래그돌)를 받지 않는다.
 * ABlizzardGimmick 이 TActorIterator 로 자동 수집하므로 별도 등록이 필요 없다.
 */
UCLASS()
class TROMBONERUMBLE_API ABlizzardShelter : public AActor
{
	GENERATED_BODY()

public:
	ABlizzardShelter();

	/** WorldLoc(주로 캐릭터 중심)이 안전지대 sphere 내부에 있으면 true */
	bool IsLocationInside(const FVector& WorldLoc) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shelter")
	TObjectPtr<USphereComponent> SafeZone;
};
