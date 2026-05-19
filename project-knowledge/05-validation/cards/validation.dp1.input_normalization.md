---
id: validation.dp1.input_normalization
title:
  uk: "Unit validation route для Stage0.1 input normalization"
  en: "Unit validation route for Stage0.1 input normalization"
tags: [validation, dp1, canonical, input-normalization, canonical-frame, unit]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/05-validation/cards/validation.dp1.input_normalization.md"
status: "draft"
---

## Definition

Ця картка визначає unit validation route для Stage0.1 `input_normalization`
pass-through baseline. Вона описує фактичні GoogleTest перевірки у
`tests/unit/dp1/t_dp1v2_input_normalization_stage.cpp`.

Stage0.1 не виконує binning, conversion, scaling, clipping, ROI або tiles. Його
обов'язок у цьому slice: перевірити `FramePacket` проти
`InputNormalizationConfig.input_route` і сформувати explicit borrowed/read-only
`CanonicalFrame`.

## Scope

Target unit under test:

- `InputNormalizationStage`;
- `variant = "passthrough"`;
- `InputNormalizationConfig.input_route` validation;
- pass-through `CanonicalFrame` formation;
- `canonical_frame` artifact registration semantics in `FrameContext`;
- pass-through `NormalizationProvenance`.

Target test file:

- `tests/unit/dp1/t_dp1v2_input_normalization_stage.cpp`

Build integration:

- `datapro1_v2/CMakeLists.txt` includes the test in standalone
  `dp1v2_unit_tests`;
- `tests/CMakeLists.txt` includes the test in the root unit-test target when
  that target is configured.

## Unit Test Cases

Current GoogleTest cases:

- `Process_WhenU8RouteMatches_EmitsCanonicalFrame`
- `Process_WhenU16RouteMatches_EmitsCanonicalFrame`
- `Process_WhenPixelFormatMismatches_ReturnsUnsupported`
- `Process_WhenBitDepthMismatches_ReturnsUnsupported`
- `Process_WhenPixelRangeMismatches_ReturnsUnsupported`
- `Process_WhenCarrierDepthMismatches_ReturnsUnsupported`
- `Process_WhenSuccessful_CanonicalFrameCanBeRegisteredAsArtifact`
- `Process_WhenSuccessful_CanonicalFrameProvenanceIsPassthroughNoBinning`

## Success Assertions

For matching `U8` input route, the tests assert:

- `StageExecutionStatus::Completed`;
- output `CanonicalFrame.frame_id` matches the source `FramePacket`;
- output `pixel_format = U8`;
- output `bit_depth = Bit8`;
- output carrier type is `CV_8UC1`;
- output image data pointer is the same as source `FramePacket.image.data`;
- `image_ownership = BorrowedReadOnly`;
- `parent_artifact_id = "raw_frame"`.

For matching `U16` input route, the tests assert:

- `StageExecutionStatus::Completed`;
- output `pixel_format = U16`;
- output `bit_depth = Bit16`;
- output carrier type is `CV_16UC1`;
- output geometry matches source image dimensions.

## Failure Assertions

For route mismatches, tests assert explicit `StageExecutionStatus::Unsupported`
and stable reason strings:

- pixel format mismatch -> `input_route pixel_format mismatch`;
- bit depth mismatch -> `input_route bit_depth mismatch`;
- pixel range mismatch -> `input_route pixel_range mismatch`;
- carrier depth mismatch -> `input_route carrier depth mismatch`.

The carrier-depth test verifies that metadata alone is not enough: the actual
`cv::Mat` depth must match the expected carrier for the configured route.

## Artifact Assertions

For successful pass-through output, the artifact registration test asserts that
`register_canonical_frame_artifact(...)` produces:

- `id = "canonical_frame"`;
- `kind = CanonicalFrame`;
- `producer_stage = "input_normalization"`;
- `parent_artifact_id = "raw_frame"`;
- `ownership = BorrowedReadOnly`.

The artifact record reflects provenance and payload lifetime. It does not make
`FrameContext` the owner of the borrowed `cv::Mat` payload.

## Provenance Assertions

For successful pass-through output, tests assert:

- `normalization.source = Stage0`;
- `normalization.copied = false`;
- `normalization.converted = false`;
- `normalization.binned = false`;
- `normalization.bin_factor_x = 1`;
- `normalization.bin_factor_y = 1`;
- `normalization.binning_mode = None`.

These assertions guard against hidden conversion, hidden copy, hidden binning,
scaling or clipping in Stage0.1.

## Test Data

All inputs are deterministic synthetic in-memory `cv::Mat` objects created
inside the test file:

- small `CV_8UC1` matrices for U8 route coverage;
- small `CV_16UC1` matrices for U16 route coverage;
- intentionally mismatched metadata or carrier depth for failure cases.

No file input, camera input, IPC, DP2 runtime, run directories, large datasets,
fixtures, snapshots, golden files or external resources are required.

## Non-goals

- Testing `software_sum_binning`.
- Testing `kbin` validation.
- Testing widened carrier policy.
- Testing dynamic-range multiplication.
- Testing average binning, max pooling, resize or `INTER_AREA`.
- Testing ROI, tiles or `Prep`.
- Testing radiometric algorithm behavior.
- Testing SWIR/mono16 run-based validation.
- Testing DP2 handoff implementation.

## Validation Command

The current implementation was validated through the built GoogleTest binary:

```bash
cmake --build build-dp1v2 --target dp1v2_unit_tests
./build-dp1v2/dp1v2_unit_tests
```

`ctest --test-dir build-dp1v2 --output-on-failure -R "dp1v2"` found no
registered tests in that build directory, so the GoogleTest binary was run
directly.

## Connections

- validates: dp1.stage.input_normalization
- validates: dp1.domain.raw.canonical_frame
- validates: dp1.domain.memory_ownership
- validates: dp1.config.pipeline_configuration_c
- validates: dp1.config.stage_variant_registry
- follows: project-knowledge/05-validation/UNIT_TESTING_GUIDE.md
- complements: validation.dp1.input_normalization.sum_binning
