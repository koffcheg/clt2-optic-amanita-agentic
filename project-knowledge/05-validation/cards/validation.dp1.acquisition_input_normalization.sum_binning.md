---
id: validation.dp1.acquisition_input_normalization.sum_binning
title:
  uk: "Unit validation route для Stage0 software sum binning"
  en: "Unit validation route for Stage0 software sum binning"
tags: [validation, dp1, canonical, acquisition, input-normalization, binning, unit]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/05-validation/cards/validation.dp1.acquisition_input_normalization.sum_binning.md"
status: "draft"
---

## Definition

Ця картка визначає unit validation route для canonical Stage0 variant
`software_sum_binning`, описаного у
`dp1.stage_spec.acquisition_input_normalization.sum_binning`.

Нова test implementation належить окремій approved task. Ця картка не створює
тести, fixtures, mocks або golden files.

## Scope

Target unit under test:

- `AcquisitionInputNormalizationStage` для `variant = "software_sum_binning"`;
- config validation для `acquisition.parameters.kbin`;
- formation of binned `CanonicalFrame`;
- binned artifact registration semantics in `FrameContext`;
- stage-level timing semantics for `StageKey = "acquisition"`.

Target future test file:

- `tests/unit/dp1/t_dp1v2_acquisition_input_normalization_sum_binning.cpp`

Target future build integration:

- extend existing DP1 v2 unit-test target after explicit test approval;
- do not add image fixtures; use synthetic in-memory `cv::Mat` inputs.

## Unit Test Cases

Required future GoogleTest cases for algorithm coverage:

- `Stage0_WhenKbin1_EmitsPassthroughCanonicalFrame`
- `Stage0_WhenU8Kbin2_SumsIntoU16CanonicalFrame`
- `Stage0_WhenU8Kbin4_SumsIntoU16CanonicalFrame`
- `Stage0_WhenU16Kbin2_SumsIntoU32CanonicalFrame`
- `Stage0_WhenU16Kbin4_SumsIntoU32CanonicalFrame`
- `Stage0_WhenU8Kbin4UsesMaxValues_DoesNotClampToU8`
- `Stage0_WhenU16Kbin4UsesMaxValues_DoesNotClampToU16`
- `Stage0_WhenInputDimensionsNotDivisibleByKbin_ReturnsFailure`
- `Stage0_WhenInputHasMultipleChannels_ReturnsFailure`
- `Stage0_WhenUnsupportedKbin_ReturnsFailure`
- `Stage0_WhenKbinMissingForSoftwareSumBinning_ReturnsConfigFailure`
- `Stage0_WhenNonSumBinningModeRequested_ReturnsFailure`
- `Stage0_WhenBinningSuccessful_UpdatesCanonicalFrameGeometry`
- `Stage0_WhenBinningSuccessful_UpdatesPixelRangeByKbinSquared`
- `Stage0_WhenBinningSuccessful_SetsNormalizationProvenance`
- `Stage0_WhenBinningSuccessful_RegistersOwnedBinnedCanonicalFrameArtifact`
- `Stage0_WhenBinningSuccessful_RecordsStageTiming`
- `Stage0_WhenInputRouteMismatch_ReturnsFailureWithoutBinning`

## Algorithm Assertions

The tests must verify exact sums, not only output shape.

For `U8, kbin = 2`, use a small synthetic `4x4` matrix with non-uniform values:

```text
1   2   3   4
5   6   7   8
9   10  11  12
13  14  15  16
```

Expected `2x2` output:

```text
14  22
46  54
```

For `U8, kbin = 4`, use a synthetic `4x4` matrix and assert the single output
pixel equals the sum of all 16 source pixels.

For `U16, kbin = 2`, use values above `255` to prove that the implementation is
not accidentally using an 8-bit accumulator.

For `U8, kbin = 4` max-value coverage:

```text
input all pixels = 255
expected output = 4080
```

For `U16, kbin = 4` max-value coverage:

```text
input all pixels = 65535
expected output = 1048560
```

These max-value cases prove widening without clipping for the supported L0
factor range.

## Metadata Assertions

For successful binned output, tests must assert:

- output geometry is `source.width / kbin` and `source.height / kbin`;
- non-divisible source geometry fails instead of cropping or resizing;
- output `pixel_range` fields are multiplied by `kbin * kbin`;
- `normalization.source = Stage0`;
- `normalization.copied = true`;
- `normalization.converted = false`;
- `normalization.binned = true`;
- `normalization.bin_factor_x = kbin`;
- `normalization.bin_factor_y = kbin`;
- `normalization.binning_mode = Sum`;
- `parent_artifact_id = "raw_frame"`;
- coordinate metadata is sufficient to map
  `x0 = xoff + kbin * xb`, `y0 = yoff + kbin * yb`;
- Stage0.2 full-frame baseline uses `xoff = 0`, `yoff = 0`.

## Artifact And Timing Assertions

For successful binned output, tests must assert `FrameContext` contains a
`canonical_frame` artifact with:

- `kind = CanonicalFrame`;
- `domain = Raw`;
- `producer_stage = acquisition`;
- `parent_artifact_id = raw_frame`;
- `ownership = OwnedByStageOutput`;
- `lifetime = StageOutputScope`;
- `status = Available`.

Stage timing assertions:

- one `acquisition` stage timing is recorded;
- status is `Completed` for valid binning;
- status and reason are explicit for invalid input/config;
- variant is `software_sum_binning`;
- level is `L0`;
- input and output formats reflect widening.

## Test Data

Use deterministic in-memory matrices constructed inside each test.

No file input, camera input, IPC, DP2 runtime, run directories,
large datasets, snapshots, golden files, or external resources are required for
these unit tests.

## Non-goals

- Testing average binning.
- Testing max pooling.
- Testing resize or `INTER_AREA`.
- Testing hardware binning.
- Testing ROI or tiles.
- Testing `Prep`.
- Testing DP2 handoff implementation.
- Testing CameraProSim runtime.
- Testing OverlayRunner.
- Creating fixtures or golden files.

## Connections

- validates: dp1.stage_spec.acquisition_input_normalization.sum_binning
- validates: dp1.stage.acquisition_input_normalization
- validates: dp1.domain.raw.canonical_frame
- validates: dp1.domain.memory_ownership
- validates: dp1.domain.coordinates
- validates: dp1.config.pipeline_configuration_c
- validates: dp1.config.stage_variant_registry
- follows: project-knowledge/05-validation/UNIT_TESTING_GUIDE.md
