# Translucent 머터리얼 캡처와 설명

대상: `/Game/Levels/TestingMaps/KH_MidtermExam_Assets/Materials/M_Translucent_Midterm_Beer`

인스턴스인 `MI_Translucent_Midterm_Beer`가 아니라 위 부모 머터리얼의 그래프를 캡처한다. 적용 오브젝트는 `KH_MidtermExam`의 `Midterm_BeerSurface`이다. Blend Mode는 **Translucent**이며, 필수 입력은 **Opacity**이다.

## 정리된 Comment 박스

1. **깊이 차이 · 접촉 경계 · 시간**: SceneDepth와 PixelDepth의 차이, 접촉 경계 범위 및 시간 입력.
2. **움직이는 경계 · 거품 마스크**: 스크롤 패턴과 Sine 줄무늬를 조합하고 Clamp·Saturate로 마스크를 정리.
3. **깊이에 따른 맥주 색상**: DepthFade에 따른 두 색상의 보간과 거품 합성.
4. **최종 거품 신호 · 표면 반사 · 발광**: 공통 거품 신호와 Metallic·Specular·Roughness·Emissive 출력.
5. **투명도 조절 [필수: Opacity]**: DepthFade → Lerp → Opacity Min/Max Clamp → Opacity 연결.
6. **흐르는 패턴 · 수면 노멀 · 굴절**: UV Panner·VertexInterpolator와 노멀·Fresnel 굴절.

거품 계산의 마지막 부분에는 **02-1 거품 줄무늬 · 최종 마스크** 소영역을 추가했다. 주영역 6개와 소영역 1개, 총 Comment 7개다.

## 최대 2장으로 캡처하기

전체 91개 노드를 한 장에 맞추려고 과도하게 축소하지 않는다. 필수 Opacity 연결과 역할이 드러나는 핵심 영역을 우선한다.

### 1장: 필수 Opacity와 최종 출력

- **05 투명도 조절**과 머터리얼 출력 노드의 **Opacity 핀**이 함께 보이게 잡는다.
- `FadeDistance → DepthFade`, 거품 신호를 받는 `Lerp`, `Opacity_Min / Opacity_Max → Clamp`와 마지막 출력선이 읽혀야 한다.
- 위쪽 **04 최종 거품 신호 · 표면 반사 · 발광**의 일부도 포함한다.
- 그래프 빈 곳을 선택하여 Details에 머터리얼 설정을 표시하고, **Blend Mode = Translucent**가 보이게 한다.
- 파일명: `학번_이름_노드_Translucent.png`

캡처 아래 설명:

> DepthFade로 오브젝트 접촉부의 투명도를 완화하고, 거품 마스크를 Lerp에 사용하여 수면과 거품의 투명도를 구분했습니다.  
> Opacity_Min·Opacity_Max로 범위를 제한한 Clamp 출력을 Opacity에 연결했으며, Blend Mode는 Translucent입니다.

### 2장: 거품 마스크와 맥주 색상 합성

- **02 움직이는 경계 · 거품 마스크**의 마지막 `Clamp → Multiply → Saturate`와 **03 깊이에 따른 맥주 색상**의 핵심 연결을 중심으로 잡는다.
- **04 최종 거품 신호**의 `Add`·연결점과 출력 노드를 1장과 일부 겹쳐서, 두 캡처 사이의 연결 관계가 이어져 보이게 한다.
- 핵심은 두 마스크를 합친 신호가 색상 Lerp·발광·투명도에 사용되는 흐름이다. UV와 노멀의 전체 계산을 억지로 같은 화면에 모두 넣지 않는다.
- 가능하면 Details의 **Blend Mode = Translucent**도 유지한다.
- 파일명: `학번_이름_노드_Translucent_2.png`

캡처 아래 설명:

> Clamp·Saturate로 정리한 경계와 줄무늬 거품 신호를 합쳐 색상 합성과 발광에 사용했습니다.  
> Color·Color_Coast를 DepthFade로 보간하고, 거품 마스크를 이용해 최종 Base Color를 구성했습니다.

두 설명은 제출 설명 PDF에서 해당 캡처 아래에 넣을 수 있다. 설명 문서에는 적용 액터 이름 `Midterm_BeerSurface`와 실제 최종 영상에서 수면이 보이는 시간대도 적는다. 최종 편집으로 영상 시간이 바뀌면 그 결과를 기준으로 기록한다.

## 보존·검증 결과

- 원래의 셰이더 노드 91개를 모두 유지했다. 출력과 연결되지 않은 미사용 노드는 없었다.
- 노드의 계산 속성·파라미터 값·입력 연결과 머터리얼 출력 연결이 정리 전과 동일했다.
- 정리 전후 컴파일된 셰이더 통계가 같았다.
- Comment는 노드의 짧은 설명문만 추가한 것이 아니라 실제 `EditorComments`로 저장했다. 주영역 6개에 거품 소영역 1개를 더한 총 7개다. 별도 에디터 미리보기에 위치와 Comment가 반영되는 것도 검증했다.
- 다른 제출 머터리얼, 맵, 시퀀스, 수면 인스턴스 및 별도 굴절 머터리얼의 파일 해시는 그대로였다.

## 준비한 실제 캡처 파일

- `Output/MidtermExam/Translucent/노드_Translucent.png`: Opacity 연결, 관련 파라미터, 최종 출력과 Blend Mode.
- `Output/MidtermExam/Translucent/노드_Translucent_2.png`: 거품의 Sine·Clamp·Saturate, 맥주 색상의 DepthFade·Lerp와 공통 거품 신호. 첫 장과 공통 출력 영역이 겹친다.

두 PNG는 정리한 실제 Unreal 머터리얼 편집기 창을 촬영한 것이다. 필수 Opacity 연결, `Opacity_Min / Opacity_Max`, Blend Mode와 Comment 제목의 가독성을 확인했다. 위에서 제시한 캡처별 2줄 설명은 최종 설명 PDF에 해당 이미지 아래로 옮기면 된다. 제출 전 두 PNG의 파일명 앞에 본인의 `학번_이름_`을 붙인다. 이미지 크기는 임의로 줄이지 않고 원본으로 첨부한다.
