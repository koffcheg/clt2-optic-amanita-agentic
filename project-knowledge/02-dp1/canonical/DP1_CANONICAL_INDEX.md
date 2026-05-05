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

Data-domain cards define the canonical boundary objects and semantic carriers
that move through stages. Algorithms must reference these cards instead of
inventing local frame, mask, candidate, segment, or measurement structures.

## Current data domains

- `data_domains/dp1.domain.raw.md` - raw sensor/source data domain.
- `data_domains/dp1.domain.frame_packet.md` - frame payload boundary object.
- `data_domains/dp1.domain.frame_context.md` - per-frame runtime/pipeline context object.
- `data_domains/dp1.domain.pixel_format.md` - U8/U16/F32/MaskU8 pixel format vocabulary.
- `data_domains/dp1.domain.processing.md` - processing-domain frame-derived payload.
- `data_domains/dp1.domain.mask.md` - binary/label mask domain.
- `data_domains/dp1.domain.candidate.md` - provisional candidate hypothesis domain.
- `data_domains/dp1.domain.segment.md` - segmentation/refined-region domain.
- `data_domains/dp1.domain.measurement.md` - final DP1 measurement output domain.

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
