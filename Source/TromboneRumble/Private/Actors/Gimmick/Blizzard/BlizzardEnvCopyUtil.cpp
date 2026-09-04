// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Blizzard/BlizzardEnvCopyUtil.h"
#include "Components/SceneComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "UObject/UnrealType.h"
#include "UObject/Package.h"

namespace
{
	/** 복사/구동 대상에서 제외할 프로퍼티 이름.
	 *  - bAffectsWorld: 템플릿은 이걸 false 로 들고 있어(invisible 안전용) 라이브에 복사되면 라이트가 꺼진다.
	 *  - SkyLight 캡처 입력들: 직접 써도 즉시 반영 안 되고, 리캡처를 유발하면 히칭 (현재 코드가 피하는 바로 그것). */
	const TSet<FName>& DeniedPropertyNames()
	{
		static const TSet<FName> Names = {
			FName(TEXT("bAffectsWorld")),
			FName(TEXT("SourceType")),
			FName(TEXT("Cubemap")),
			FName(TEXT("CubemapResolution")),
			FName(TEXT("SourceCubemapAngle")),
			FName(TEXT("SkyDistanceThreshold")),
			FName(TEXT("bCaptureEmissiveOnly")),
			FName(TEXT("bRealTimeCapture")),
		};
		return Names;
	}

	bool ShouldDriveProperty(const FProperty* Prop)
	{
		if (!Prop) return false;

		// USceneComponent 를 "상속한" 클래스에서 선언된 것만. (베이스의 트랜스폼/어태치/가시성/틱 제외)
		const UClass* OwnerClass = Prop->GetOwnerClass();
		if (!OwnerClass) return false;
		if (OwnerClass == USceneComponent::StaticClass()) return false;
		if (!OwnerClass->IsChildOf(USceneComponent::StaticClass())) return false;

		// 에디터 저작(EditAnywhere 등) 또는 interp(라이트 Intensity/LightColor 처럼 interp-only 인 것) 만.
		if (!Prop->HasAnyPropertyFlags(CPF_Edit | CPF_Interp)) return false;

		if (Prop->HasAnyPropertyFlags(CPF_EditConst | CPF_Transient | CPF_DuplicateTransient
			| CPF_NonPIEDuplicateTransient | CPF_Deprecated | CPF_EditorOnly)) return false;

		if (Prop->HasAnyPropertyFlags(CPF_InstancedReference | CPF_ContainsInstancedReference)) return false;

		// 오브젝트/클래스/소프트/위크 참조는 v1 에서 구동 대상 제외 (텍스처/머티리얼 스왑은 팝핑·GC 위험).
		if (CastField<FObjectPropertyBase>(Prop) != nullptr) return false;

		if (DeniedPropertyNames().Contains(Prop->GetFName())) return false;

		return true;
	}

	/** 보간 가능한 leaf/struct 값을 A,B 사이 Alpha 로 보간해 Out 에 기록 (재귀). */
	void LerpPropertyValue(const FProperty* Prop, const void* A, const void* B, void* Out, float Alpha)
	{
		if (const FNumericProperty* Num = CastField<FNumericProperty>(Prop))
		{
			const double a = Num->GetFloatingPointPropertyValue(A);
			const double b = Num->GetFloatingPointPropertyValue(B);
			Num->SetFloatingPointPropertyValue(Out, FMath::Lerp(a, b, static_cast<double>(Alpha)));
			return;
		}

		if (const FStructProperty* Struct = CastField<FStructProperty>(Prop))
		{
			if (Struct->Struct == TBaseStructure<FLinearColor>::Get())
			{
				*static_cast<FLinearColor*>(Out) = FMath::Lerp(
					*static_cast<const FLinearColor*>(A), *static_cast<const FLinearColor*>(B), Alpha);
				return;
			}
			if (Struct->Struct == TBaseStructure<FColor>::Get())
			{
				const FLinearColor la(*static_cast<const FColor*>(A));
				const FLinearColor lb(*static_cast<const FColor*>(B));
				*static_cast<FColor*>(Out) = FMath::Lerp(la, lb, Alpha).ToFColor(true);
				return;
			}
			for (TFieldIterator<FProperty> It(Struct->Struct); It; ++It)
			{
				const FProperty* Member = *It;
				LerpPropertyValue(Member,
					Member->ContainerPtrToValuePtr<void>(A),
					Member->ContainerPtrToValuePtr<void>(B),
					Member->ContainerPtrToValuePtr<void>(Out),
					Alpha);
			}
		}
	}
}

const TArray<const FProperty*>& FBlizzardEnvCopyUtil::GetDrivenProperties(const UClass* ComponentClass)
{
	static TMap<const UClass*, TArray<const FProperty*>> Cache;

	if (const TArray<const FProperty*>* Found = Cache.Find(ComponentClass))
	{
		return *Found;
	}

	TArray<const FProperty*> List;
	if (ComponentClass)
	{
		for (TFieldIterator<FProperty> It(ComponentClass); It; ++It)
		{
			if (ShouldDriveProperty(*It))
			{
				List.Add(*It);
			}
		}
	}
	return Cache.Add(ComponentClass, MoveTemp(List));
}

bool FBlizzardEnvCopyUtil::IsLerpable(const FProperty* Prop)
{
	if (const FNumericProperty* Num = CastField<FNumericProperty>(Prop))
	{
		return Num->IsFloatingPoint();
	}
	if (const FStructProperty* Struct = CastField<FStructProperty>(Prop))
	{
		if (Struct->Struct == TBaseStructure<FLinearColor>::Get()) return true;
		if (Struct->Struct == TBaseStructure<FColor>::Get()) return true;

		bool bAnyMember = false;
		for (TFieldIterator<FProperty> It(Struct->Struct); It; ++It)
		{
			if (!IsLerpable(*It)) return false;
			bAnyMember = true;
		}
		return bAnyMember;
	}
	return false;
}

bool FBlizzardEnvCopyUtil::CopyProperties(const USceneComponent* Src, USceneComponent* Dst)
{
	if (!Src || !Dst || !Src->IsA(Dst->GetClass())) return false;

	bool bChanged = false;
	for (const FProperty* Prop : GetDrivenProperties(Dst->GetClass()))
	{
		const void* S = Prop->ContainerPtrToValuePtr<void>(Src);
		void* D = Prop->ContainerPtrToValuePtr<void>(Dst);
		if (Prop->Identical(S, D)) continue;
		Prop->CopyCompleteValue(D, S);
		bChanged = true;
	}
	return bChanged;
}

bool FBlizzardEnvCopyUtil::CopyOverriddenProperties(const USceneComponent* Src, USceneComponent* Dst)
{
	if (!Src || !Dst || !Src->IsA(Dst->GetClass())) return false;

	const USceneComponent* CDO = Cast<USceneComponent>(Src->GetClass()->GetDefaultObject());
	if (!CDO) return false;

	bool bChanged = false;
	for (const FProperty* Prop : GetDrivenProperties(Src->GetClass()))
	{
		const void* S = Prop->ContainerPtrToValuePtr<void>(Src);
		const void* C = Prop->ContainerPtrToValuePtr<void>(CDO);
		if (Prop->Identical(S, C)) continue;  // 아티스트가 안 건드림 (= 클래스 기본값) → 오버레이 대상 아님

		void* D = Prop->ContainerPtrToValuePtr<void>(Dst);
		if (Prop->Identical(S, D)) continue;
		Prop->CopyCompleteValue(D, S);
		bChanged = true;
	}
	return bChanged;
}

bool FBlizzardEnvCopyUtil::ApplyNonLerpable(const USceneComponent* Target, USceneComponent* Live)
{
	if (!Target || !Live || !Target->IsA(Live->GetClass())) return false;

	bool bChanged = false;
	for (const FProperty* Prop : GetDrivenProperties(Live->GetClass()))
	{
		if (IsLerpable(Prop)) continue;
		const void* T = Prop->ContainerPtrToValuePtr<void>(Target);
		void* L = Prop->ContainerPtrToValuePtr<void>(Live);
		if (Prop->Identical(T, L)) continue;
		Prop->CopyCompleteValue(L, T);
		bChanged = true;
	}
	return bChanged;
}

bool FBlizzardEnvCopyUtil::LerpProperties(const USceneComponent* Start, const USceneComponent* Target,
                                          USceneComponent* Live, float Alpha, const TSet<FName>* ExcludeNames)
{
	if (!Start || !Target || !Live) return false;
	if (!Start->IsA(Live->GetClass()) || !Target->IsA(Live->GetClass())) return false;

	bool bChanged = false;
	for (const FProperty* Prop : GetDrivenProperties(Live->GetClass()))
	{
		if (!IsLerpable(Prop)) continue;
		if (ExcludeNames && ExcludeNames->Contains(Prop->GetFName())) continue;

		const void* A = Prop->ContainerPtrToValuePtr<void>(Start);
		const void* B = Prop->ContainerPtrToValuePtr<void>(Target);
		if (Prop->Identical(A, B)) continue;  // 시작==목표 → 애니메이션할 것 없음

		void* Out = Prop->ContainerPtrToValuePtr<void>(Live);
		LerpPropertyValue(Prop, A, B, Out, Alpha);
		bChanged = true;
	}
	return bChanged;
}

const TSet<FName>& FBlizzardEnvCopyUtil::GetHotPropNames(const USceneComponent* Comp)
{
	// SkyLight: 프록시 재생성 없는 fast-path setter 가 있는 프로퍼티들.
	static const TSet<FName> SkyLightHot = {
		FName(TEXT("Intensity")),
		FName(TEXT("LightColor")),
		FName(TEXT("IndirectLightingIntensity")),
		FName(TEXT("VolumetricScatteringIntensity")),
		FName(TEXT("LowerHemisphereColor")),
	};
	// Directional/Point 라이트: Intensity/LightColor 는 UpdateColorAndBrightness fast-path.
	static const TSet<FName> LightHot = {
		FName(TEXT("Intensity")),
		FName(TEXT("LightColor")),
	};
	static const TSet<FName> Empty;

	if (Comp && Comp->IsA<USkyLightComponent>()) return SkyLightHot;
	if (Comp && Comp->IsA<ULightComponent>()) return LightHot;
	return Empty;
}

void FBlizzardEnvCopyUtil::ApplyHotProps(const USceneComponent* Start, const USceneComponent* Target,
                                         USceneComponent* Live, float Alpha)
{
	// GetHotPropNames() 와 짝. 여기서 setter 로 처리하는 프로퍼티는 거기서 LerpProperties 제외 목록에 든다.
	if (USkyLightComponent* SkyLive = Cast<USkyLightComponent>(Live))
	{
		const USkyLightComponent* S = Cast<USkyLightComponent>(Start);
		const USkyLightComponent* T = Cast<USkyLightComponent>(Target);
		if (!S || !T) return;

		SkyLive->SetIntensity(FMath::Lerp(S->Intensity, T->Intensity, Alpha));
		SkyLive->SetLightColor(FMath::Lerp(FLinearColor(S->LightColor), FLinearColor(T->LightColor), Alpha));
		SkyLive->SetIndirectLightingIntensity(FMath::Lerp(S->IndirectLightingIntensity, T->IndirectLightingIntensity, Alpha));
		SkyLive->SetVolumetricScatteringIntensity(FMath::Lerp(S->VolumetricScatteringIntensity, T->VolumetricScatteringIntensity, Alpha));
		SkyLive->SetLowerHemisphereColor(FMath::Lerp(S->LowerHemisphereColor, T->LowerHemisphereColor, Alpha));
		return;
	}

	if (ULightComponent* LightLive = Cast<ULightComponent>(Live))
	{
		const ULightComponent* S = Cast<ULightComponent>(Start);
		const ULightComponent* T = Cast<ULightComponent>(Target);
		if (!S || !T) return;

		LightLive->SetIntensity(FMath::Lerp(S->Intensity, T->Intensity, Alpha));
		LightLive->SetLightColor(FMath::Lerp(FLinearColor(S->LightColor), FLinearColor(T->LightColor), Alpha));
	}
}

USceneComponent* FBlizzardEnvCopyUtil::CreateSnapshot(const USceneComponent* Live)
{
	if (!Live) return nullptr;

	UClass* Cls = Live->GetClass();
	// Outer=TransientPackage → GetWorld()==null → SkyLight 캡처 큐가 월드 불일치로 처리 스킵 (히칭 회피).
	// 등록하지 않으므로 렌더 스테이트도 안 만든다 (순수 값 보관용).
	USceneComponent* Snapshot = NewObject<USceneComponent>(GetTransientPackage(), Cls, NAME_None, RF_Transient);
	if (Snapshot)
	{
		CopyProperties(Live, Snapshot);
	}
	return Snapshot;
}
