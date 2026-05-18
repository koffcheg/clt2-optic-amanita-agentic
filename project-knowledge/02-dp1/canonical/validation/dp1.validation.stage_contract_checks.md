---
id: dp1.validation.stage_contract_checks
title:
  uk: "Перевірки контрактів етапів DP1"
  en: "DP1 stage contract checks"
tags: [dp1, canonical, validation, stage-contract, code-generation]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/02-dp1/canonical/validation/dp1.validation.stage_contract_checks.md"
status: "draft"
---

## Definition

Ця картка визначає knowledge-level checklist перевірок, які future code
generation task має врахувати для кожної stage implementation. Вона не створює
автоматизовані тести і не визначає runtime test workflow.

## Assumptions

- Автоматизовані тести не створюються в межах цієї knowledge-only підготовки.
- Практичне виконання checks має бути описане окремо у validation execution
  route або implementation task.
- Stage specs можуть додавати variant-specific checks.

## Theorem / Contract

Для кожної stage implementation мають бути перевірки:

```yaml
stage_contract_checks:
  - check: "StageConfig contains enabled, variant, level, parameters"
    source: "dp1.config.pipeline_configuration_c"
  - check: "stage.variant exists in dp1.config.stage_variant_registry"
    source: "dp1.config.stage_variant_registry"
  - check: "stage.level is compatible with dp1.config.complexity_levels"
    source: "dp1.config.complexity_levels"
  - check: "input carrier matches dp1.pipeline.stage_io_matrix"
    source: "dp1.pipeline.stage_io_matrix"
  - check: "output carrier matches stage-interface card"
    source: "canonical/stages/*.md"
  - check: "cv::Mat invariants pass before OpenCV primitive call"
    source: "dp1.domain.opencv_invariants"
  - check: "no implicit conversion changes domain without metadata"
    source: "dp1.domain.conversion_rules"
  - check: "FrameContext is not used as hidden output owner"
    source: "dp1.domain.runtime.frame_context"
  - check: "кореневий artifact raw_frame існує після успішного створення FramePacket"
    source: "dp1.domain.runtime.frame_context + dp1.pipeline.stage_io_matrix"
  - check: "завершений авторитетний вихід етапу має відповідний запис у FrameContext.artifacts"
    source: "dp1.pipeline.stage_contract + dp1.pipeline.stage_io_matrix"
  - check: "етап зі статусом failed, unsupported, skipped або disabled не реєструє успішний output artifact"
    source: "dp1.pipeline.stage_contract + dp1.domain.runtime.frame_context"
  - check: "artifact semantic_name, kind і domain узгоджені з context_artifacts у dp1.pipeline.stage_io_matrix"
    source: "dp1.pipeline.stage_io_matrix"
  - check: "некореневий output artifact має parent_artifact_id, пов'язаний із source artifact"
    source: "dp1.domain.runtime.frame_context + dp1.pipeline.stage_io_matrix"
  - check: "producer_stage відповідає stable stage key або approved runtime boundary"
    source: "dp1.domain.runtime.frame_context + dp1.config.stage_variant_registry"
  - check: "записи artifact registry не означають прихованого володіння heavy buffers або подовження borrowed lifetime"
    source: "dp1.domain.runtime.frame_context + dp1.domain.memory_ownership"
  - check: "artifact ownership, lifetime і status разом не створюють хибне враження live payload reference"
    source: "dp1.domain.runtime.frame_context + dp1.domain.memory_ownership"
  - check: "Visualization domain is not used as computation input"
    source: "dp1.domain.visualization"
  - check: "each executed stage contributes one StageTiming record"
    source: "PROFILING_POLICY.md + dp1.domain.profiling"
  - check: "profiling StageKey matches stage registry for core DP1 stages"
    source: "PROFILING_POLICY.md + dp1.domain.profiling + dp1.config.stage_variant_registry"
  - check: "variant and level are recorded separately in profiling records"
    source: "PROFILING_POLICY.md + dp1.domain.profiling"
  - check: "duration units are explicit and use monotonic runtime source"
    source: "PROFILING_POLICY.md + dp1.domain.profiling + dp1.domain.time"
  - check: "format conversions and large copies are timed or counted"
    source: "PROFILING_POLICY.md + dp1.domain.profiling + dp1.domain.conversion_rules"
  - check: "cardinality metrics are updated where a stage creates counted entities"
    source: "PROFILING_POLICY.md + dp1.domain.profiling"
  - check: "raw profiling trace and aggregation windows are bounded"
    source: "PROFILING_POLICY.md + dp1.domain.profiling + dp1.config.application.profiling"
  - check: "runtime structures expose profiling through a field named profiling with scope-specific type"
    source: "PROFILING_POLICY.md + dp1.domain.profiling + dp1.domain.runtime"
  - check: "FrameContext.artifacts не використовується як сховище даних профілювання або друга модель профілювання"
    source: "PROFILING_POLICY.md + dp1.domain.profiling + dp1.config.application.profiling + dp1.domain.runtime.frame_context"
  - check: "записи artifact можуть посилатися на `producer_stage` або ідентичність timing тільки як метадані походження"
    source: "PROFILING_POLICY.md + dp1.domain.profiling + dp1.domain.runtime.frame_context"
  - check: "записи профілювання не використовуються для передачі алгоритмічних виходів між stages"
    source: "PROFILING_POLICY.md + dp1.domain.profiling + dp1.pipeline.stage_contract"
```

Перевірки відображення artifacts у FrameContext:

```yaml
frame_context_artifact_reflection_checks:
  - check: "успішна input boundary реєструє raw_frame як кореневий artifact FrameContext"
    source: "dp1.pipeline.stage_io_matrix"
    scope: "pipeline-level"
  - check: "raw_frame має producer_stage=input, domain=Raw, kind=RawFrame і parent_artifact_id=null"
    source: "dp1.pipeline.stage_io_matrix + dp1.domain.runtime.frame_context"
    scope: "pipeline-level"
  - check: "кожен завершений авторитетний вихід етапу повертається явно і відображається у FrameContext.artifacts"
    source: "dp1.pipeline.stage_contract"
    scope: "stage-level"
  - check: "відображений artifact має semantic_name, kind, domain і producer_stage, узгоджені з рядком stage_io_matrix"
    source: "dp1.pipeline.stage_io_matrix"
    scope: "stage-level"
  - check: "некореневі artifact records містять parent_artifact_id для provenance"
    source: "dp1.domain.runtime.frame_context"
    scope: "stage-level"
  - check: "етапи зі статусом failed, unsupported, skipped або disabled можуть записувати status/diagnostics, але не реєструють успішні output artifacts"
    source: "dp1.pipeline.stage_contract"
    scope: "stage-level"
  - check: "diagnostic, visualization або profiling artifacts не приймаються як заміна авторитетних output artifacts"
    source: "dp1.pipeline.stage_contract + dp1.pipeline.stage_io_matrix"
    scope: "stage-level"
  - check: "metadata ownership, lifetime і status для artifact не подовжують borrowed або stage-output payload lifetime"
    source: "dp1.domain.runtime.frame_context + dp1.domain.memory_ownership"
    scope: "stage-level"
  - check: "radiometric.processing_frame у current C++ slice є StageOutputScope metadata/provenance record, якщо FrameContext не зберігає typed payload reference"
    source: "dp1.domain.runtime.frame_context + dp1.pipeline.stage_io_matrix"
    scope: "stage-level"
```

Tile-route checks:

```yaml
tile_route_checks:
  - check: "TileDesc.tile_id is unique inside frame"
  - check: "TileDesc.roi_with_border is inside FramePacket geometry"
  - check: "TileDesc.valid_area is inside TileRawView.image extent"
  - check: "TileRawView.image is non-owning ROI view"
  - check: "TileRawView.image may be non-continuous"
  - check: "Tile-local outputs are cropped by valid_area before acceptance"
  - check: "Tile-local coordinates are converted to FrameGlobal before final output"
  - check: "TileResult does not carry image buffers"
  - check: "Tile profiling summary uses dp1.domain.profiling and does not carry raw image payload"
  - check: "Duplicate suppression policy is applied or marked blocking"
```

Measurement-boundary checks:

```yaml
measurement_checks:
  - check: "MeasurementRecord has frame_id, camera_id/source relation, time_ref"
  - check: "MeasurementRecord coordinate_space is FrameGlobal for DP1 -> DP2 handoff"
  - check: "Photometry source is Raw/Input or explicitly allowed Processing domain"
  - check: "DP1 -> DP2 handoff does not require debug images, masks, or temporary buffers"
  - check: "Payload schema/version policy is present or marked blocking"
```

## Fields / Interface

Checklist entries мають такі поля:

```yaml
fields:
  - name: "check"
    meaning: "Що має бути перевірено."
  - name: "source"
    meaning: "Canonical source card для правила."
  - name: "scope"
    meaning: "Stage-level, tile-route, measurement-boundary або pipeline-level."
```

## Input / Output

Input:

- canonical stage-interface cards;
- data-domain cards;
- configuration cards;
- pipeline Stage I/O Matrix.
- `dp1.domain.profiling`;
- `dp1.config.application.profiling`.

Output:

- checklist для future implementation and review;
- перелік blocking open questions для code-generation readiness.

## Constraints

- Ця картка не створює test files.
- Ця картка не замінює `05-validation` execution workflow.
- Якщо check потребує stage-specific algorithm details, він має бути перенесений
  у future stage spec.

## Failure cases

- Code generation починається без перевірки registered variant.
- Stage приймає `CV_8UC1` без розрізнення Raw U8 і MaskU8.
- Tile-local measurement потрапляє у DP2 handoff без globalization.
- Missing duplicate suppression policy приховується замість explicit blocker.
- Profiling event записаний як log message instead of structured profiling data.
- Stage implementation не створює `StageTiming` для executed stage.
- `StageTiming` для current C++ slice відсутній або не містить stable stage key, status, variant, level і monotonic duration.
- Raw profiling trace не має configured bound.
- `FrameContext.artifacts` використано як profiling storage або як друга модель
  profiling замість `PROFILING_POLICY.md`, `dp1.domain.profiling` і
  `dp1.config.application.profiling`.
- Profiling record або profiling artifact використано для передачі
  алгоритмічного output між stages.
- Успішний `FramePacket` не має кореневого artifact `raw_frame`.
- Авторитетний вихід етапу повернуто явно, але відповідний запис у `FrameContext.artifacts` відсутній.
- Етап зі статусом `failed`, `unsupported`, `skipped` або `disabled` реєструє фіктивний успішний output artifact.
- `diagnostics`, `visualization` або `profiling` artifact помилково зараховано як заміну авторитетного output artifact.
- `radiometric.processing_frame` позначено як payload-available без live typed reference у `FrameContext`.
- Некореневий artifact не має `parent_artifact_id`, тому provenance chain розірваний.

## Typical misuse

- Трактувати checklist як automated test suite.
- Закривати open questions неявними assumptions у code.
- Використовувати цю картку як дозвіл змінювати source code.

## Open questions

- Exact runtime mechanism для reporting failed checks.
- Які checks мають бути compile-time/static, а які runtime.
- Як `05-validation` має materialize ці checks у майбутніх validation routes.

## Connections

- uses: dp1.pipeline.stage_contract
- uses: dp1.pipeline.stage_io_matrix
- uses: dp1.config.stage_variant_registry
- uses: dp1.domain.opencv_invariants
- uses: dp1.domain.common_types
- uses: dp1.domain.profiling
- uses: dp1.domain.conversion_rules
- uses: dp1.domain.coordinates
- uses: dp1.domain.measurement.record
- extends: dp1.validation.canonical_conformance
