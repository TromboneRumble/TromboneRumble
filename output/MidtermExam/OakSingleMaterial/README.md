# SM_Move_Oak 단일 머티리얼용 Base Color

- 원본: `/Game/Arts/Environment/JazzBar/Oak/SM_Move_Oak.SM_Move_Oak`
- 실제 원본 메시 검사: LOD 1개, 삼각형 2,200개, UV 채널 1개, 머티리얼 그룹 3개.
- 기존 UV는 서로 다른 재질 그룹 사이에 겹친다. 원본 UV를 그대로 사용하면 한 장의 Base Color에서 나무와 금속을 구분할 수 없다.
- 원본 Unreal 메시, 머티리얼, 맵은 수정하지 않았다.

## 파일

- `T_Oak_SingleMaterial_BaseColor.png`: ImageGen 내장 도구로 제작한 1254 x 1254 RGB 이미지. 왼쪽 절반은 갈색 나무, 오른쪽 위는 회색 금속, 오른쪽 아래는 밝은 나무.
- `SM_Move_Oak_SingleMaterial.obj`: 원본의 2,200개 삼각형 위치를 유지하고 UV0를 재질별로 분리한 별도 메시. 머티리얼 참조는 1개. 원본의 블루프린트 기능, 충돌 설정, 피벗 외 메타데이터 등을 교체하는 Unreal 에셋이 아니라 UV 작업용 OBJ이며 아직 임포트하지 않았다. OBJ에는 면별 평면 셰이딩을 지정했다.
- `SM_Move_Oak_SingleMaterial.mtl`: Base Color를 참조하는 OBJ 머티리얼.
- `SM_Move_Oak_SingleMaterial_UV0.png`: 새 UV 배치 확인용 2048 x 2048 가이드. 최종 Base Color로 사용하지 않는다.
- `SM_Move_Oak_BaseColor_Preview.png`: 실제 메시와 새 UV, 생성된 Base Color를 조합한 외부 소프트웨어 렌더 미리보기. Unreal 적용 화면이 아니다.
- `UV_layout.json`: 원본 슬롯별 새 UV 영역. 좌표는 정규화에 사용한 2048 기준이며 이미지 해상도와 독립적이다. 각 영역 안쪽에 24/2048 UV 여백을 확보했다.
- `generation_prompt.txt`: 사용한 전체 이미지 생성 프롬프트.

## Unreal에서 사용할 때

원본 UV에는 바로 적용하지 말고, 새 UV 배치를 과제용 메시 복제본에 반영한 뒤 머티리얼 슬롯/섹션을 하나로 합쳐야 한다. 최종 머티리얼의 Base Color에는 이 이미지의 RGB를 연결한다. 금속 영역을 물리적으로 금속처럼 표현하려면 이후 Metallic/Roughness 영역 설정이 필요하다. 이 작업은 Base Color 이미지 제작까지만 수행했으며, 과제의 Opaque 4개 필수 입력 완성을 의미하지 않는다.

이미지는 RGB이며 색 텍스처 용도로 sRGB를 사용한다. 필요 시 Unreal의 Power of Two 설정으로 패딩/밉맵 정책을 정한다. 원본 생성 이미지는 리샘플링하지 않았다.
