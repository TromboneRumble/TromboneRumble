#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MatchPawn.generated.h"

class UNameplateComponent;
class UCapsuleComponent;
class UArrowComponent;
class APlayerStart;

/**
 * Pawn class used in the Match menu
 */
UCLASS()
class TROMBONERUMBLE_API AMatchPawn : public APawn
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	AMatchPawn();
	
	// TODO : 스킨 컬러 관련해서 MatchPawn과 TromboneCharacterBase에서 중복되는 코드가 있음. 캐릭터 베이스 클래스 재작성 필요
	/** Apply unique skin color to the pawn */
	void UpdateSkinFromPlayerState() const;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 SkinMaterialIndex = 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 FaceMaterialIndex = 2;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;
	
protected:

	/** Pawn collision component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* CapsuleComponent;
	
	/** Pawn skeletal mesh component. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* SkeletalMeshComponent;
	
	/** Pawn nameplate component. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UNameplateComponent* NameplateComponent;

#if WITH_EDITORONLY_DATA
	/** Pawn arrow component. */
	UPROPERTY()
	UArrowComponent* ArrowComponent;
#endif
	
public:

	//~ Begin APawn Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	//~ End APawn Interface
	
};
