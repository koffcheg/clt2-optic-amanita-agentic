# DP1_CANONICAL_INDEX

## Purpose

Canonical entry point for DP1 as an engineering product.

This section defines target DP1 architecture independently from legacy `datapro1` code. Legacy materials may be used only when a task card explicitly allows them.

## Source of truth route

1. `product/dp1.canonical.product_definition.md`
2. `product/dp1.canonical.source_of_truth.md`
3. `pipeline/dp1.pipeline.formal_model.md`
4. `pipeline/dp1.pipeline.stage_contract.md`
5. `data_domains/*.md`
6. `stages/*.md`
7. `stage_specs/*.md`
8. `configuration/dp1.config.pipeline_configuration_c.md`
9. `configuration/dp1.config.complexity_levels.md`
10. `validation/dp1.validation.canonical_conformance.md`
11. `../../04-protocols/cards/protocols.dp1_dp2.measurement_handoff.md`

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

## Current data domains

- `data_domains/dp1.domain.raw.md` - Raw/Input domain for source frame data.
- `data_domains/dp1.domain.processing.md` - Processing domain for frame-derived computational payloads.
- `data_domains/dp1.domain.mask.md` - Mask domain for binary/label semantics.
- `data_domains/dp1.domain.struct.md` - Struct domain for candidates, segments, and intermediate object-like structures.
- `data_domains/dp1.domain.measurement.md` - Measurement domain for final DP1 product output and DP1 -> DP2 handoff.
- `data_domains/dp1.domain.visualization.md` - Visualization domain for display/debug rendering outputs.
- `data_domains/dp1.domain.pixel_format.md` - Shared U8/U16/F32/MaskU8 pixel-format vocabulary used by the domains.

## Current domain structures

- `data_domains/dp1.domain.raw.frame_packet.md` - `FramePacket` structure in Raw/Input domain.
- `data_domains/dp1.domain.processing.frame.md` - `ProcessingFrame` structure in Processing domain.
- `data_domains/dp1.domain.mask.binary_mask.md` - `BinaryMask` structure in Mask domain.
- `data_domains/dp1.domain.struct.candidate.md` - `Candidate` structure in Struct domain.
- `data_domains/dp1.domain.struct.segment.md` - `Segment` structure in Struct domain.
- `data_domains/dp1.domain.measurement.record.md` - `MeasurementRecord` structure in Measurement domain.

## Current runtime / tile-local execution structures

These cards describe target execution-support structures for AI-coder context.
They do not implement parallelism and do not claim current runtime support.

- `data_domains/dp1.domain.runtime.frame_context.md` - `FrameContext` for per-frame runtime/config/profiling context.
- `data_domains/dp1.domain.runtime.tile_desc.md` - `TileDesc` for ROI/tile + border/valid-area description.
- `data_domains/dp1.domain.runtime.tile_context.md` - `TileContext` for per-tile/per-worker buffers and diagnostics.
- `data_domains/dp1.domain.runtime.tile_result.md` - `TileResult` for tile-local outputs before merge.

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
