// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuideSignalComponent.generated.h"

class AGuideLine;

/** UGuideSignalComponent
 *
 * UGuideSignalComponent holds one replicated bool that tells guide lines to show or hide.
 * An actor that guides players creates it, for example ABlizzardShelter or ABeerFloodGimmick.
 * The server sets the bool, and every machine applies it to the lines registered here.
 *
 * Keep in mind that the owner must be a replicated actor, or clients never get the value.
 *
 * @see AGuideLine
 */
UCLASS(ClassGroup = (Gimmick), meta = (BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UGuideSignalComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UGuideSignalComponent();

	/**
	 * Show or hide every guide line registered to this component.
	 * Only the server can call this. The value is replicated, so clients apply it too.
	 *
	 * @param bInGuiding True shows the lines, false hides them.
	 */
	void SetGuiding(bool bInGuiding);

	/** @return Whether the registered lines are shown right now. */
	bool IsGuiding() const { return bGuiding; }

	/**
	 * Add a line and apply the current value to it at once.
	 * A client that joins while the value is true gets the right state this way.
	 */
	void RegisterLine(AGuideLine* Line);

	/** Remove a line. Called from AGuideLine::EndPlay. */
	void UnregisterLine(AGuideLine* Line);

private:

	/** Apply the new value to the lines on a client. */
	UFUNCTION()
	void OnRep_Guiding();

	/** Call SetGuideVisible with bGuiding on every registered line. */
	void ApplyToLines() const;

	/** Are the registered lines shown. */
	UPROPERTY(ReplicatedUsing = OnRep_Guiding)
	bool bGuiding = false;

	/** Lines that use the owner as their Source. */
	TArray<TWeakObjectPtr<AGuideLine>> Lines;

public:

	//~ Begin UActorComponent Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UActorComponent Interface
};
