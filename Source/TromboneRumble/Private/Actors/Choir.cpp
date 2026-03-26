#include "Actors/Choir.h"

AChoir::AChoir()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
    BookMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BookMesh"));
    BookMesh->SetupAttachment(MeshComponent, BookSocketName);
}

void AChoir::PlayIdle()
{
    if (AnimationData)
    {
        StartAnimationInternal(&AnimationData->IdleAnimation, IdleAnimAsset, false);
    }
}

void AChoir::PlaySing()
{
    if (AnimationData)
    {
        StartAnimationInternal(&AnimationData->SingAnimation, SingAnimAsset, true);
    }
}


void AChoir::BeginPlay()
{
	Super::BeginPlay();
    if (UMaterialInterface* BaseMat = MeshComponent->GetMaterial(FaceMaterialIndex))
    {
        FaceMID = MeshComponent->CreateAndSetMaterialInstanceDynamic(FaceMaterialIndex);
    }

    if (AnimationData)
    {
        CurrentPlayRate = FMath::FRandRange(AnimationData->MinPlayRate, AnimationData->MaxPlayRate);
        float StartOffset = FMath::FRandRange(0.0f, AnimationData->MaxStartOffset);

        GetWorld()->GetTimerManager().SetTimer(TimerHandle_StartDelay, this, &AChoir::InitializeRandomAnimation, StartOffset, false);
    }
}

void AChoir::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(TimerHandle_FaceAnim);
    GetWorldTimerManager().ClearTimer(TimerHandle_StartDelay);
    
    Super::EndPlay(EndPlayReason);
}

void AChoir::InitializeRandomAnimation()
{
    if (FMath::RandBool())
    {
        PlaySing();
    }
    else
    {
        PlaySing();
    }
}

void AChoir::StartAnimationInternal(FChoirAnimationSequence* InFaceSequence, UAnimSequence* InBodyAnim, bool bIsSinging)
{

    if (MeshComponent && InBodyAnim)
    {
        MeshComponent->PlayAnimation(InBodyAnim, true);
        MeshComponent->SetPlayRate(CurrentPlayRate);   
    }

    if (BookMesh)
    {
        BookMesh->SetVisibility(bIsSinging);
    }

    if (InFaceSequence && InFaceSequence->Frames.Num() > 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle_FaceAnim);
        CurrentSequence = InFaceSequence;
        CurrentFrameIndex = 0;
        ProcessNextFrame();
    }
}

void AChoir::ProcessNextFrame()
{
    if (!CurrentSequence) return;

    const FChoirFaceFrame& Frame = CurrentSequence->Frames[CurrentFrameIndex];
    UpdateFaceMaterial(Frame.FaceIndex);

    CurrentFrameIndex++;
    if (CurrentFrameIndex >= CurrentSequence->Frames.Num())
    {
        if (CurrentSequence->bLoop)
        {
            CurrentFrameIndex = 0;
        }
        else
        {
            return;
        }
    }

    // 재생 속도를 반영하여 실제 대기 시간 계산
    // PlayRate가 1.0보다 크면 분모가 커져서 실제 대기 시간이 짧아짐
    float AdjustedDuration = Frame.Duration / CurrentPlayRate;

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_FaceAnim, this, &AChoir::ProcessNextFrame, AdjustedDuration, false);
}

void AChoir::UpdateFaceMaterial(int32 Index)
{
    if (FaceMID)
    {
        FaceMID->SetScalarParameterValue(FaceExpressionParameterName, static_cast<float>(Index));
    }
}



