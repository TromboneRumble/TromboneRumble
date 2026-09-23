// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GuideLine.generated.h"

class UGuideSignalComponent;
class UNiagaraComponent;

/** AGuideLine
 *
 * AGuideLine is a particle line that shows players where to go.
 * It is placed in the level by hand, one Blueprint instance (BP_Guideline) per path.
 * The UGuideSignalComponent of the Source actor activates and deactivates it.
 * Many lines can use the same Source.
 *
 * Keep in mind that the Blueprint owns the spline and the Niagara component.
 * A component created in C++ would become the root and move the spline under it.
 *
 * @see UGuideSignalComponent
 */
UCLASS()
class TROMBONERUMBLE_API AGuideLine : public AActor
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	AGuideLine();

	/**
	 * Activate or deactivate the Niagara component.
	 * Deactivating only stops new particles, so particles already spawned still reach the end of the spline.
	 */
	void SetGuideVisible(bool bVisible);

private:

	/** Actor whose UGuideSignalComponent this line follows. */
	UPROPERTY(EditInstanceOnly, Category = "Config|GuideLine", meta = (AllowPrivateAccess = "true", DisplayName = "안내 신호 액터"))
	TSoftObjectPtr<AActor> Source;

	/** Niagara component of the Blueprint. Found in PreInitializeComponents. */
	TWeakObjectPtr<UNiagaraComponent> GuideFX;

	/** Signal component of Source. Kept to unregister in EndPlay. */
	TWeakObjectPtr<UGuideSignalComponent> Signal;

public:

	//~ Begin AActor Interface
	virtual void PreInitializeComponents() override;
	//~ End AActor Interface

protected:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};
