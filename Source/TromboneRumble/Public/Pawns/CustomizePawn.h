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
protected:
	// face 표정 머티리얼 교체 시 nullptr 복원용 (CustomizeMap은 스킨컬러 틴트 없이 기본 머티리얼 사용)
	UPROPERTY()
	TObjectPtr<UMaterialInterface> OriginalFaceMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	virtual void BeginPlay() override;

public:
	USkeletalMeshComponent* GetMeshComponent() const { return SkeletalMeshComponent; }
};
