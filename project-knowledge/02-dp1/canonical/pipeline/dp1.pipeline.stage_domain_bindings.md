---
id: dp1.pipeline.stage_domain_bindings
title:
  uk: "Прив'язка етапів DP1 до доменів і структур"
  en: "DP1 stage to domain/structure bindings"
tags: [dp1, canonical, pipeline, stage-contract, data-domain]
kind: pipeline-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/pipeline/dp1.pipeline.stage_domain_bindings.md"
status: "draft"
---

## Definition

Ця картка задає pipeline-level огляд прив'язки DP1 stages до дозволених
input/context/output domains і structures. Stage0 нормалізації входу є canonical boundary перед `Prep`.

Мета картки — з'єднати stage-interface cards і data-domain cards, не
дублюючи повний per-stage contract. Джерелом істини для per-stage domain
bindings є відповідні stage-interface cards.

Cross-stage compatibility matrix винесено у
`dp1.pipeline.stage_io_matrix`. Ця картка описує binding sources і tile/merge
route, а Stage I/O Matrix задає послідовний input/output flow між етапами.

## Assumptions

- Це knowledge-only binding matrix, а не claim про поточну реалізацію в `datapro1_v2`.
- Конкретні algorithm variants мають уточнюватися у stage specs.
- `prep.variant` має кілька canonical routes: `full_frame`, `roi`, `tiles`,
  `adaptive_roi`. Tile-local structures описують тільки route
  `prep.variant = "tiles"` і не замінюють stage specs для інших варіантів.
- Один DP1 stage invocation працює в межах одного active `camera_id` /
  `source_id`; multi-camera aggregation не є внутрішнім stage contract DP1.

## Theorem / Contract

Кожен stage має працювати за загальною формою:

```text
process(input, context, config) -> output
```

This is the semantic stage form. C++ implementations may use route-specific overloads named `process(...)`, provided that input, runtime context/config, and explicit output roles remain separate. Route-specific execution names such as `processFullFrame(...)` or `processTile(...)` are not canonical.

де:
- `input` має належати дозволеному input domain/structure для цього stage;
- `context` має бути runtime context, а не контейнером stage output;
- `output` має бути explicit domain/structure output;
- internal buffers не мають ставати canonical output без явного domain contract.
- `input`, `context` і `output` мають бути узгоджені за одним
  `camera_id`/`source_id` у межах поточного DP1 instance.

## Stage-interface binding sources

```yaml
stage_interface_binding_sources:
  - stage: "acquisition_input_normalization"
    card: "../stages/dp1.stage.acquisition_input_normalization.md"
    id: "dp1.stage.acquisition_input_normalization"
  - stage: "prep"
    card: "../stages/dp1.stage.prep.md"
    id: "dp1.stage.prep"
  - stage: "radiometric_correction"
    card: "../stages/dp1.stage.radiometric_correction.md"
    id: "dp1.stage.radiometric_correction"
  - stage: "enhancement"
    card: "../stages/dp1.stage.enhancement.md"
    id: "dp1.stage.enhancement"
  - stage: "matched_filtering"
    card: "../stages/dp1.stage.matched_filtering.md"
    id: "dp1.stage.matched_filtering"
  - stage: "candidate_extraction"
    card: "../stages/dp1.stage.candidate_extraction.md"
    id: "dp1.stage.candidate_extraction"
  - stage: "segmentation_refinement"
    card: "../stages/dp1.stage.segmentation_refinement.md"
    id: "dp1.stage.segmentation_refinement"
  - stage: "object_filtering"
    card: "../stages/dp1.stage.object_filtering.md"
    id: "dp1.stage.object_filtering"
  - stage: "measurement"
    card: "../stages/dp1.stage.measurement.md"
    id: "dp1.stage.measurement"
```

Stage-interface cards визначають точні allowed input/context/output domains,
route-specific carriers і stage-local заборони. Ця pipeline card перевіряє, що
ці bindings сумісні між собою, з data-domain cards і з tile/merge route.

`Visualization` поки не входить до восьми main DP1 detection/measurement stage
cards. Артефакт visualization належить `dp1.domain.visualization` і не має
подаватися назад в обчислення без explicit stage spec.

## Stage0 binding

```yaml
stage0_binding:
  stage: "acquisition_input_normalization"
  input: "FramePacket"
  output: "CanonicalFrame"
  config_contract: "PipelineConfig.input_route + PipelineConfig.acquisition"
  frame_context_artifact: "canonical_frame"
  parent_artifact: "raw_frame"
  stage0_1_constraints:
    - "pass-through only"
    - "no binning"
    - "no pixel conversion"
    - "no ROI або tiles"
```

Stage0 формує canonical input boundary. `Prep` має працювати з
`CanonicalFrame`, а не виконувати нормалізацію source frame самостійно.

## Prep variant bindings

```yaml
prep_variant_bindings:
  - variant: "`full_frame`"
    role: "Весь кадр є одним processing unit."
    required_stage_spec_scope: "Full-frame memory ownership, allowed full-frame buffers, coordinate policy."
  - variant: "`roi`"
    role: "Обробляються явно задані ROI."
    required_stage_spec_scope: "ROI schema, bounds validation, local/global coordinates, optional split policy."
  - variant: "`tiles`"
    role: "Frame або ROI розбивається на `TileDesc[]`."
    required_stage_spec_scope: "Tile grid, border/overlap, valid area, tile merge, duplicate suppression."
  - variant: "`adaptive_roi`"
    role: "ROI вибираються динамічно."
    required_stage_spec_scope: "ROI source, state/fallback policy, miss-risk validation."
```

## Tile execution binding

Tile-local execution support structures використовуються для
`prep.variant = "tiles"` без зміни semantic stage contracts:

```yaml
tile_execution_structures:
  - structure: "`dp1.domain.runtime.tile_desc`"
    role: "опис tile/ROI, border, valid area"
    forbidden_as: "image buffer або stage output"
  - structure: "`dp1.domain.raw.tile_raw_view`"
    role: "read-only ROI view на `FramePacket.image`"
    forbidden_as: "owner image memory або mutable output"
  - structure: "`dp1.domain.processing.tile_processing_frame`"
    role: "tile-local processing payload"
    forbidden_as: "full-frame default buffer або mask"
  - structure: "`dp1.domain.mask.tile_binary_mask`"
    role: "tile-local binary mask payload"
    forbidden_as: "photometry source або grayscale processing frame"
  - structure: "`dp1.domain.runtime.tile_context`"
    role: "per-worker reusable buffers і diagnostics"
    forbidden_as: "global mutable state або final output"
  - structure: "`dp1.domain.runtime.tile_result`"
    role: "explicit tile-local results перед merge"
    forbidden_as: "final DP1 output без merge"
```

Tile-local outputs мають бути обрізані за valid area, перетворені в global coordinates і merged перед тим, як стати frame-level `MeasurementRecord` output.

## Tiles variant pipeline route

```yaml
tile_pipeline_route:
  - step: "prep"
    binding_source: "../stages/dp1.stage.prep.md"
    pipeline_output: "TileDesc[]"
    notes: "Будує tile grid, border і valid area."
  - step: "scheduler / worker setup"
    tile_local_input: "`CanonicalFrame` + `TileDesc`"
    runtime_owner_context: "`TileContext`"
    tile_local_output: "`TileRawView`"
    notes: "`TileRawView.image` є ROI view, не clone."
  - step: "radiometric_correction"
    binding_source: "../stages/dp1.stage.radiometric_correction.md"
  - step: "enhancement"
    binding_source: "../stages/dp1.stage.enhancement.md"
  - step: "matched_filtering"
    binding_source: "../stages/dp1.stage.matched_filtering.md"
  - step: "candidate_extraction"
    binding_source: "../stages/dp1.stage.candidate_extraction.md"
  - step: "segmentation_refinement"
    binding_source: "../stages/dp1.stage.segmentation_refinement.md"
  - step: "object_filtering"
    binding_source: "../stages/dp1.stage.object_filtering.md"
  - step: "measurement"
    binding_source: "../stages/dp1.stage.measurement.md"
    pipeline_output: "tile-local MeasurementRecord[] перед TileResult/merge"
  - step: "merge"
    tile_local_input: "`TileResult[]`"
    runtime_owner_context: "frame-level merge context"
    tile_local_output: "frame-level `MeasurementRecord[]`"
    notes: "Crop by `valid_area`, transform coordinates згідно з `dp1.domain.coordinates`, suppress border duplicates. Merge об'єднує tiles одного frame/source, не різні камери."
```

Memory route для `prep.variant = "tiles"`:

```text
CanonicalFrame.image     one canonical full-frame input carrier
TileDesc[]               small metadata
TileContext[worker_count] reusable tile-local buffers
TileResult[]             structural results only
MeasurementRecord[]      frame-level output after merge
```

Не є загальним canonical route і не має застосовуватися автоматично до
`full_frame`, `roi` або `adaptive_roi`:

```text
full raw frame
+ full processing frame per stage
+ full binary mask frame
+ hidden outputs in FrameContext
```

## Interpretation

Stage cards визначають межі відповідальності і per-stage domain bindings.
Data-domain cards визначають дозволені semantic objects. Ця pipeline card
з'єднує обидва шари, щоб implementation task могла обмежити, що кожен stage
має право читати і видавати.

Stage specs можуть звужувати дозволені domains, але не мають розширювати їх
без оновлення відповідної stage-interface card, цієї pipeline overview card
або окремо погодженої task card.

## Failure cases

- `candidate_extraction` напряму видає `MeasurementRecord`.
- `object_filtering` повертає filtered `Candidate[]` або `Segment[]` замість
  explicit `ValidatedObject[]`.
- `FrameContext` зберігає candidates, segments, masks або measurements як прихований output.
- Tile execution пише напряму в global measurement output без `TileResult` і merge semantics.
- Stage або tile merge змішує frames/results різних камер як один DP1 output.
- Visualization image використовується як computation input.
- Tile-specific structures використовуються для `full_frame`, `roi` або
  `adaptive_roi` без explicit stage spec.
- Tile worker мутує `CanonicalFrame.image` або source `FramePacket.image`.

## Typical misuse

- Вважати `cv::Mat` type достатнім для вибору stage input domain.
- Передавати `Candidate` або `Segment` у stages, які очікують Processing або Mask domain.
- Використовувати runtime context як заміну explicit input/output contracts.

## Open questions

- Exact stage naming synchronization із code-level names.
- Чи має `segmentation_refinement` приймати тільки `Candidate` + mask або також mask-only routes.

## Connections

- uses: dp1.pipeline.stage_contract
- uses: dp1.pipeline.stage_io_matrix
- uses: dp1.domain.raw.frame_packet
- uses: dp1.domain.raw.canonical_frame
- uses: dp1.domain.raw.tile_raw_view
- uses: dp1.domain.runtime.frame_context
- uses: dp1.domain.processing.frame
- uses: dp1.domain.processing.tile_processing_frame
- uses: dp1.domain.mask.binary_mask
- uses: dp1.domain.mask.tile_binary_mask
- uses: dp1.domain.struct.candidate
- uses: dp1.domain.struct.segment
- uses: dp1.domain.struct.validated_object
- uses: dp1.domain.measurement.record
- links: dp1.stage.acquisition_input_normalization
- links: dp1.stage.prep
- links: dp1.stage.radiometric_correction
- links: dp1.stage.enhancement
- links: dp1.stage.matched_filtering
- links: dp1.stage.candidate_extraction
- links: dp1.stage.segmentation_refinement
- links: dp1.stage.object_filtering
- links: dp1.stage.measurement
