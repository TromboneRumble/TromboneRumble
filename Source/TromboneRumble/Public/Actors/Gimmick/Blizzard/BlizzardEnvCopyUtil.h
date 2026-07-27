// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class USceneComponent;

/** FBlizzardEnvCopyUtil
 * 리플렉션 기반 상태-템플릿 복사/보간 유틸. 게임 스레드 전용.
 *
 * 눈보라 기믹은 상태별(전조/눈보라) 라이팅을 "템플릿 컴포넌트"에 저작하고,
 * 런타임에 그 값을 레벨의 라이브 컴포넌트에 복사/보간해 반영한다.
 * 여기서 다루는 대상은 "USceneComponent 파생 클래스에서 선언된, 에디터/interp 로 저작 가능한
 * 렌더링 프로퍼티"뿐이다 (트랜스폼/가시성/어태치 등 베이스 프로퍼티는 제외).
 *
 * 이렇게 하면 기획/아트가 컴포넌트에 새 값을 조정하고 싶을 때 프로그래머가
 * UPROPERTY + 캡처 + 보간 + 적용 코드를 추가할 필요 없이, 템플릿 컴포넌트에서 값만 바꾸면 된다.
 */
struct FBlizzardEnvCopyUtil
{
	/** ComponentClass 의 구동 대상 프로퍼티 목록 (클래스별 캐시). */
	static const TArray<const FProperty*>& GetDrivenProperties(const UClass* ComponentClass);

	/** 프로퍼티가 수치 보간 가능한지 (float/double, FLinearColor, FColor, 또는 그것들로만 구성된 struct). */
	static bool IsLerpable(const FProperty* Prop);

	/** Src 의 모든 구동 프로퍼티 값을 Dst 에 복사. 하나라도 달랐으면 true.
	 *  (스냅샷 캡처 / 에디터 "월드->템플릿 저장" / "복원" 에 사용) */
	static bool CopyProperties(const USceneComponent* Src, USceneComponent* Dst);

	/** Src 가 자기 클래스 CDO 와 다른(= 아티스트가 명시적으로 바꾼) 구동 프로퍼티만 Dst 에 덮어쓴다. 변경 시 true.
	 *  (템플릿 오버레이 / 에디터 "템플릿->월드 미리보기" 에 사용) */
	static bool CopyOverriddenProperties(const USceneComponent* Src, USceneComponent* Dst);

	/** 보간 불가(bool/enum 등) 구동 프로퍼티만 Target -> Live 로 복사 (블렌드 시작 시 1회 스냅). 변경 시 true. */
	static bool ApplyNonLerpable(const USceneComponent* Target, USceneComponent* Live);

	/** 보간 가능한 구동 프로퍼티를 Start/Target 사이 Alpha 보간해 Live 에 직접 기록.
	 *  ExcludeNames 에 든 프로퍼티는 건너뛴다 (호출측이 네이티브 fast-path setter 로 처리 = ApplyHotProps).
	 *  값이 하나라도 바뀌었으면 true -> 호출측이 MarkRenderStateDirty() 를 부른다. */
	static bool LerpProperties(const USceneComponent* Start, const USceneComponent* Target,
	                           USceneComponent* Live, float Alpha, const TSet<FName>* ExcludeNames = nullptr);

	/** 라이트/스카이라이트의 "핫" 프로퍼티(Intensity/LightColor 등)를 프록시 재생성 없는 public setter 로 보간 적용.
	 *  GetHotPropNames() 와 반드시 짝을 맞춰야 한다 (여기서 처리하는 이름 = 거기서 제외하는 이름). */
	static void ApplyHotProps(const USceneComponent* Start, const USceneComponent* Target,
	                          USceneComponent* Live, float Alpha);

	/** ApplyHotProps 가 처리하는 프로퍼티 이름 집합. LerpProperties 의 ExcludeNames 로 넘긴다. */
	static const TSet<FName>& GetHotPropNames(const USceneComponent* Comp);

	/** Live 와 같은 클래스의 비등록 스냅샷 생성 (Outer=TransientPackage 라 GetWorld()==null →
	 *  SkyLight 캡처 큐가 월드 불일치로 스킵한다). 값은 Live 에서 복사된다. */
	static USceneComponent* CreateSnapshot(const USceneComponent* Live);
};
