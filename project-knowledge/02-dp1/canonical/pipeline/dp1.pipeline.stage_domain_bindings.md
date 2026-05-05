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
  lines: "1-N"
status: "draft"
---

## Definition

Ця картка задає canonical matrix прив'язки DP1 stages до дозволених input/context/output domains і structures.

Мета картки — не дозволити AI-кодеру довільно використовувати структури не в тому етапі або ховати stage outputs у `FrameContext` / `TileContext`.

## Assumptions

- Це knowledge-only binding matrix, а не claim про поточну реалізацію в `datapro1_v2`.
- Конкретні algorithm variants мають уточнюватися у stage specs.
- Tile-local execution structures є target execution-support model, not current runtime claim.

## Theorem / Contract

Кожен stage має працювати за загальною формою:

```text
process(input, context, config) -> output
```

де:
- `input` має належати дозволеному input domain/structure для цього stage;
- `context` має бути runtime context, а не прихованим контейнером stage output;
- `output` має бути explicit domain/structure output;
- internal buffers не мають ставати canonical output без явного domain contract.

## Canonical stage-domain binding matrix

| Stage | Allowed input | Runtime context | Allowed output | Notes |
|---|---|---|---|---|
| `prep` | `dp1.domain.raw.frame_packet` | `dp1.domain.runtime.frame_context` | `dp1.domain.raw.frame_packet` або `dp1.domain.processing.frame` | ROI/tile extraction and source preparation. No candidates/masks/measurements. |
| `radiometric_correction` | `dp1.domain.raw.frame_packet` або `dp1.domain.processing.frame` | `dp1.domain.runtime.frame_context` | `dp1.domain.processing.frame` | Produces corrected/residual processing representation. No `Mask`/`Struct` output. |
| `enhancement` | `dp1.domain.processing.frame` | `dp1.domain.runtime.frame_context` | `dp1.domain.processing.frame` | Denoise/enhance processing representation. |
| `matched_filtering` | `dp1.domain.processing.frame` | `dp1.domain.runtime.frame_context` | `dp1.domain.processing.frame` | Detector/response representation remains Processing domain. |
| `candidate_extraction` | `dp1.domain.processing.frame` | `dp1.domain.runtime.frame_context` | `dp1.domain.mask.binary_mask` + `dp1.domain.struct.candidate` | Candidate is provisional, not validated object. |
| `segmentation_refinement` | `dp1.domain.mask.binary_mask` + optional `dp1.domain.struct.candidate` | `dp1.domain.runtime.frame_context` | `dp1.domain.struct.segment` | Segments refine candidates/regions. |
| `object_filtering` | `dp1.domain.struct.candidate` або `dp1.domain.struct.segment` | `dp1.domain.runtime.frame_context` | `dp1.domain.struct.candidate` або `dp1.domain.struct.segment` | Filtering changes acceptance/quality, not measurement payload. |
| `measurement` | `dp1.domain.struct.segment` + optional `dp1.domain.raw.frame_packet` або `dp1.domain.processing.frame` for photometry | `dp1.domain.runtime.frame_context` | `dp1.domain.measurement.record` | Final DP1 product output for DP1 -> DP2 handoff. |
| `visualization` | any explicit domain object needed for display/debug | `dp1.domain.runtime.frame_context` | visualization artifact in `dp1.domain.visualization` | Visualization output must not feed computation unless a stage spec explicitly allows it. |

## Tile-local execution binding

Tile-local execution support structures may be used by runtime/execution model without changing semantic stage contracts:

| Runtime structure | Role | Must not be used as |
|---|---|---|
| `dp1.domain.runtime.tile_desc` | опис tile/ROI, border, valid area | image buffer або stage output |
| `dp1.domain.runtime.tile_context` | per-tile/per-worker working buffers and diagnostics | global mutable state або final output |
| `dp1.domain.runtime.tile_result` | explicit tile-local results before merge | final DP1 output without merge |

Tile-local outputs must be cropped by valid area, transformed to global coordinates, and merged before becoming frame-level `MeasurementRecord` output.

## Interpretation

Stage cards define responsibility boundaries. Data-domain cards define allowed semantic objects. This binding matrix connects both layers so an implementation task can restrict what each stage may read and emit.

Stage specs may narrow the allowed domains further, but should not broaden them without updating this binding matrix or an approved task card.

## Failure cases

- `measurement` reads directly from `MaskU8` as if it were photometry.
- `candidate_extraction` emits `MeasurementRecord` directly.
- `FrameContext` stores candidates, segments, masks, or measurements as hidden payload.
- Tile worker writes directly into global measurement output without `TileResult`/merge semantics.
- Visualization image is used as computation input.

## Typical misuse

- Treating `cv::Mat` type as enough to select stage input domain.
- Passing `Candidate` or `Segment` to stages that expect Processing or Mask domain.
- Using runtime context as a substitute for explicit input/output contracts.

## Open questions

- Exact stage naming synchronization with code-level names.
- Whether `object_filtering` should introduce a separate accepted-object structure after MVP.
- Whether `segmentation_refinement` should accept only `Candidate` + mask or also mask-only routes.

## Connections

- uses: dp1.pipeline.stage_contract
- uses: dp1.domain.raw.frame_packet
- uses: dp1.domain.runtime.frame_context
- uses: dp1.domain.processing.frame
- uses: dp1.domain.mask.binary_mask
- uses: dp1.domain.struct.candidate
- uses: dp1.domain.struct.segment
- uses: dp1.domain.measurement.record
- informs: dp1.stage.candidate_extraction
- informs: dp1.stage.segmentation_refinement
- informs: dp1.stage.object_filtering
- informs: dp1.stage.measurement
