#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CustomizePawn.generated.h"

class UCapsuleComponent;
class UCustomizationComponent;

UCLASS()
class TROMBONERUMBLE_API ACustomizePawn : public APawn
{
	GENERATED_BODY()

public:
	ACustomizePawn();

	void ApplyFaceMaterial(UMaterialInterface* Material);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCustomizationComponent> CustomizationComp;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 SkinMaterialIndex = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 FaceMaterialIndex = 2;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> OriginalFaceMaterial;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

private:
	void UpdateSkinFromPlayerState() const;
};
