---
id: dp1.stage_spec.candidate_extraction.pilot
title:
  uk: "Пілотна мала специфікація candidate extraction"
  en: "Pilot small specification for candidate extraction"
tags: [dp1, canonical, stage-spec, small-tz, pilot]
kind: stage-spec-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stage_specs/dp1.stage_spec.candidate_extraction.pilot.md"
  lines: "1-170"
status: "draft"
---

## Definition

Pilot small-TZ for `ICandidateExtractionStage`.

The card narrows the stage-interface contract to an implementation-grade route
for canonical development.

## Scope

- Stage interface: `ICandidateExtractionStage`.
- Pipeline slot: `candidate_extraction`.
- Complexity level: `L0`.
- Variant: `global_threshold`.

## Inputs

- Detector response map from upstream stage.
- Allowed types: `CV_32FC1` (preferred) or `CV_8UC1`.
- Input must represent one-channel response in processing domain geometry.

## Outputs

- `CV_8UC1` binary candidate mask.
- Candidate hypothesis collection derived from connected components of mask.
- Hypotheses are provisional and must not be treated as validated objects.

## Preconditions

- Config fragment `candidate_extraction` exists.
- `enabled=true` for execution path.
- `variant=global_threshold` and `level=L0` for this pilot spec.
- Threshold parameter exists and is numeric.

## Deterministic flow

1. Validate config keys and variant-level pair.
2. Normalize input representation:
   - if `CV_8UC1`, convert to internal scalar range policy;
   - if `CV_32FC1`, use directly.
3. Apply global threshold to produce binary mask.
4. Run connected-component extraction for hypothesis generation.
5. Emit mask and hypothesis set with explicit "not validated" semantic.

## Configuration contract

Canonical `C` fragment:

```yaml
candidate_extraction:
  enabled: true
  variant: global_threshold
  level: L0
  parameters:
    threshold: <number>
    min_area: <int, optional>
```

Rules:
- Missing `enabled|variant|level|parameters.threshold` => configuration error.
- Unknown keys in `candidate_extraction.parameters` => warning + ignore.
- `threshold` range policy must be explicit in implementation config notes.

## Invariants

- Output mask is single-channel `CV_8UC1` with binary values.
- Candidate hypotheses are derived only from produced mask.
- Stage must not perform downstream validation/classification.
- Stateful background models are forbidden in this pilot variant.

## Failure cases

- Threshold too high -> near-empty mask and candidate loss.
- Threshold too low -> noisy mask and candidate flooding.
- Type mismatch -> contract violation and stage failure path.

## Non-goals

- Adaptive thresholding (`L1`) behavior definition.
- Stateful background extraction (`L2`) behavior definition.
- Final object validation and measurement.

## Validation hooks

- Per-frame candidate count.
- Mask fill ratio.
- Threshold value trace.
- Stage time for threshold + connected components.

## Source-of-truth and canonicalization safety

Target canonical architecture source:
- this card defines intended behavior for canonical implementation.

Observed runtime/build/config/protocol facts:
- must be verified against code/config/protocol artifacts before claiming
  current runtime behavior.

Safety rule:
- if code/runtime behavior diverges from this card, report mismatch,
  classify it as either "canonical gap" or "runtime deviation", and propose
  minimal reconciliation updates; do not silently collapse the distinction.

## Connections

- Stage interface card: `../stages/dp1.stage.candidate_extraction.md`
- Canonical index: `../DP1_CANONICAL_INDEX.md`
- Pipeline/config route: `../pipeline/dp1.pipeline.stage_contract.md`,
  `../configuration/dp1.config.pipeline_configuration_c.md`
