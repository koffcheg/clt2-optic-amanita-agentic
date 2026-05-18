---
id: dp1.pipeline.stage_io_matrix
title:
  uk: "Матриця входів і виходів етапів DP1"
  en: "DP1 stage input/output matrix"
tags: [dp1, canonical, pipeline, stage-contract, data-domain, code-generation]
kind: pipeline-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/pipeline/dp1.pipeline.stage_io_matrix.md"
status: "draft"
---

## Definition

Ця картка задає canonical Stage I/O Matrix для Stage0 нормалізації входу,
восьми основних computational етапів DP1 і tile merge boundary.
Вона описує, які domain carriers можуть передаватися між етапами, щоб AI-кодер
не виводив pipeline topology з неформального тексту.

## Assumptions

- Matrix описує interface-level flow, а не implementation algorithm.
- Stage specs можуть звужувати allowed input/output, але не мають розширювати
  їх без оновлення stage-interface card або цієї matrix.
- Tile-local route є route variant, а не окрема pipeline semantics.

## Theorem / Contract

Canonical flow для default computational route:

```text
FramePacket
  -> CanonicalFrame
  -> Raw view / TileRawView / ROI raw view
  -> ProcessingFrame або TileProcessingFrame (RadiometricResidual, F32 або explicit signed residual route)
  -> ProcessingFrame або TileProcessingFrame (EnhancedFrame, F32)
  -> ProcessingFrame або TileProcessingFrame (DetectorResponse, F32)
  -> BinaryMask або TileBinaryMask (MaskU8) + Candidate[]
  -> Segment[]
  -> ValidatedObject[]
  -> MeasurementRecord[]
```

Скорочений route для AI-кодера:

```text
Raw U8/U16 -> CanonicalFrame -> Processing F32 -> Processing F32 -> DetectorResponse F32 -> MaskU8 -> Struct -> MeasurementRecord
```

Stage I/O matrix:

```yaml
stage_io_matrix:
  - stage: "input"
    input: []
    context: ["PipelineConfig"]
    output:
      - "FramePacket"
    context_artifacts:
      - semantic_name: "raw_frame"
        kind: "RawFrame"
        domain: "Raw"
        producer_stage: "input"
        parent_artifact_id: null
        ownership: "OwnedByFramePacket"
        lifetime: "InputBoundary"
        status: "Available"
    required_domain: "Raw/Input"
    notes: "Input route задає U8 або U16 carrier і фактичну bit depth."

  - stage: "acquisition"
    stage_number: 0
    input:
      - "FramePacket"
    context:
      - "FrameContext"
      - "PipelineConfig.input_route"
      - "PipelineConfig.acquisition"
    output:
      - "CanonicalFrame"
    context_artifacts:
      - semantic_name: "canonical_frame"
        kind: "CanonicalFrame"
        domain: "Raw/CanonicalInput"
        producer_stage: "acquisition"
        parent_artifact_id: "raw_frame"
        ownership: "BorrowedReadOnly у Stage0.1 pass-through"
        lifetime: "InputBoundary або FrameBoundary, якщо implementation явно гарантує lifetime"
        status: "Available"
    required_domain: "Raw/Input + Acquisition"
    notes: "Stage0.1 перевіряє input_route і формує CanonicalFrame без binning, conversion, ROI або tiles."

  - stage: "prep"
    input:
      - "CanonicalFrame"
    context:
      - "FrameContext"
      - "PipelineConfig.prep"
    output:
      - "CanonicalFrame view route"
      - "TileDesc[]"
      - "TileRawView[] або equivalent route-local raw views"
    context_artifacts:
      - semantic_name: "canonical_frame"
        kind: "CanonicalFrame"
        domain: "Raw/CanonicalInput"
        producer_stage: "acquisition"
        parent_artifact_id: "raw_frame"
        status: "consumed/provenance source; prep does not create a new authoritative frame artifact by default"
    required_domain: "Raw/Input + Runtime"
    notes: "Не виконує binning, pixel-format normalization або bit-depth conversion; не створює candidates, segments, objects або measurements."

  - stage: "radiometric_correction"
    input:
      - "CanonicalFrame або raw view"
      - "TileRawView у tiles route"
    context:
      - "FrameContext"
      - "TileContext у tiles route"
      - "PipelineConfig.radiometric_correction"
    output:
      - "ProcessingFrame"
      - "TileProcessingFrame у tiles route"
    context_artifacts:
      - semantic_name: "radiometric.processing_frame"
        kind: "ProcessingFrame"
        domain: "Processing"
        producer_stage: "radiometric_correction"
        parent_artifact_id: "raw_frame"
        ownership: "OwnedByStageOutput"
        lifetime: "StageOutputScope"
        status: "MetadataOnly у current C++ slice, якщо FrameContext не зберігає typed payload reference"
    required_domain: "Processing"
    notes: "Output domain має явно вказати processing_domain і range_policy."

  - stage: "enhancement"
    input:
      - "ProcessingFrame"
      - "TileProcessingFrame у tiles route"
    context:
      - "FrameContext"
      - "TileContext у tiles route"
      - "PipelineConfig.enhancement"
    output:
      - "ProcessingFrame"
      - "TileProcessingFrame у tiles route"
    context_artifacts:
      - semantic_name: "enhanced_frame"
        kind: "ProcessingFrame"
        domain: "Processing"
        producer_stage: "enhancement"
        parent_artifact_id: "radiometric.processing_frame"
        status: "planned canonical artifact"
    required_domain: "Processing"
    notes: "Output не є detector response, якщо variant явно не задає це як contract."

  - stage: "matched_filtering"
    input:
      - "ProcessingFrame"
      - "TileProcessingFrame у tiles route"
    context:
      - "FrameContext"
      - "TileContext у tiles route"
      - "PipelineConfig.matched_filtering"
    output:
      - "ProcessingFrame(DetectorResponse)"
      - "TileProcessingFrame(DetectorResponse) у tiles route"
    context_artifacts:
      - semantic_name: "detector_response"
        kind: "ProcessingFrame"
        domain: "Processing"
        producer_stage: "matched_filtering"
        parent_artifact_id: "enhanced_frame"
        status: "planned canonical artifact"
    required_domain: "Processing"
    notes: "Карта відгуку не є visualization image."

  - stage: "candidate_extraction"
    input:
      - "ProcessingFrame(DetectorResponse або allowed processing domain)"
      - "TileProcessingFrame у tiles route"
    context:
      - "FrameContext"
      - "TileContext у tiles route"
      - "PipelineConfig.candidate_extraction"
    output:
      - "BinaryMask"
      - "TileBinaryMask у tiles route"
      - "Candidate[]"
    context_artifacts:
      - semantic_name: "binary_mask"
        kind: "BinaryMask"
        domain: "Mask"
        producer_stage: "candidate_extraction"
        parent_artifact_id: "detector_response"
        status: "planned canonical artifact"
      - semantic_name: "candidate_list"
        kind: "CandidateSet"
        domain: "Struct"
        producer_stage: "candidate_extraction"
        parent_artifact_id: "binary_mask"
        status: "planned canonical artifact"
    required_domain: "Mask + Struct"
    notes: "Не видає Segment, ValidatedObject або MeasurementRecord."

  - stage: "segmentation_refinement"
    input:
      - "BinaryMask + optional Candidate[]"
      - "TileBinaryMask + optional Candidate[] у tiles route"
    context:
      - "FrameContext"
      - "TileContext у tiles route"
      - "PipelineConfig.segmentation_refinement"
    output:
      - "Segment[]"
    context_artifacts:
      - semantic_name: "segment_list"
        kind: "SegmentSet"
        domain: "Struct"
        producer_stage: "segmentation_refinement"
        parent_artifact_id: "binary_mask"
        status: "planned canonical artifact"
    required_domain: "Struct"
    notes: "Segment є проміжним object region, не final measurement."

  - stage: "object_filtering"
    input:
      - "Candidate[] або Segment[]"
      - "optional Raw/Processing photometry reference if stage interface allows"
    context:
      - "FrameContext"
      - "TileContext у tiles route"
      - "PipelineConfig.object_filtering"
    output:
      - "ValidatedObject[]"
    context_artifacts:
      - semantic_name: "validated_object_list"
        kind: "ValidatedObjectSet"
        domain: "Struct"
        producer_stage: "object_filtering"
        parent_artifact_id: "segment_list"
        status: "planned canonical artifact"
    required_domain: "Struct"
    notes: "Filtered candidates не замінюють ValidatedObject[] output."

  - stage: "measurement"
    input:
      - "ValidatedObject[]"
      - "optional Segment[]"
      - "optional Raw/Processing photometry reference"
    context:
      - "FrameContext"
      - "TileContext у tiles route"
      - "PipelineConfig.measurement"
    output:
      - "MeasurementRecord[]"
    context_artifacts:
      - semantic_name: "measurement_list"
        kind: "MeasurementSet"
        domain: "Measurement"
        producer_stage: "measurement"
        parent_artifact_id: "validated_object_list"
        status: "planned canonical artifact"
    required_domain: "Measurement"
    notes: "У tiles route output є tile-local до TileResult і merge."

  - stage: "merge"
    input:
      - "TileResult[]"
    context:
      - "FrameContext"
    output:
      - "frame-level MeasurementRecord[]"
      - "future FrameMeasurementBatch"
    context_artifacts:
      - semantic_name: "measurement_list"
        kind: "MeasurementSet"
        domain: "Measurement"
        producer_stage: "merge"
        parent_artifact_id: "measurement_list"
        status: "planned canonical frame-level artifact for tiles route"
    required_domain: "Measurement"
    notes: "Merge виконує valid_area crop, globalization згідно з `dp1.domain.coordinates` і duplicate suppression."
```

Visualization domain не входить у computational flow. Visualization artifacts
не мають передаватися назад у stages 1-8 без окремого explicit stage spec.

`context_artifacts` описує очікувані записи у `FrameContext.artifacts`.
Ці записи є audit/provenance reflection для авторитетних products кадру. Вони
не замінюють explicit `output`, не є transport container і не вимагають, щоб
`FrameContext` володів heavy buffers. Для current raw + radiometric C++ slice
`ownership`, `lifetime` і `status` мають читатися разом: raw frame є
`OwnedByFramePacket + InputBoundary + Available`, а radiometric output reflection
є `OwnedByStageOutput + StageOutputScope + MetadataOnly`, якщо context не
зберігає live typed payload reference. `status: "planned canonical artifact"` позначає очікування для канонічних
етапів, які описані матрицею, але не вимагає їх реалізації в межах цієї картки.

## Fields / Interface

Кожен рядок matrix визначає:

```yaml
fields:
  - name: "stage"
    meaning: "Canonical stage id або pipeline support boundary."
  - name: "input"
    meaning: "Allowed domain carriers."
  - name: "context"
    meaning: "Allowed runtime/config context, не output container."
  - name: "output"
    meaning: "Explicit result carriers."
  - name: "context_artifacts"
    meaning: "Expected `FrameContext.artifacts` records для authoritative output/provenance reflection; metadata/provenance layer, не payload ownership або lifetime-extension requirement."
  - name: "required_domain"
    meaning: "Semantic domain output."
  - name: "notes"
    meaning: "Stage-level constraints для future code-generation task."
```

## Input / Output

Input:

- `dp1.pipeline.stage_contract`;
- stage-interface cards;
- data-domain structure cards;
- `PipelineConfig`.

Output:

- canonical cross-stage compatibility matrix;
- validation source для `dp1.validation.stage_contract_checks`.

## Constraints

- Stage output має бути explicit structure, а не hidden field у `FrameContext`.
- Кожен успішний authoritative output має бути відображений у
  `FrameContext.artifacts` згідно з `context_artifacts`.
- `context_artifacts` не вимагає deep copy, ownership або lifetime extension для
  referenced payload.
- `MaskU8` не має передаватися як Processing-domain grayscale image.
- `CV_8U` processing route дозволений лише як explicit fast/compatibility route.
- Raw photometry має читати Raw/Input carrier або explicitly allowed
  Processing-domain source, не visualization image.
- Tile-local outputs не є final frame-level outputs до merge.

## Failure cases

- `candidate_extraction` напряму формує DP2 payload.
- `object_filtering` повертає filtered `Candidate[]` замість `ValidatedObject[]`.
- `measurement` читає debug visualization як photometry source.
- Tile worker пише напряму у global `MeasurementRecord[]`.
- Stage виконує implicit `convertTo` і змінює domain без output metadata.
- Stage повертає authoritative output явно, але не має відповідного запису в
  `FrameContext.artifacts`.
- Pipeline реєструє debug artifact замість authoritative output artifact і
  вважає reflection rule виконаним.

## Typical misuse

- Вважати `cv::Mat` достатнім type contract між stages.
- Використовувати `FrameContext` як загальний mutable output container.
- Вважати `TileProcessingFrame` допустимим full-frame output для всіх routes.
- Трактувати `context_artifacts` як вимогу реалізувати missing stages або
  перенести heavy buffers у `FrameContext`.

## Open questions

- Exact duplicate suppression policy на tile borders.
- Чи має frame-level batch бути частиною Measurement domain або protocol card.
- Повна alias policy для config keys `radiometric` / `radiometric_correction`,
  `matched_filter` / `matched_filtering`, `segmentation` /
  `segmentation_refinement`.

## Connections

- uses: dp1.pipeline.stage_contract
- uses: dp1.pipeline.stage_domain_bindings
- uses: dp1.config.stage_variant_registry
- uses: dp1.domain.raw.frame_packet
- uses: dp1.domain.raw.canonical_frame
- uses: dp1.domain.raw.tile_raw_view
- uses: dp1.domain.processing.frame
- uses: dp1.domain.processing.tile_processing_frame
- uses: dp1.domain.mask.binary_mask
- uses: dp1.domain.mask.tile_binary_mask
- uses: dp1.domain.struct.candidate
- uses: dp1.domain.struct.segment
- uses: dp1.domain.struct.validated_object
- uses: dp1.domain.measurement.record
- constrained_by: dp1.domain.opencv_invariants
- checked_by: dp1.validation.stage_contract_checks
