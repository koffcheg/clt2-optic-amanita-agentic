# DP1_CANONICAL_INDEX

## Purpose

Canonical entry point for DP1 as an engineering product.

This section defines target DP1 architecture independently from legacy `datapro1` code. Legacy materials may be used only when a task card explicitly allows them.

## Source of truth route

1. `product/dp1.canonical.product_definition.md`
2. `product/dp1.canonical.source_of_truth.md`
3. `pipeline/dp1.pipeline.formal_model.md`
4. `pipeline/dp1.pipeline.stage_contract.md`
5. `pipeline/dp1.pipeline.stage_domain_bindings.md`
6. `pipeline/dp1.pipeline.stage_io_matrix.md`
7. `data_domains/*.md` and `data_domains/structures/**/*.md`
8. `stages/*.md`
9. `configuration/dp1.config.pipeline_configuration_c.md`
10. `configuration/dp1.config.stage_variant_registry.md`
11. `configuration/dp1.config.complexity_levels.md`
12. `stage_specs/*.md`
13. `validation/dp1.validation.canonical_conformance.md`
14. `validation/dp1.validation.stage_contract_checks.md`
15. `../../04-protocols/cards/protocols.dp1_dp2.measurement_handoff.md`

## Code generation rule

```text
Code = f(Cards, Stage_Spec, C)
```

Code generation from informal text, legacy code, or legacy-reference cards is forbidden.

Stage-interface cards define boundaries, technical-requirements-derived
interface constraints, complexity variants, OpenCV mapping, configuration
fragments, data formats, and critical invariants. They are not sufficient for
code generation without small stage specifications.

Data-domain cards define the canonical domains and boundary structures that
move through stages. Algorithms must reference these cards instead of inventing
local frame, mask, candidate, segment, or measurement structures.

Stage-interface cards define the authoritative per-stage domain bindings:
which domain structures each stage may read and emit. The stage-domain binding
pipeline card links those eight stage cards and keeps cross-stage route,
tile/merge, and hidden-output consistency rules.

## Single-Camera DP1 Instance Invariant

One active DP1 instance accepts one camera/source input. A `FramePacket`,
`FrameContext`, stage invocation, tile merge, and frame-level
`MeasurementRecord[]` output all belong to one `camera_id` / `source_id`.

Multi-camera scenarios are represented by multiple DP1 instances or by a
downstream DP2/orchestration aggregation boundary. Canonical DP1 cards do not
define a multi-camera input container inside one DP1 instance.

## Current data domains

- `data_domains/dp1.domain.raw.md` - Raw/Input domain for source frame data.
- `data_domains/dp1.domain.processing.md` - Processing domain for frame-derived computational payloads.
- `data_domains/dp1.domain.mask.md` - Mask domain for binary/label semantics.
- `data_domains/dp1.domain.struct.md` - Struct domain for candidates, segments, and intermediate object-like structures.
- `data_domains/dp1.domain.measurement.md` - Measurement domain for final DP1 product output and DP1 -> DP2 handoff.
- `data_domains/dp1.domain.visualization.md` - Visualization domain for display/debug rendering outputs.
- `data_domains/dp1.domain.runtime.md` - Runtime/context/tile execution support domain.
- `data_domains/dp1.domain.pixel_format.md` - Shared pixel-format, input bit-depth, range-policy, and route metadata vocabulary used by the domains.
- `data_domains/dp1.domain.common_types.md` - Shared status, route, geometry, profiling, diagnostics, and parameter vocabulary.

## Current domain policies

- `data_domains/dp1.domain.identity.md` - canonical identity tuple and compact local-id policy.
- `data_domains/dp1.domain.time.md` - timestamp roles and explicit clock semantics.
- `data_domains/dp1.domain.memory_ownership.md` - image/view ownership and tile-worker memory policy.
- `data_domains/dp1.domain.threshold.md` - threshold units and range semantics for code generation.
- `data_domains/dp1.domain.quality_flags.md` - compact quality/reject flag registry.
- `data_domains/dp1.domain.coordinates.md` - tile-local/frame-global coordinate policy and merge boundary.
- `data_domains/dp1.domain.opencv_invariants.md` - OpenCV `cv::Mat`, ROI, continuity, type, and rectangle invariants.
- `data_domains/dp1.domain.profiling.md` - canonical profiling levels, stage/operation timing, cardinality, memory metrics, bounded traces, and summaries.

## Current domain structures

- `data_domains/structures/raw/dp1.domain.raw.frame_packet.md` - `FramePacket` structure in Raw/Input domain.
- `data_domains/structures/raw/dp1.domain.raw.tile_raw_view.md` - `TileRawView` read-only ROI view in Raw/Input domain.
- `data_domains/structures/processing/dp1.domain.processing.frame.md` - `ProcessingFrame` structure in Processing domain.
- `data_domains/structures/processing/dp1.domain.processing.tile_processing_frame.md` - `TileProcessingFrame` tile-local processing payload in Processing domain.
- `data_domains/structures/mask/dp1.domain.mask.binary_mask.md` - `BinaryMask` structure in Mask domain.
- `data_domains/structures/mask/dp1.domain.mask.tile_binary_mask.md` - `TileBinaryMask` tile-local binary mask payload in Mask domain.
- `data_domains/structures/struct/dp1.domain.struct.candidate.md` - `Candidate` structure in Struct domain.
- `data_domains/structures/struct/dp1.domain.struct.segment.md` - `Segment` structure in Struct domain.
- `data_domains/structures/struct/dp1.domain.struct.validated_object.md` - `ValidatedObject` structure emitted by object filtering in Struct domain.
- `data_domains/structures/measurement/dp1.domain.measurement.record.md` - `MeasurementRecord` structure in Measurement domain.

## Current runtime / tile-local execution structures

These cards describe target execution-support structures for AI-coder context.
They do not implement parallelism and do not claim current runtime support.

- `data_domains/structures/runtime/dp1.domain.runtime.frame_context.md` - `FrameContext` for per-frame runtime/config/profiling context.
- `data_domains/structures/runtime/dp1.domain.runtime.cyclic_frame_buffer.md` - `CyclicFrameBuffer` for bounded reusable frame-history state owned by stateful stages.
- `data_domains/structures/runtime/dp1.domain.runtime.tile_desc.md` - `TileDesc` for ROI/tile + border/valid-area description.
- `data_domains/structures/runtime/dp1.domain.runtime.tile_context.md` - `TileContext` for per-worker reusable tile-local buffers and diagnostics.
- `data_domains/structures/runtime/dp1.domain.runtime.tile_result.md` - `TileResult` for tile-local outputs before merge.

## Prep Execution Variants

DP1 supports several `prep.variant` execution routes:

- `full_frame` — the whole frame is one processing unit.
- `roi` — one or more explicit frame-global ROI regions are processing units.
- `tiles` — the frame or selected ROI is split into `TileDesc[]` and processed
  through tile-local support structures.
- `adaptive_roi` — ROI regions are selected dynamically by a dedicated stage
  spec and fallback policy.

The tile execution route uses:

```text
FramePacket.image
  -> TileDesc[]
  -> TileRawView per tile
  -> TileContext per worker
  -> TileProcessingFrame / TileBinaryMask / Candidate / Segment / ValidatedObject / MeasurementRecord
  -> TileResult[]
  -> frame-level MeasurementRecord[]
```

Tile-local structures define the `prep.variant = "tiles"` route. They must not
be treated as the only canonical execution model. `full_frame`, `roi`, and
`adaptive_roi` require their own stage specs and memory/coordinate policies.

Structures are domain carriers. Algorithms are route-specific implementations.
`U8` and `U16` input routes must be represented through `PixelFormat`,
`InputBitDepth`, `PixelRange`, and `PipelineRoute` metadata instead of
structure names such as `raw16` or `proc32`.

## Current pipeline bindings

- `pipeline/dp1.pipeline.stage_domain_bindings.md` - cross-stage binding overview with links to the eight authoritative stage-interface binding sections, plus pipeline-level tile/merge and hidden-output rules.
- `pipeline/dp1.pipeline.stage_io_matrix.md` - canonical input/output matrix for the eight DP1 stages and tile merge boundary.

## Current configuration contracts

- `configuration/dp1.config.pipeline_configuration_c.md` - canonical DP1 pipeline configuration `C`.
- `configuration/dp1.config.application.md` - root canonical DP1 application-runtime configuration.
- `configuration/dp1.config.application.source.md` - canonical DP1 application frame source configuration.
- `configuration/dp1.config.application.logging.md` - canonical DP1 application logging configuration.
- `configuration/dp1.config.application.profiling.md` - canonical DP1 application profiling configuration.
- `configuration/dp1.config.application.dp2.md` - canonical DP1 -> DP2 runtime connection configuration.
- `configuration/dp1.config.stage_variant_registry.md` - registry of the eight fixed stages, allowed variants, and variant/level separation.
- `configuration/dp1.config.complexity_levels.md` - canonical complexity-level vocabulary.

## Current validation contracts

- `validation/dp1.validation.canonical_conformance.md` - DP1-local canonical conformance requirements.
- `validation/dp1.validation.stage_contract_checks.md` - stage contract checklist for future implementation review.

## Current stage specifications

- `stage_specs/dp1.stage_spec.radiometric_correction.inverse_median.md` -
  stage spec for the existing `radiometric_correction` variant
  `inverse_median`.
- `stage_specs/dp1.stage_spec.candidate_extraction.pilot.md` -
  pilot small-TZ stage spec for `candidate_extraction` variant
  `global_threshold` at `L0`.

The current canonical stage-interface card set covers only the eight main DP1
detection/measurement stages. Infrastructure stages such as visualization and
persistence are intentionally not described in this pass.

## OpenCV boundary

OpenCV provides low-level primitives and storage types. It does not define DP1 architecture. DP1 architecture is defined by data domains, stage contracts, pipeline composition, configuration `C`, and validation rules.
