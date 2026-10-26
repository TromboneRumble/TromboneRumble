# KH_MidtermExam 중간과제 점검표

점검 일시: 2026-10-26 23:16, 한국 시간

대상: `/Game/Levels/TestingMaps/KH_MidtermExam.KH_MidtermExam`

근거: 사용자가 첨부한 `[중간고사과제 안내] 10_31까지 제출.pdf` 전체 4쪽과 사용자가 전달한 교수님 이메일 답변.

이번 점검에서는 레벨과 에셋을 수정하지 않았다. 현재 열린 맵의 액터 180개, Static/Skeletal Mesh 컴포넌트의 머터리얼, 필수 출력 입력, Niagara Sprite Renderer, 레벨에 연결된 Sequencer와 바인딩을 읽어서 확인했다.

## 판정 기준

- Opaque, Masked, Translucent는 서로 구분되는 Static Mesh 또는 Skeletal Mesh 레벨 오브젝트에 직접 적용해야 한다.
- Niagara Mesh Renderer나 Sprite Renderer의 머터리얼은 위 세 종류의 레벨 직접 적용 조건으로 계산하지 않는다.
- Niagara Sprite 이펙트는 PDF에 적힌 별도의 필수 항목으로 점검한다.
- 아래의 '레벨 구현 완료'는 현재 노드 연결과 배치·트랙을 확인했다는 뜻이다. 최종 영상에서 보여야 하는 조건과 제출물 완성까지 대신 인정하는 뜻은 아니다.

## 1. 레벨에서 구현·적용이 확인된 작업

| 항목 | 현재 근거 | 판정 |
|---|---|---|
| Opaque 1종 | `SM_Oak_ME` StaticMeshActor에 `M_Opaque_Oak_ME` 직접 적용. Blend Mode Opaque이며 Base Color, Roughness, Metallic, Normal 모두 텍스처 노드 연결 | 레벨 구현·적용 완료 |
| Translucent 1종 | `Midterm_BeerSurface` StaticMeshActor의 Plane에 `MI_Translucent_Midterm_Beer` 직접 적용. 부모 `M_Translucent_Midterm_Beer`의 Blend Mode Translucent, Opacity에 Clamp 노드 연결 | 레벨 구현·적용 완료 |
| Distortion | `Midterm_BeerRefraction` StaticMeshActor에 `MI_Translucent_Midterm_BeerDistortion` 적용. 부모 머터리얼의 Refraction 및 Normal 연결, Pixel Normal Offset 사용, Refraction_Strength 1.5. 홍수 표면 액터에 부착 | 레벨 구성 완료. 최종 영상에서 굴절 가시성 확인 필요 |
| Niagara Sprite | `BP_BeerStream_02_1`, `_2`, `_3`, `_4`에 `NS_AnimeWater_02` 배치. `BeerDroplets_Sprite` 이미터의 NiagaraSpriteRendererProperties 및 `M_Translucent_BeerDroplet_02` 연결 확인. 이미터·렌더러 활성 설정 확인 | 제작·배치·타이밍 구성 완료. 최종 영상에서 흰 스프라이트 가시성 확인 필요 |
| Sequencer 길이 | `LS_KH_MidtermExam`, 30fps, 0–900프레임, 30초 | 시퀀스 길이 조건 충족. MP4 출력은 별도 |
| 카메라 컷 | Camera Cut 섹션 3개. 0–6초 `Midterm_Cam_01_Opaque`, 6–18초 `Midterm_Cam_02_Streams`, 18–30초 `Midterm_Cam_03_Flood` | 컷 2개 이상 구성 조건 충족 |
| 시네마틱 트랙 | 카메라·오크통·홍수 Transform 트랙, Niagara Life Cycle 4개, 머터리얼 파라미터 트랙 | 구성 완료. 시퀀스의 액터 참조 17개와 Niagara 컴포넌트 참조 4개가 실제 레벨 오브젝트로 연결됨 |
| 머터리얼 이름 | 제출 후보 이름에 `Opaque`, `Masked`, `Translucent` 포함 | 이름 규칙 적용 확인 |

### 확인한 핵심 에셋 경로

- Opaque 메시: `/Game/Arts/Environment/JazzBar/Oak/MidtermExam/SM_Oak_ME`
- Opaque 머터리얼: `/Game/Arts/Environment/JazzBar/Oak/MidtermExam/M_Opaque_Oak_ME`
- Translucent 부모: `/Game/Levels/TestingMaps/KH_MidtermExam_Assets/Materials/M_Translucent_Midterm_Beer`
- Translucent 인스턴스: `/Game/Levels/TestingMaps/KH_MidtermExam_Assets/Materials/MI_Translucent_Midterm_Beer`
- 굴절 인스턴스: `/Game/Levels/TestingMaps/KH_MidtermExam_Assets/Materials/MI_Translucent_Midterm_BeerDistortion`
- 시퀀스: `/Game/Levels/TestingMaps/KH_MidtermExam_Assets/Sequencer/LS_KH_MidtermExam`

### 현재 설정된 효과 시간

- 물줄기 및 흰 스프라이트: 6–20초.
- 오크통 움직임: 약 2–8초.
- 홍수 상승: 8–14초, 최고 수위 유지: 14–22초, 하강: 22–28초.
- 수위 키: 0초 -3cm, 8초 -3cm, 14초 120cm, 22초 120cm, 28초 -3cm, 30초 -3cm.

이 시간은 현재 시퀀스의 설정값이며 최종 설명 문서의 시간대는 최종 렌더링 영상에 맞춰 확정해야 한다.

## 2. 아직 완료되지 않은 레벨 작업

### Masked를 일반 메시 오브젝트에 직접 적용하고 촬영에 포함하기

현재 맵의 StaticMeshComponent 및 SkeletalMeshComponent 전체 검사 결과, Masked 머터리얼이 직접 적용된 슬롯은 **0개**다.

다음 두 Masked 머터리얼은 이미 존재하며 Opacity Mask 연결도 확인했다.

- `/Game/Levels/TestingMaps/KH_MidtermExam_Assets/Materials/M_Masked_Midterm_Erosion01`
- `/Game/Levels/TestingMaps/KH_MidtermExam_Assets/Materials/M_Masked_Midterm_Erosion02`

현재 적용 위치는 Niagara의 메시 렌더러이므로 교수님 이메일의 직접 적용 조건을 충족하지 않는다. 따라서 **머터리얼 자체는 있지만 과제용 레벨 적용은 미완료**다.

해야 할 일:

1. Opaque 오크통 및 Translucent 홍수 표면과 구분되는 일반 Static/Skeletal Mesh 레벨 오브젝트에 Masked를 직접 적용한다.
2. 기존 자작 Masked를 활용할 경우 일반 메시에서도 마스크가 작동하도록 입력값을 확인한다. 현재 그래프에는 ParticleColor와 DynamicParameter가 있다.
3. Opacity Mask로 잘리는 부분이 카메라에서 분명히 보이게 배치하고 촬영 시간을 확보한다.

새 머터리얼이나 새 메시를 반드시 처음부터 제작해야 한다는 조건은 없다. 기존 자작 머터리얼과 기존 일반 메시를 활용할 수 있다. 여기서 요구하는 적용 대상은 Niagara 렌더러가 아니라 레벨의 일반 메시 오브젝트다.

## 3. 제출 전 제작·확인이 필요한 항목

아래 항목은 레벨 안에 에셋이 있다는 사실만으로 완료 판정할 수 없다. 이번 검사에서 KH_MidtermExam의 제출 완성본으로 확인한 파일은 없으며, 다른 폴더에 사용자가 따로 만든 파일의 존재 여부까지 부정하는 뜻은 아니다.

- [ ] **최종 영상 가시성 확인:** Opaque, Masked, Translucent가 적용된 각 일반 메시 오브젝트가 영상에서 구분되어 보여야 한다. Niagara Sprite와 Distortion도 영상에서 확인되어야 한다.
- [ ] **Sequencer 렌더링 MP4:** 15초 이상 60초 이하. 현재 30초 시퀀스는 범위에 맞지만 최종 MP4를 별도로 출력·확인해야 한다.
- [ ] **노드 스크린샷:** Opaque, Masked, Translucent 각 1장, 총 3장. 노드 그래프와 Details의 Blend Mode가 함께 보여야 한다. 노드가 많으면 종류당 최대 2장, 총 최대 6장. 일반 장면 캡처나 메시 미리보기 이미지는 이 제출물을 대신하지 않는다.
- [ ] **설명 PDF:** 컨셉 이미지 또는 설명, 세 머터리얼 각각의 적용 액터 이름과 영상 시간대, Niagara 이펙트의 액터 이름과 영상 시간대.
- [ ] **외부 에셋 출처:** 사용한 외부 에셋의 출처를 설명 PDF에 기재한다.
- [ ] **제출 파일명·메일 정리 및 발송:** 아래 PDF의 지정 형식을 따른다.

### 제출 형식

- 기한: 10월 31일 23:59.
- 수신: `subj21@kakao.com`
- 메일 제목: `[중간과제] 학번_이름`
- 영상: `학번_이름_영상.mp4`
- 노드: `학번_이름_노드_Opaque.png`, `학번_이름_노드_Masked.png`, `학번_이름_노드_Translucent.png`
- 노드 추가 장이 있는 경우 파일명 끝에 `_2` 추가.
- 설명: `학번_이름_설명.pdf`
- 영상 1개, 노드 이미지 3–6개, 설명 PDF 1개를 한 통의 메일로 제출. 영상 용량이 커서 첨부가 안 되면 PDF에서 허용한 공개 열람 권한의 Google Drive 링크 방식 사용 가능.

## 4. 필수 추가 제작 대상으로 잡지 않는 항목

- 자체 제작 Landscape: 필수 아님.
- 별도의 임포트 시연 결과물: 필수 아님.
- Fog: Distortion 또는 Fog 중 하나이므로 현재 굴절 연출이 최종 영상에서 확인되면 별도 Fog 추가는 필수 아님.
- 머터리얼 3종을 Niagara 안에서 각각 보여 주는 작업: 교수님 이메일에 따라 일반 메시 직접 적용 조건을 대신하지 못함.
- 플레이어·게임 진행·UI 제거와 오크통 흔들림 자체: 촬영 구성상 작업이며 PDF의 별도 필수 제출 항목은 아님.

## 작업 우선순위

1. Masked 일반 메시 직접 적용과 화면 연출 완성.
2. 최종 카메라 구도에서 세 머터리얼, Sprite, Distortion이 모두 보이는지 확인.
3. MP4 렌더링 및 재생 검사.
4. 머터리얼 노드·Blend Mode 캡처.
5. 최종 영상의 실제 시간대로 설명 PDF와 출처 작성.
6. 파일명과 제출 형식 점검 후 제출.
