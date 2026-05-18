---
id: dp1.stage.acquisition_input_normalization
title:
  uk: "Stage0 нормалізації входу DP1"
  en: "DP1 Stage0 input normalization"
tags: [dp1, canonical, stage, acquisition, input-normalization, canonical-frame]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.acquisition_input_normalization.md"
status: "draft"
---

## Definition

`Stage0 Input Normalization` є вхідним canonical stage DP1, який приймає
`FramePacket`, перевіряє його проти `PipelineConfig.input_route` і формує
`CanonicalFrame` для подальших етапів DP1.

Stage0 відповідає за підготовку самого кадру як canonical input. Stage0 не
визначає, як цей кадр буде оброблятися просторово. Просторовий route
`full_frame`, `roi`, `tiles` або `adaptive_roi` належить `Stage1 Prep`.

## Interface

`IAcquisitionInputNormalizationStage`.

## Contract

```text
process(input, context, config) -> output
```

де:

- `input` є `FramePacket`;
- `context` є `FrameContext`;
- `config` містить `PipelineConfig.input_route` і фрагмент
  `PipelineConfig.acquisition`;
- `output` є explicit `CanonicalFrame`.

Для Stage0.1 контракт обмежений pass-through route: Stage0 тільки перевіряє
вхідний кадр і створює borrowed/read-only `CanonicalFrame`, якщо payload уже
відповідає `input_route`.

## Algorithmic idea

Stage0 відділяє сирий input boundary від canonical DP1 pipeline input.

Stage0.1:

1. читає metadata `FramePacket`;
2. порівнює фактичний carrier, pixel format, bit depth, pixel range, geometry і
   stride з `PipelineConfig.input_route`;
3. якщо pass-through без перетворення можливий, формує `CanonicalFrame`;
4. реєструє authoritative artifact `canonical_frame` у `FrameContext`;
5. записує `StageTiming` для `StageKey = "acquisition"`;
6. якщо потрібне перетворення, binning або unsupported route, повертає failure
   з явною причиною.

## Inputs

- `FramePacket`;
- `FrameContext`;
- `PipelineConfig.input_route`;
- `PipelineConfig.acquisition`;
- metadata source frame: `frame_id`, `camera_id`, `source_id`, timestamp,
  geometry, stride, pixel format, bit depth і pixel range.

## Internal computation domain

Stage0.1 не виконує обчислень над pixels. Для pass-through route `cv::Mat` або
еквівалентний image carrier не копіюється і не конвертується.

Майбутній Stage0.2 може додати bounded normalization або binning тільки через
окрему StageSpec.

## Outputs

- `CanonicalFrame`;
- `FrameContext.artifacts` запис для `canonical_frame`;
- `StageTiming` для `acquisition`.

## Domain bindings

```yaml
frame_level_binding:
  allowed_input: "dp1.domain.raw.frame_packet"
  runtime_context: "dp1.domain.runtime.frame_context"
  allowed_output:
    - "dp1.domain.raw.canonical_frame"
  notes: "Stage0.1 формує pass-through CanonicalFrame без binning або conversion."
route_specific_carriers:
  - route: "passthrough"
    input_carrier: "FramePacket"
    output_carrier: "CanonicalFrame"
```

## Complexity variants

- `L0`: pass-through validation and wrapping.
- `L1`: reserved for bounded input normalization after an approved StageSpec.

## OpenCV mapping

- `cv::Mat` view/wrapper: `native`;
- `convertTo`: forbidden in Stage0.1;
- `resize`: forbidden in Stage0.1;
- binning kernels: forbidden in Stage0.1.

## Config fragment

Ключ DSL: `acquisition`.

Обов'язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимий для Stage0.1 `variant`: `passthrough`.

`PipelineConfig.input_route` є validation contract для Stage0.1. Якщо вхідний
`FramePacket` не відповідає `input_route`, Stage0 має повернути explicit
failure замість прихованої конверсії.

## Timing / profiling

Stage0 має записувати stage-level timing:

- `StageKey = "acquisition"`;
- validation/wrap duration;
- status `Completed` або explicit failure status;
- optional cardinality metadata: width, height, payload bytes.

Operation-level timings для conversion/binning не входять у Stage0.1.

## Must not do

- Не виконувати binning у Stage0.1.
- Не виконувати pixel-value conversion у Stage0.1.
- Не виконувати implicit upscale/downscale.
- Не будувати ROI.
- Не будувати tiles або `TileDesc[]`.
- Не запускати radiometric, enhancement, detection або measurement logic.
- Не розширювати CameraProSim runtime.
- Не змінювати DP2, OverlayRunner або IPC layout.

## Constraints

- Кожен успішний authoritative `CanonicalFrame` має бути відображений у
  `FrameContext.artifacts`.
- Artifact має посилатися на parent `raw_frame` або equivalent `FramePacket`
  artifact.
- Ownership/lifetime artifact не має вводити в оману: borrowed payload не
  стає owned payload через registry.
- Якщо payload reference не переживає declared boundary, artifact має бути
  `MetadataOnly` або `ExpiredReference`.

## Failure cases

- Source carrier mismatch.
- Unsupported pixel format.
- Unsupported bit depth.
- Invalid geometry або stride.
- Ambiguous pixel range.
- Required conversion unsupported by Stage0.1.
- Binning requested but no approved Stage0.2 StageSpec exists.

## Typical misuse

- Виконувати binning у `Prep`.
- Вважати `CanonicalFrame` просто alias для `FramePacket` без provenance.
- Виконувати прихований `convertTo` при mismatch `input_route`.
- Реєструвати `canonical_frame` у `FrameContext`, але не повертати explicit
  `CanonicalFrame` output.

## Open questions

- Exact C++ class/interface names для Stage0.1 implementation.
- Чи буде `SharedFrameView` окремим input carrier перед `FramePacket` у
  майбутньому IPC route.
- Повна StageSpec для Stage0.2 binning.

## Connections

- uses: dp1.domain.raw.frame_packet
- produces: dp1.domain.raw.canonical_frame
- uses: dp1.domain.runtime.frame_context
- uses: dp1.config.pipeline_configuration_c
- constrained_by: dp1.config.stage_variant_registry
- constrained_by: dp1.pipeline.stage_io_matrix
- constrained_by: dp1.pipeline.stage_domain_bindings
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- feeds: dp1.stage.prep
