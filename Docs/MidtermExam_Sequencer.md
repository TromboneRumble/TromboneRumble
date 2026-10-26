# KH_MidtermExam 촬영용 Sequencer

대상 맵: `/Game/Levels/TestingMaps/KH_MidtermExam.KH_MidtermExam`

시퀀스: `/Game/Levels/TestingMaps/KH_MidtermExam_Assets/Sequencer/LS_KH_MidtermExam`

30fps, 0–990프레임, 33초. 맵의 `Midterm_Sequence` 액터가 Play에서 시퀀스를 자동 재생하고 마지막 프레임에서 멈춘다. `Playback Settings > Auto Play`를 끄면 Play의 자동 시네마틱 재생도 중지할 수 있다.

## 기본 타이밍

| 연출 | 시간 | 프레임 | 편집 위치 |
|---|---|---|---|
| 오크통 붉은끼 | 5–8초 증가, 8–13초 유지, 13–16초 색 복귀 | 150, 240, 390, 480 | 각 오크통의 mesh color > Redness, 최고값 0.55 |
| 오크통 9개 미세 움직임 | 8–11초 및 팽창 후 13–23초 | 240–330, 390–690 | 각 오크통의 `IdleWobbleStrength` 트랙: 해당 구간만 0.007, 나머지는 0 |
| 오크통 9개 원본 팽창 | 11–13초 팽창, 13–23초 크기 유지, 23–23.8초 복귀 | 이벤트 330, 690 | `Midterm_Oak_01`–`09`의 Event 트랙: `StartGrowWobble`, `EndGrowWobble` |
| 물줄기·흰 스프라이트 | 13–23초 | 390–690 | 각 `BP_BeerStream_02_* > Niagara`의 Life Cycle 섹션 |
| 홍수 상승 | 13–17초 (물줄기 시작과 동일) | 390–510 | `Midterm_BeerSurface > Transform > Location.Z` |
| 최고 수위 유지 | 17–23초 | 510–690 | 같은 Z 키 |
| 홍수 하강 | 23–31초 (물줄기 종료와 동일) | 690–930 | 같은 Z 키 |

수위는 월드 Z -3cm에서 76cm까지 이동한다. 최고 수위를 바꾸려면 `Midterm_BeerSurface > Transform > Location.Z`의 510프레임(17초)과 690프레임(23초) 키 값을 모두 같은 높이로 수정한다. 액터 Details의 Z 또는 채널 기본값만 바꾸면 재생 시 키 값이 우선한다. `Midterm_BeerRefraction`은 표면의 자식으로 부착되어 Z -0.25cm 간격으로 함께 움직인다.

물줄기 시작·종료 시각은 Life Cycle 섹션의 양 끝을 드래그해 조절한다. 홍수는 Transform 키, 오크통은 Event 키를 옮겨 조절한다. 홍수 연출을 바꾸면 `Beer flood material state / progress` 트랙과 마커도 함께 맞춘다.

오크통 `Midterm_Oak_01`–`09`의 위치는 레벨에서 조절한다. 2026-10-27에 합성 Transform 트랙을 제거하고 사용자 원본 `BP_Move_Oak`의 타임라인 동작으로 복구했다. 맵 전용 복사본 `BP_Midterm_OriginalOak`은 현재 `SM_Oak_ME`와 단일 머터리얼을 유지하도록 메시와 MID 생성 슬롯을 맞췄다. 원본 애니메이션 곡선과 원본 BP 에셋은 변경하지 않았다. 이후 사용자 요청으로 BeginPlay의 idle 강도 계산 계수 0.007을 0으로 변경했다. 처음 5초는 정지하고, 물줄기 동안 팽창한 크기를 유지하며, 복귀 후에도 미세 크기 변화가 없다. 원본 idle 타임라인 자체는 실행되며, Sequencer가 IdleWobbleStrength를 8–11초 및 13–23초에 0.007로 설정한다. 팽창 전에는 원래 크기, 팽창 후에는 커진 크기를 기준으로 미세하게 움직이고, 복귀 후에는 강도 0으로 정지한다. 이벤트는 Play에서 실행되므로 에디터 시간 스크럽만으로 원본 BP 타임라인이 재현되지는 않는다.

## 카메라

| 카메라 | 시간 | 용도 |
|---|---|---|
| `Midterm_Cam_01_Opaque` | 0–9초 | `SM_Oak_ME`의 Opaque 머터리얼 소개 |
| `Midterm_Cam_02_Streams` | 9–21초 | 물줄기 4개와 흰 스프라이트. Niagara의 Masked는 머터리얼 3종 적용 조건으로 계산하지 않음 |
| `Midterm_Cam_03_Flood` | 21–33초 | 홍수 표면과 굴절, 배수 |

카메라는 초안이다. Sequencer의 Camera Cuts 섹션으로 컷 길이를 조절하고 각 카메라 Transform으로 구도를 조절한다. 카메라 Transform 기본값이 트랙에 들어 있으므로 카메라를 이동한 뒤 해당 트랙도 갱신해야 한다.

## 이 맵에서 바뀐 게임 구성

- 월드 GameMode를 네이티브 `GameModeBase` 기반 `BP_Midterm_CinematicGameMode`로 변경했다. Default Pawn, Spectator, HUD 클래스는 비웠다.
- 배치된 게임 진행 액터 `BP_GimmickManager`, `BP_RhythmActor`, `BP_GarbageSpawner`를 이 맵에서 제거했다.
- `Midterm_Oak_01`–`09`는 사용자 원본 `BP_Move_Oak`의 맵 전용 복사본 `BP_Midterm_OriginalOak`이다. 현재 메시·머터리얼·배치를 유지하며 Sequencer 이벤트가 원래 팽창·복귀 함수를 호출한다.
- `BP_BeerFloodGimmick`의 BeerPlane은 `Midterm_BeerSurface` StaticMeshActor로 교체했다. 게임의 홍수 타이머 대신 Sequencer가 수위를 제어한다.
- `Midterm_BeerBuoyancy` (`ASequencedBeerBuoyancy`)가 `Midterm_BeerSurface`의 실제 월드 Z를 기존 `FloatableSubsystem`에 매 프레임 전달한다. 의자·맥주잔의 기존 `FloatableComponent`와 물리 설정을 그대로 사용한다. 수위가 `DrySurfaceZ`(-3cm)로 내려오면 `EndFlood`를 호출해 부력과 물속 저항 처리를 종료한다. 수위를 수정해도 별도 의자 애니메이션 키를 수정할 필요가 없다. 연결 액터는 이 맵에만 배치했다.
- `BP_BeerStream_02` 4개와 `NS_AnimeWater_02`의 외형·입자 설정은 유지하고, 맵의 컴포넌트에 Sequencer Life Cycle 트랙을 연결했다.
- 원본 `BP_Move_Oak`, `BP_BeerFloodGimmick`, `BP_BeerStream`, `BP_BeerStream_02`, `NS_AnimeWater_02` 에셋 파일은 이번 구성 작업으로 변경하지 않았다.

이 작업 직전 맵 백업:
`Saved/Backups/CodexMidtermSequencer/20261026_184525/KH_MidtermExam.umap`

## 검증 및 남은 제출 작업

실제 Play 월드에서 Pawn 0개, 최상위 UI 0개, 게임 진행 액터 0개를 확인했다. 0초, 3.17초, 5.97초, 6초, 11초, 19초, 20.03초, 25초, 29.97초에서 오크통 움직임, 물줄기 켜짐·꺼짐, 수위 및 굴절 표면 동기화와 카메라 전환을 확인했다.

이 구성은 영상 제작을 위한 초안이며 최종 MP4 렌더링, 머터리얼 노드 스크린샷, 제출 설명 PDF 작성은 포함하지 않는다. 과제의 실제 영상 가시성은 최종 렌더링 후 확인해야 한다.

## 교수님 답변으로 확정된 머터리얼 적용 조건

사용자가 전달한 교수님 답변에 따라 Opaque·Masked·Translucent는 서로 구분되는 Static Mesh 또는 Skeletal Mesh 레벨 오브젝트에 각각 직접 적용해야 한다. Niagara Mesh Renderer의 머터리얼은 이 적용 조건으로 인정되지 않는다. Niagara 스프라이트 이펙트는 별도의 필수 항목으로 유지한다.

현재 KH_MidtermExam의 일반 메시 컴포넌트를 다시 확인한 결과:

| 항목 | 직접 적용된 오브젝트 | 상태 |
|---|---|---|
| Opaque | `SM_Oak_ME` / `M_Opaque_Oak_ME` | 적용됨. 최종 영상 가시성·노드 캡처 확인 필요 |
| Translucent | `Midterm_BeerSurface` / `MI_Translucent_Midterm_Beer` | 적용됨. 최종 영상 가시성·노드 캡처 확인 필요 |
| Masked | 없음 | 미완료. 별도 Static/Skeletal Mesh 오브젝트에 직접 적용해야 함 |

다음 작업은 Masked용 일반 메시 오브젝트를 배치하고, 머터리얼의 Opacity Mask가 실제로 일부 표면을 잘라 내는 모습이 최종 카메라에 보이도록 구성하는 것이다. 기존 자작 Masked를 재사용할 수 있으나 Niagara 전용 입력에 의존하는 부분은 일반 메시에서도 작동하도록 확인해야 한다. 현재 Niagara 연출은 유지할 수 있다.





