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

	/** Voice chat talker — configured for 2D (omnidirectional) playback in MatchMenuMap. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Voice")
	TObjectPtr<UTromboneVOIPTalker> VOIPTalker;

	/** Widget shown above the pawn while this player is speaking. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Voice")
	TObjectPtr<UWidgetComponent> SpeakerIndicatorComponent;

	/** Per-pawn voice volume slider — own pawn: sender volume; other pawns: listener adjustment. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Voice")
	TObjectPtr<UWidgetComponent> VoiceSliderComponent;

	void TryRegisterVOIPTalker();
	void TryInitVoiceSlider();
	bool bVoiceSliderInitialized = false;

	FTimerHandle RetryVOIPRegistrationHandle;

	UFUNCTION()
	void HandleVoiceTalkingStateChanged(bool bIsTalking);
	
	bool bDesiredSpeakingByPTT = false;
	void SetSpeakerIconVisible(bool bVisible);

public:
	// UVOIPTalker::OnTalkingBegin은 Listener에게만 적용되기 때문에, RPC를 통해 SpeakerIcon을 제어
	// True인 경우에는 해당 플레이어가 PushToTalk 모드를 사용해서 말을 하고 있음.
	UFUNCTION(Server, Reliable)
	void Server_SetSpeaking(bool bSpeaking);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetSpeaking(bool bSpeaking);

#if WITH_EDITORONLY_DATA
	/** Pawn arrow component. */
	UPROPERTY()
	UArrowComponent* ArrowComponent;
#endif
	
public:

	//~ Begin APawn Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface
	
};
