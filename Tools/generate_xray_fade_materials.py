# X-Ray 페이드 - 사본 머티리얼 + 매핑 DataAsset 생성 스크립트
#
# 원본 머티리얼은 건드리지 않는다. MF_OcclusionFade를 심은 "사본"을 따로 만들고,
# 원본->사본 매핑을 DataAsset에 기록한다. 런타임에는 UXRayTranslucentFadeComponent가
# 가려지는 동안에만 메시 슬롯을 사본으로 갈아끼운다.
#
# 이 스크립트는 레벨을 읽기만 한다. 배치된 액터도 .umap도 수정하지 않는다.
#
# 사본은 Translucent + MF_OcclusionFade를 Opacity에 연결한다. FadedOpacity(0.35)~1.0
# 범위의 연속 알파를 그대로 블렌딩한다.
#
# 주의: Nanite는 Opaque/Masked만 지원한다. Translucent 사본이 Nanite 메시에 물리면
# 그 섹션은 회색 기본 머티리얼로 렌더된다(NaniteResources.cpp의 IsSupportedBlendMode).
# 이 스크립트는 수집 단계에서 Nanite가 켜진 XRayBlocker 메시를 경고로만 알려준다 -
# 해당 컴포넌트의 Disallow Nanite를 켜는 건 수동으로 해야 한다.
#
# [대상 레벨]
#   Trombone Map Settings(UGameMapDeveloperSettings)에서 Trombone.Maps.InGame.* 태그에
#   바인딩된 레벨을 쓴다. 스크립트에 레벨 이름을 적어두지 않으므로, 인게임 맵을 추가할 때는
#   이 파일이 아니라 프로젝트 설정의 태그 바인딩을 고치면 된다.
#
# [출력 - 레벨마다 따로]
#   사본:   /Game/Arts/Environment/XRayFade/<레벨명>/
#   매핑:   /Game/Data/DA_XRayFadeMaterialMap_<레벨명>
#   레벨 하나를 다시 만들어도 다른 레벨 데이터는 건드리지 않는다.
#
# 사전 조건:
#   1. Edit > Plugins > "Python Editor Script Plugin" 활성화 후 에디터 재시작
#   2. C++ 클래스 UXRayFadeMaterialMap 이 컴파일돼 있을 것
#   3. Content/Arts/Environment/XRayFade/MF_OcclusionFade 머티리얼 함수가 있을 것
#      (그래프: ScalarParameter "OcclusionFade"(기본 0), ScalarParameter "FadedOpacity"(기본 0.35)
#       -> Lerp(A=1.0, B=FadedOpacity, Alpha=OcclusionFade)
#       -> FunctionOutput)
#
# 실행: Tools > Execute Python Script > 이 파일 선택  (InGame 태그 전체)
#       UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<이 파일>"
#       한 레벨만: -script="<이 파일> Trombone.Maps.InGame.SnowField"
#                  -script="<이 파일> /Game/Levels/InGame_SnowField"
#       고아 정리: -script="<이 파일> --prune"   (바인딩이 빠진 레벨의 사본/DA 삭제)
#
# 주의: 대상 레벨의 출력 폴더를 비우고 다시 만든다(멱등성). 실행 전 해당 머티리얼 탭들을 닫을 것.
#       헤드리스 실행 시 Display 로그는 stdout에 안 찍히므로 log_warning을 쓴다.

import sys

import unreal

MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary

MF_PATH = '/Game/Arts/Environment/XRayFade/MF_OcclusionFade'
LEGACY_MF_PATH = '/Game/Arts/Environment/MF_OcclusionFade'   # 옮기기 전 경로
OUTPUT_ROOT = '/Game/Arts/Environment/XRayFade'
MAP_ASSET_DIR = '/Game/Data'
MAP_ASSET_STEM = 'DA_XRayFadeMaterialMap'
MAP_ASSET_PREFIX = MAP_ASSET_STEM + '_'
OCCLUDER_TAG = 'XRayBlocker'   # UXRayComponentBase::OccluderTag 기본값과 일치해야 함
FADE_SUFFIX = '_XRayFade'

# 대상 레벨은 Trombone Map Settings 의 이 태그 밑에서 가져온다.
# TromboneGamePlayTags.h 의 InGamePath 와 같은 문자열.
INGAME_TAG_PREFIX = 'Trombone.Maps.InGame'

# 레벨별로 나누기 전에 쓰던 전역 매핑 에셋. 남아 있으면 정리한다.
LEGACY_MAP_ASSET_PATH = '%s/%s' % (MAP_ASSET_DIR, MAP_ASSET_STEM)


def log(msg):
    # 헤드리스에서도 보이도록 warning 레벨 사용
    unreal.log_warning(msg)


# --- 대상 레벨 수집 ---

def package_name_of(object_path):
    """'/Game/Levels/X.X' -> '/Game/Levels/X'. 이미 패키지명이면 그대로."""
    return str(object_path).strip().strip('"').strip("'").split('.')[0]


def asset_name_of(object_path):
    return package_name_of(object_path).rsplit('/', 1)[-1]


def tag_string(tag):
    """FGameplayTag -> 'Trombone.Maps.InGame.SnowField'.

    TagName 이 protected 라 파이썬 속성으로 안 잡힌다. export_text 는
    '(TagName="Trombone.Maps.InGame.SnowField")' 를 준다.
    """
    text = tag.export_text()
    start = text.find('"')
    end = text.rfind('"')
    return text[start + 1:end] if 0 <= start < end else ''


def get_ingame_bindings():
    """Trombone Map Settings 에서 InGame 태그에 바인딩된 (태그, 레벨패키지) 목록."""
    # UDeveloperSettings 파생은 BlueprintType 이 아니라 unreal.GameMapDeveloperSettings 가
    # 생성되지 않는다. 클래스를 경로로 직접 불러 CDO 를 잡고, 프로퍼티도 C++ 이름으로 읽는다.
    settings_class = unreal.load_class(None, '/Script/TromboneRumble.GameMapDeveloperSettings')
    if not settings_class:
        raise RuntimeError('UGameMapDeveloperSettings 를 찾을 수 없습니다. C++이 컴파일됐는지 확인하세요.')

    settings = unreal.get_default_object(settings_class)
    game_play_map = settings.get_editor_property('GamePlayMap')

    bindings = []
    for tag, soft_path in game_play_map.items():
        tag_str = tag_string(tag)
        # 접두사 자체와 그 하위 전부. 더 깊게 중첩된 태그도 걸린다.
        if tag_str != INGAME_TAG_PREFIX and not tag_str.startswith(INGAME_TAG_PREFIX + '.'):
            continue
        bindings.append((tag_str, package_name_of(soft_path.export_text())))

    bindings.sort()
    return bindings


def resolve_levels(bindings, requested):
    """요청 인자를 레벨 패키지 목록으로. 인자가 없으면 바인딩 전체. 같은 레벨은 1회만."""
    by_tag = dict(bindings)

    if requested:
        wanted = []
        for token in requested:
            if token.startswith('/Game/'):
                wanted.append(package_name_of(token))
            elif token in by_tag:
                wanted.append(by_tag[token])
            else:
                raise RuntimeError(
                    '알 수 없는 인자입니다: %s (레벨 경로는 /Game/ 으로 시작, 태그는 GamePlayMap 에 있어야 함)' % token)
    else:
        wanted = [level for _, level in bindings]

    levels = []
    for level in wanted:
        if level in levels:
            continue
        if not EAL.does_asset_exist(level):
            log('  [경고] 레벨이 없어 건너뜁니다: %s' % level)
            continue
        levels.append(level)

    return levels


# --- 사본 생성 ---

def collect_occluder_materials():
    """열린 레벨의 XRayBlocker 액터들이 쓰는 유니크 머티리얼을 수집한다 (읽기 전용).

    동시에 두 가지를 더 모은다.
    - Nanite가 켜진 채 Disallow Nanite도 안 된 메시 이름 (Translucent 사본이 Nanite
      경로에서 회색 기본 머티리얼로 렌더되므로 경고용)
    - 스켈레탈 메시가 쓰는 머티리얼 경로 (사본에 usage 플래그를 켜야 하므로)
    """
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    all_actors = actor_subsystem.get_all_level_actors()

    materials = {}   # path -> UMaterialInterface (메시가 실제로 물고 있는 것)
    tagged_count = 0
    nanite_meshes = set()
    skinned_paths = set()

    for actor in all_actors:
        if not actor.actor_has_tag(OCCLUDER_TAG):
            continue
        tagged_count += 1

        for comp in actor.get_components_by_class(unreal.MeshComponent):
            # SkeletalMeshComponent 말고 SkinnedMeshComponent로 본다.
            # GPU 스키닝 렌더 경로를 소유하는 베이스라 PoseableMesh까지 덮는다.
            is_skinned = isinstance(comp, unreal.SkinnedMeshComponent)

            for slot_index in range(comp.get_num_materials()):
                mat = comp.get_material(slot_index)
                if mat:
                    materials[mat.get_path_name()] = mat
                    if is_skinned:
                        skinned_paths.add(mat.get_path_name())

            if isinstance(comp, unreal.StaticMeshComponent) and not comp.get_editor_property('disallow_nanite'):
                mesh = comp.get_editor_property('static_mesh')
                if mesh and mesh.get_editor_property('nanite_settings').get_editor_property('enabled'):
                    nanite_meshes.add(mesh.get_name())

    return materials, tagged_count, nanite_meshes, skinned_paths


def variant_path(original, out_dir, used_paths):
    """사본 경로. 이름이 겹치면 원본의 상위 폴더명을 앞에 붙여 구분한다."""
    path = '%s/%s%s' % (out_dir, original.get_name(), FADE_SUFFIX)
    if path not in used_paths:
        return path

    parent_folder = package_name_of(original.get_path_name()).rsplit('/', 2)[-2]
    stem = '%s_%s' % (parent_folder, original.get_name())
    unique = '%s/%s%s' % (out_dir, stem, FADE_SUFFIX)

    index = 2
    while unique in used_paths:
        unique = '%s/%s_%d%s' % (out_dir, stem, index, FADE_SUFFIX)
        index += 1

    log('  [주의] 이름 충돌: %s -> %s' % (original.get_name(), unique))
    return unique


def make_base_variant(original, dst_path, mf, control_instructions):
    """base Material 사본: Translucent + MF_OcclusionFade 주입."""
    src_path = package_name_of(original.get_path_name())

    variant = EAL.duplicate_asset(src_path, dst_path)
    if not variant:
        log('  [실패] 복제 불가: %s' % src_path)
        return None

    variant.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)

    node = MEL.create_material_expression(
        variant, unreal.MaterialExpressionMaterialFunctionCall, -450, 300)
    node.set_material_function(mf)   # 필수: 호출 노드의 Outputs 갱신 포함

    if not MEL.connect_material_property(node, '', unreal.MaterialProperty.MP_OPACITY):
        log('  [실패] Opacity 연결 실패: %s' % dst_path)
        return None

    MEL.recompile_material(variant)

    # 통계는 풀 에디터에서만 채워진다(커맨드릿에서는 원본조차 0). 그래서 손대지 않은
    # 원본으로 먼저 재본 control_instructions가 0보다 클 때만 검증에 쓴다.
    if control_instructions > 0:
        stats = MEL.get_statistics(variant)
        if stats.num_pixel_shader_instructions == 0:
            log('  [실패] 컴파일 결과가 비어 있음: %s' % dst_path)
            return None

    EAL.save_loaded_asset(variant)
    return variant


def make_instance_variant(original, dst_path, parent_variant):
    """MaterialInstance 사본: 부모만 사본으로 갈아끼우고 파라미터 오버라이드는 그대로 둔다."""
    src_path = package_name_of(original.get_path_name())

    variant = EAL.duplicate_asset(src_path, dst_path)
    if not variant:
        log('  [실패] 복제 불가: %s' % src_path)
        return None

    variant.set_editor_property('parent', parent_variant)

    # 인스턴스가 blend mode를 Opaque로 오버라이드해 두면 부모의 Translucent가 무시된다.
    # 사본에서는 확실하게 Translucent로 못박는다.
    overrides = variant.get_editor_property('base_property_overrides')
    overrides.set_editor_property('override_blend_mode', True)
    overrides.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
    variant.set_editor_property('base_property_overrides', overrides)

    EAL.save_loaded_asset(variant)
    return variant


def get_or_create_variant(original, out_dir, mf, control_instructions, cache, used_paths, failed):
    """원본에 대응하는 사본을 만들거나 캐시에서 반환. MaterialInstance는 부모까지 재귀."""
    key = original.get_path_name()
    if key in cache:
        return cache[key]

    if isinstance(original, unreal.MaterialInstance):
        parent = original.get_editor_property('parent')
        if not parent:
            log('  [실패] 부모가 없는 인스턴스: %s' % key)
            failed.append(original.get_name())
            return None

        parent_variant = get_or_create_variant(
            parent, out_dir, mf, control_instructions, cache, used_paths, failed)
        if not parent_variant:
            failed.append(original.get_name())
            return None

        dst_path = variant_path(original, out_dir, used_paths)
        used_paths.add(dst_path)
        variant = make_instance_variant(original, dst_path, parent_variant)

    elif isinstance(original, unreal.Material):
        dst_path = variant_path(original, out_dir, used_paths)
        used_paths.add(dst_path)
        variant = make_base_variant(original, dst_path, mf, control_instructions)

    else:
        log('  [실패] 지원하지 않는 타입 %s: %s' % (type(original).__name__, key))
        failed.append(original.get_name())
        return None

    if not variant:
        failed.append(original.get_name())
        return None

    cache[key] = variant
    return variant


# --- 매핑 DataAsset ---

def load_or_create_map_asset(map_path):
    if EAL.does_asset_exist(map_path):
        map_asset = EAL.load_asset(map_path)
    else:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        map_asset = asset_tools.create_asset(
            map_path.rsplit('/', 1)[1],
            map_path.rsplit('/', 1)[0],
            unreal.XRayFadeMaterialMap,
            unreal.DataAssetFactory())

    if not map_asset:
        raise RuntimeError('매핑 DataAsset을 만들 수 없습니다: %s' % map_path)
    return map_asset


def write_map_asset(map_asset, pairs):
    """그 레벨의 원본->사본 매핑을 통째로 다시 쓴다."""
    map_asset.set_editor_property('fade_variants', pairs)
    EAL.save_loaded_asset(map_asset)


# --- 검증 ---

def base_material_of(material):
    """MaterialInstance 체인을 타고 올라가 최종 base Material. 못 찾으면 None."""
    seen = set()
    current = material
    while isinstance(current, unreal.MaterialInstance):
        key = current.get_path_name()
        if key in seen:
            return None
        seen.add(key)
        current = current.get_editor_property('parent')

    return current if isinstance(current, unreal.Material) else None


# 원본에 켜져 있으면 사본에도 켜져 있어야 하는 usage 플래그들.
# duplicate_asset이 그대로 복사해 주므로, 여기서 격차가 잡히면 사본이 낡았다는 뜻이다.
# 엔진이 이름을 바꾸면 그 항목만 조용히 건너뛴다 (아래 try/except).
USAGE_PROPS = (
    'used_with_skeletal_mesh',
    'used_with_morph_targets',
    'used_with_spline_meshes',
    'used_with_instanced_static_meshes',
    'used_with_particle_sprites',
    'used_with_beam_trails',
    'used_with_mesh_particles',
    'used_with_niagara_sprites',
    'used_with_niagara_ribbons',
    'used_with_niagara_mesh_particles',
    'used_with_static_lighting',
    'used_with_geometry_collections',
    'used_with_clothing',
    'used_with_geometry_cache',
    'used_with_water',
    'used_with_hair_strands',
    'used_with_lidar_point_cloud',
    'used_with_virtual_heightfield_mesh',
    'used_with_nanite',
)

SKELETAL_USAGE_PROP = 'used_with_skeletal_mesh'


def get_usage(material, prop):
    """usage 플래그 하나를 읽는다. 엔진에 없는 이름이면 None."""
    try:
        return bool(material.get_editor_property(prop))
    except Exception:
        return None


def ensure_skinned_usage(variant):
    """스켈레탈이 쓰는 머티리얼의 사본은 usage 플래그를 반드시 켠다.

    안 켜면 FSkeletalMeshSceneProxy가 CheckMaterialUsage_Concurrent 실패로 회색 기본
    머티리얼을 대신 물린다. 에디터는 게임 스레드에서 자동 수리하지만 쿡된 빌드는 못 고친다.
    플래그는 MaterialInstance에 없다 - 반드시 베이스 Material에 켜야 한다.
    """
    base = base_material_of(variant)
    if not base:
        log('  [경고] 베이스 Material 을 못 찾아 usage 플래그를 못 켭니다: %s' % variant.get_name())
        return

    if get_usage(base, SKELETAL_USAGE_PROP) is not False:
        return   # 이미 켜져 있거나(True) 프로퍼티가 없는 엔진(None)

    base.set_editor_property(SKELETAL_USAGE_PROP, True)
    MEL.recompile_material(base)
    EAL.save_loaded_asset(base)
    log('  usage 플래그 설정: %s <- Used with Skeletal Mesh' % base.get_name())


def verify_variants(pairs, skinned_paths):
    """사본이 진짜 Translucent 로 저장됐는지, 원본만큼의 usage 를 갖는지 확인한다.

    컴파일 검증(get_statistics)은 커맨드릿에서 0만 나와 못 쓴다. 프로퍼티 확인은
    어디서 돌려도 동작한다.
    """
    problems = []

    for original, variant in sorted(pairs.items(), key=lambda kv: kv[1].get_name()):
        name = variant.get_name()

        if isinstance(variant, unreal.MaterialInstance):
            overrides = variant.get_editor_property('base_property_overrides')
            if not overrides.get_editor_property('override_blend_mode'):
                problems.append('%s: blend mode 오버라이드가 꺼져 있음' % name)
            elif overrides.get_editor_property('blend_mode') != unreal.BlendMode.BLEND_TRANSLUCENT:
                problems.append('%s: 오버라이드 blend mode 가 %s'
                                % (name, overrides.get_editor_property('blend_mode')))

        base = base_material_of(variant)
        if not base:
            problems.append('%s: 베이스 Material 을 못 찾음' % name)
            continue

        if base.get_editor_property('blend_mode') != unreal.BlendMode.BLEND_TRANSLUCENT:
            problems.append('%s: 베이스 %s 의 blend mode 가 %s'
                            % (name, base.get_name(), base.get_editor_property('blend_mode')))

        # 스켈레탈이 쓰는 머티리얼인데 플래그가 없으면 쿡 빌드에서 회색으로 렌더된다
        if original.get_path_name() in skinned_paths and get_usage(base, SKELETAL_USAGE_PROP) is False:
            problems.append('%s: 스켈레탈 메시가 쓰는데 베이스 %s 에 %s 가 꺼져 있음'
                            % (name, base.get_name(), SKELETAL_USAGE_PROP))

        # 사본이 원본보다 능력이 모자라면 사본이 낡은 것이다
        original_base = base_material_of(original)
        if original_base:
            for prop in USAGE_PROPS:
                if get_usage(original_base, prop) is True and get_usage(base, prop) is False:
                    problems.append('%s: 원본 %s 에는 있는 %s 가 사본에 없음 (사본이 낡음)'
                                    % (name, original_base.get_name(), prop))

    if problems:
        log('  [검증 실패] %d건:' % len(problems))
        for problem in problems:
            log('    %s' % problem)
        return False

    log('  검증 통과: 사본 %d개 전부 Translucent' % len(pairs))
    return True


# --- 정리 ---

def get_referencers(package):
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    options = unreal.AssetRegistryDependencyOptions()   # 기본값이 하드+소프트 패키지 참조
    return [str(name) for name in registry.get_referencers(package, options)]


def outside_referencers(package):
    """XRayFade 폴더와 매핑 DataAsset 을 뺀 참조자. 남아 있으면 지우면 안 된다.

    매핑 DataAsset 은 지우기 직전에 항상 비우므로 걸림돌이 아니다.
    """
    result = []
    for ref in get_referencers(package):
        if ref.startswith(OUTPUT_ROOT + '/'):
            continue
        if asset_name_of(ref).startswith(MAP_ASSET_STEM):
            continue
        result.append(ref)
    return result


def delete_map_and_folder(map_path, out_dir):
    """DA 의 하드 참조를 먼저 끊고 폴더 -> DA 순으로 지운다. 순서를 바꾸면 참조 다이얼로그가 뜬다."""
    if EAL.does_asset_exist(map_path):
        write_map_asset(load_or_create_map_asset(map_path), {})

    if EAL.does_directory_exist(out_dir):
        EAL.delete_directory(out_dir)

    if EAL.does_asset_exist(map_path):
        EAL.delete_asset(map_path)


def cleanup_legacy(mf_package):
    """레벨별로 나누기 전의 전역 DA와 XRayFade 루트 바로 아래 사본들을 정리한다.

    mf_package는 현재 쓰는 MF_OcclusionFade의 실제 경로 - OUTPUT_ROOT 바로 아래로
    옮겨져 있어 "레벨 폴더로 나누기 전의 평면 사본"으로 오인될 수 있으므로 스윕에서 뺀다.
    """
    if EAL.does_asset_exist(LEGACY_MAP_ASSET_PATH):
        blockers = outside_referencers(LEGACY_MAP_ASSET_PATH)
        if blockers:
            log('[레거시] %s 를 아직 참조하는 곳이 있어 남겨둡니다: %s'
                % (LEGACY_MAP_ASSET_PATH, ', '.join(blockers)))
        else:
            write_map_asset(load_or_create_map_asset(LEGACY_MAP_ASSET_PATH), {})
            EAL.delete_asset(LEGACY_MAP_ASSET_PATH)
            log('[레거시] 전역 매핑 DataAsset 삭제: %s' % LEGACY_MAP_ASSET_PATH)

    if not EAL.does_directory_exist(OUTPUT_ROOT):
        return

    # 루트 바로 아래 에셋 = 레벨 폴더로 나누기 전에 만든 평면 사본들 (MF_OcclusionFade 본체는 제외)
    flat = EAL.list_assets(OUTPUT_ROOT, False, False)
    for object_path in flat:
        package = package_name_of(object_path)
        if package == mf_package:
            continue
        blockers = outside_referencers(package)
        if blockers:
            log('[레거시] %s 를 참조하는 곳이 있어 남겨둡니다: %s' % (package, ', '.join(blockers)))
            continue
        EAL.delete_asset(package)
        log('[레거시] 옛 사본 삭제: %s' % package)


def existing_level_folders():
    """XRayFade 아래에 실제로 사본이 들어 있는 레벨 폴더 이름들."""
    if not EAL.does_directory_exist(OUTPUT_ROOT):
        return set()

    names = set()
    for object_path in EAL.list_assets(OUTPUT_ROOT, True, False):
        rest = package_name_of(object_path)[len(OUTPUT_ROOT) + 1:]
        if '/' in rest:
            names.add(rest.split('/')[0])
    return names


def existing_map_assets():
    """DA_XRayFadeMaterialMap_<레벨명> 에서 레벨명만 뽑는다."""
    names = set()
    for object_path in EAL.list_assets(MAP_ASSET_DIR, False, False):
        name = asset_name_of(object_path)
        if name.startswith(MAP_ASSET_PREFIX):
            names.add(name[len(MAP_ASSET_PREFIX):])
    return names


def sweep_orphans(active_names, prune):
    """바인딩이 빠진 레벨의 사본 폴더와 DA. 기본은 보고만 하고 --prune 일 때만 지운다."""
    orphans = sorted((existing_level_folders() | existing_map_assets()) - set(active_names))
    if not orphans:
        return

    for name in orphans:
        map_path = '%s/%s%s' % (MAP_ASSET_DIR, MAP_ASSET_PREFIX, name)
        out_dir = '%s/%s' % (OUTPUT_ROOT, name)

        if not prune:
            log('[고아] %s — InGame 태그에 없는 레벨입니다. 지우려면 --prune 을 붙여 다시 실행하세요.' % name)
            continue

        blockers = outside_referencers(map_path) if EAL.does_asset_exist(map_path) else []
        if blockers:
            log('[고아] %s — 아직 참조하는 곳이 있어 남겨둡니다: %s' % (name, ', '.join(blockers)))
            continue

        delete_map_and_folder(map_path, out_dir)
        log('[고아] 삭제: %s' % name)


# --- 레벨 하나 처리 ---

def process_level(level_package, mf):
    name = asset_name_of(level_package)
    out_dir = '%s/%s' % (OUTPUT_ROOT, name)
    map_path = '%s/%s%s' % (MAP_ASSET_DIR, MAP_ASSET_PREFIX, name)

    log('')
    log('--- %s ---' % name)

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.load_level(level_package):
        log('  [실패] 레벨을 열 수 없습니다: %s' % level_package)
        return 'failed'

    originals, tagged_count, nanite_meshes, skinned_paths = collect_occluder_materials()
    log('  %s 태그 액터 %d개에서 유니크 머티리얼 %d개 수집 (스켈레탈이 쓰는 것 %d개)'
        % (OCCLUDER_TAG, tagged_count, len(originals), len(skinned_paths)))

    if tagged_count == 0:
        # 아직 태그를 안 붙인 레벨일 수 있으니 기존 데이터를 지우지 않고 넘어간다
        log('  [건너뜀] %s 태그가 달린 액터가 없습니다.' % OCCLUDER_TAG)
        return 'skipped'

    if nanite_meshes:
        log('  [경고] Nanite 메시 %d개가 %s 입니다. Translucent 사본은 Nanite 경로에서 '
            '회색 기본 머티리얼로 렌더됩니다(NaniteResources.cpp의 IsSupportedBlendMode). '
            '해당 컴포넌트의 Disallow Nanite 를 켜세요: %s'
            % (len(nanite_meshes), OCCLUDER_TAG, ', '.join(sorted(nanite_meshes))))

    # 손대지 않은 원본으로 통계 가용 여부를 먼저 잰다. 풀 에디터에서는 >0, 커맨드릿에서는 0이 나온다.
    control = next(iter(originals.values()))
    control_instructions = MEL.get_statistics(control).num_pixel_shader_instructions
    if control_instructions == 0:
        log('  참고: 이 환경에서는 머티리얼 통계를 얻을 수 없어 컴파일 검증을 건너뜁니다 '
            '(커맨드릿 실행). 사본을 에디터에서 한 번 열어 확인하세요.')

    # 맵이 사본들을 하드 참조하고 있으므로, 폴더를 지우기 전에 참조부터 끊는다.
    # 안 그러면 "다른 에셋이 참조 중" 확인 다이얼로그가 떠서 헤드리스 실행이 멈춘다.
    map_asset = load_or_create_map_asset(map_path)
    write_map_asset(map_asset, {})

    # 이 레벨의 출력 폴더만 비우고 다시 만든다 (멱등성). 다른 레벨 폴더는 건드리지 않는다.
    if EAL.does_directory_exist(out_dir):
        EAL.delete_directory(out_dir)
    EAL.make_directory(out_dir)

    cache = {}        # 원본 경로 -> 사본 (부모까지 포함. 중복 생성 방지)
    used_paths = set()
    failed = []
    pairs = {}        # 메시가 물고 있는 원본 -> 사본

    for _, original in sorted(originals.items(), key=lambda kv: kv[0]):
        variant = get_or_create_variant(
            original, out_dir, mf, control_instructions, cache, used_paths, failed)
        if variant:
            pairs[original] = variant

    # 사본(variant)을 넘겨 헬퍼가 base_material_of로 거슬러 올라가게 한다.
    # 메시가 인스턴스(M_fabric_W)를 물고 있어도 플래그가 베이스 사본에 정확히 떨어진다.
    for original, variant in sorted(pairs.items(), key=lambda kv: kv[1].get_name()):
        if original.get_path_name() in skinned_paths:
            ensure_skinned_usage(variant)

    write_map_asset(map_asset, pairs)

    log('  매핑 등록 (%d): %s' % (len(pairs), ', '.join(sorted(m.get_name() for m in pairs.keys()))))
    log('  생성된 사본 (%d, 부모 포함) -> %s' % (len(cache), out_dir))
    log('  매핑 기록: %s' % map_path)

    verified = verify_variants(pairs, skinned_paths)

    if failed:
        log('  실패 (%d): %s' % (len(failed), ', '.join(failed)))
        return 'failed'

    return 'done' if verified else 'failed'


# --- 실행 ---

def main():
    args = [a for a in sys.argv[1:] if a]
    prune = '--prune' in args
    requested = [a for a in args if not a.startswith('--')]

    mf = unreal.load_asset(MF_PATH)
    mf_package = MF_PATH
    if not mf:
        mf = unreal.load_asset(LEGACY_MF_PATH)
        mf_package = LEGACY_MF_PATH
    if not mf:
        raise RuntimeError('MF_OcclusionFade 를 찾을 수 없습니다: %s 또는 %s' % (MF_PATH, LEGACY_MF_PATH))

    log('=== X-Ray 페이드 사본 생성 시작 ===')

    bindings = get_ingame_bindings()
    if not bindings:
        raise RuntimeError(
            '%s.* 태그에 바인딩된 레벨이 없습니다. '
            'Project Settings > Trombone Map Settings 를 확인하세요.' % INGAME_TAG_PREFIX)

    log('InGame 바인딩 %d개:' % len(bindings))
    for tag_str, level in bindings:
        log('  %s -> %s' % (tag_str, level))

    levels = resolve_levels(bindings, requested)
    if not levels:
        raise RuntimeError('처리할 레벨이 없습니다.')

    log('처리 대상 %d개: %s' % (len(levels), ', '.join(asset_name_of(l) for l in levels)))

    cleanup_legacy(mf_package)

    results = {}
    for level in levels:
        results[asset_name_of(level)] = process_level(level, mf)

    # 인자로 레벨을 지정한 실행은 부분 실행이라 고아 판정을 하지 않는다.
    # 살아있는 이름은 "처리한 레벨"이 아니라 "바인딩된 레벨" 전체다 —
    # 태그가 없어 건너뛴 레벨을 고아로 오해하면 안 된다.
    if not requested:
        sweep_orphans([asset_name_of(level) for _, level in bindings], prune)
    elif prune:
        log('[고아] 레벨을 지정한 실행에서는 고아 정리를 하지 않습니다. 인자 없이 --prune 만 주세요.')

    log('')
    log('=== 완료 ===')
    for name in sorted(results):
        log('  %s: %s' % (name, results[name]))
    log('다음 단계: 각 레벨에 AXRayFadeMaterialProvider를 배치하고 그 레벨의 '
        '%s<레벨명> 을 지정한 뒤 레벨을 저장하세요.' % MAP_ASSET_PREFIX)


main()
