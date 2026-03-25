#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NameplateComponent.generated.h"

class UWidgetComponent;

/**
 * Delegate triggered when the nameplate is created.
 * 
 * @param NameplateWidget The widget that was created.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNameplateCreated, UUserWidget*, NameplateWidget);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API UNameplateComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:

	/** Default constructor. */
	UNameplateComponent();

public:

	/**
	 * The widget to use for the nameplate.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Nameplate")
	TSubclassOf<UUserWidget> NameplateWidgetClass;

	/** Draw size of the nameplate widget's 'canvas'. Does not scale the actual widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Nameplate")
	FVector2D NameplateDrawSize;

	/** Nameplate offset from the center of the actor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Nameplate")
	FVector NameplateOffset;

private:

	/** The component that renders the nameplate widget. */
	UPROPERTY(Transient)
	UWidgetComponent* WidgetComponent;

	/** Event when the nameplate is created. At this point the player state has replicated. */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Nameplate Created", meta = (AllowPrivateAccess))
	FOnNameplateCreated OnNameplateCreatedEvent;

public:
	
	/** @return The delegate fired when the nameplate is created. */
	FOnNameplateCreated& OnNameplateCreated() { return OnNameplateCreatedEvent; }
	
protected:

	/** Check if all necessary objects have been replicated. If not, the check will be performed again in the next frame. */
	virtual void WaitInitialReplication();

	/** Create the nameplate for the player. */
	virtual void CreateNameplate();

	/** Check if the nameplate widget has been created. If not, the check will be performed again in the next frame. */
	virtual void WaitForNameplateWidget();

public:

	//~ Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	//~ End UActorComponent Interface
};
