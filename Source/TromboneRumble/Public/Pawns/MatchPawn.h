#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MatchPawn.generated.h"

class UNameplateComponent;
class UCapsuleComponent;
class UArrowComponent;
class UWidgetComponent;
class UTromboneVOIPTalker;
class APlayerStart;
class UCustomizationComponent;
class UMaterialInterface;

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

	// 커스터마이징용 페이스 머티리얼 교체. nullptr 전달 시 원본 머티리얼로 복원
	void ApplyFaceMaterial(UMaterialInterface* Material);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCustomizationComponent> CustomizationComp;
	
protected:
	
	/** Pawn collision component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	/** Pawn skeletal mesh component. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
	
	/** Pawn nameplate component. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNameplateComponent> NameplateComponent;
	
#if WITH_EDITORONLY_DATA
	/** Pawn arrow component. */
	UPROPERTY()
	UArrowComponent* ArrowComponent;
#endif
	
	/** Voice chat talker — configured for 2D (omnidirectional) playback in MatchMenuMap. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Voice")
	TObjectPtr<UTromboneVOIPTalker> VOIPTalker;

	/** Per-pawn voice volume slider — own pawn: sender volume; other pawns: listener adjustment. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Voice")
	TObjectPtr<UWidgetComponent> VoiceSliderComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> OriginalFaceMaterial;

	void TryRegisterVOIPTalker();
	void TryInitVoiceSlider();
	bool bVoiceSliderInitialized = false;

	FTimerHandle RetryVOIPRegistrationHandle;

public:
	/** @return The voice talker for this pawn, so UI widgets can bind to its talking-state delegate. */
	UTromboneVOIPTalker* GetVOIPTalker() const { return VOIPTalker; }

	// UVOIPTalker::OnTalkingBegin은 Listener에게만 적용되기 때문에, RPC를 통해 자기표시(PTT) 상태를 전파.
	// True인 경우에는 해당 플레이어가 PushToTalk 모드를 사용해서 말을 하고 있음.
	UFUNCTION(Server, Reliable)
	void Server_SetSpeaking(bool bSpeaking);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetSpeaking(bool bSpeaking);


public:

	//~ Begin APawn Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface
	
	// 모듈러 커스터마이징의 leader(메인) 메시 — CustomizationComponent가 follower를 붙일 대상
	USkeletalMeshComponent* GetMeshComponent() const { return SkeletalMeshComponent; }

};
