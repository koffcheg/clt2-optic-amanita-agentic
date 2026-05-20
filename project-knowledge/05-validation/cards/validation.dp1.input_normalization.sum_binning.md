---
id: validation.dp1.input_normalization.sum_binning
title:
  uk: "Unit validation route для Stage0 software sum binning"
  en: "Unit validation route for Stage0 software sum binning"
tags: [validation, dp1, canonical, input-normalization, binning, unit]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/05-validation/cards/validation.dp1.input_normalization.sum_binning.md"
status: "draft"
---

## Definition

Stage0.1 and Stage0.2-pre are implementation/task slice labels, not canonical stage names. The canonical stage remains Stage0 Input Normalization.

Ця картка визначає unit validation route для canonical Stage0 variant
`software_sum_binning`, описаного у
`dp1.stage_spec.input_normalization.sum_binning`.

Нова test implementation належить окремій approved task. Ця картка не створює
тести, fixtures, mocks або golden files.

## Scope

Цільовий unit under test:

- `InputNormalizationStage` для `variant = "software_sum_binning"`;
- config validation для `input_normalization.parameters.kbin`;
- формування binned `CanonicalFrame`;
- lossless accumulated carrier і metadata динамічного діапазону;
- без прихованого downcast, scaling або clipping;
- explicit failure, якщо downstream route не може спожити розширений accumulated carrier;
- semantics реєстрації binned artifact у `FrameContext`;
- stage-level timing semantics для `StageKey = "input_normalization"`.

Цільовий future test file:

- `tests/unit/dp1/t_dp1v2_input_normalization_sum_binning.cpp`

Цільова future build integration:

- розширити existing DP1 v2 unit-test target після explicit test approval;
- не додавати image fixtures; використовувати synthetic in-memory `cv::Mat` inputs.

## Unit Test Cases

Обовʼязкові future GoogleTest cases для algorithm coverage:

- `Stage0_WhenKbin1_EmitsPassthroughCanonicalFrame`
- `Stage0_WhenU8Kbin2_SumsIntoAccumU32SemanticCarrier`
- `Stage0_WhenU8Kbin4_SumsIntoAccumU32SemanticCarrier`
- `Stage0_WhenU16Kbin2_SumsIntoAccumU32CanonicalFrame`
- `Stage0_WhenU16Kbin4_SumsIntoAccumU32CanonicalFrame`
- `Stage0_WhenU8Kbin4UsesMaxValues_DoesNotClampToU8`
- `Stage0_WhenU16Kbin4UsesMaxValues_DoesNotClampToU16`
- `Stage0_WhenBinningSuccessful_DoesNotScaleOrAverageOutput`
- `Stage0_WhenBinningSuccessful_DoesNotHiddenDowncastOutput`
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
- `Pipeline_WhenDownstreamRejectsAccumulatedCarrier_FailsExplicitly`

## Algorithm Assertions

Тести мають перевіряти exact sums, а не лише output shape.

Для `U8, kbin = 2` використовувати small synthetic `4x4` matrix із non-uniform values:

```text
1   2   3   4
5   6   7   8
9   10  11  12
13  14  15  16
```

Очікуваний `2x2` output:

```text
14  22
46  54
```

Для `U8, kbin = 4` використовувати synthetic `4x4` matrix і перевірити, що single output pixel дорівнює сумі всіх 16 source pixels.

Для `U16, kbin = 2` використовувати values above `255`, щоб довести, що implementation не використовує випадково 8-bit accumulator.

Для `U8, kbin = 4` max-value coverage:

```text
input all pixels = 255
expected output = 4080
```

Для `U16, kbin = 4` max-value coverage:

```text
input all pixels = 65535
expected output = 1048560
```

Ці max-value cases доводять widening without clipping для supported L0 factor range.
Тести також мають перевіряти, що output values не діляться на `kbin * kbin`;
average pooling, resize, `INTER_AREA`, scaling і hidden compatibility conversion
є недійсними для `software_sum_binning`.

## Metadata Assertions

Для successful binned output тести мають перевіряти:

- output geometry дорівнює `source.width / kbin` і `source.height / kbin`;
- non-divisible source geometry завершується failure замість crop або resize;
- output `pixel_range` fields множаться на `kbin * kbin`;
- output range fields використовують `output_min`, `output_max`, `black_level` and
  `saturation_level` для accumulated range, а не source range;
- output carrier metadata декларує `AccumU32` semantics; transitional `S32` допустимий тільки як non-negative storage carrier з explicit metadata;
- `normalization.source = Stage0`;
- `normalization.copied = true`;
- `normalization.converted = false`;
- `normalization.binned = true`;
- `normalization.bin_factor_x = kbin`;
- `normalization.bin_factor_y = kbin`;
- `normalization.binning_mode = Sum`;
- `parent_artifact_id = "raw_frame"`;
- coordinate metadata достатня для mapping
  `x0 = xoff + kbin * xb`, `y0 = yoff + kbin * yb`;
- Stage0.2 full-frame baseline використовує `xoff = 0`, `yoff = 0`.

## Artifact And Timing Assertions

Для successful binned output тести мають перевіряти, що `FrameContext` містить
`canonical_frame` artifact with:

- `kind = CanonicalFrame`;
- `domain = Raw`;
- `producer_stage = input_normalization`;
- `parent_artifact_id = raw_frame`;
- `ownership = OwnedByStageOutput`;
- `lifetime = StageOutputScope`;
- `status = Available`.

Assertions для downstream compatibility:

- route validation повертає explicit failure, якщо Radiometric/downstream не підтримує
  розширений accumulated carrier;
- failure reason відрізняє unsupported widened carrier від algorithm error;
- fallback до `U8`, `U16`, scaled, clipped або averaged output не приймається.

Assertions для StageTiming:

- записано один stage timing `input_normalization`;
- status дорівнює `Completed` для valid binning;
- status і reason explicit для invalid input/config;
- variant дорівнює `software_sum_binning`;
- level дорівнює `L0`;
- input і output formats відображають widening.

## Test Data

Використовувати deterministic in-memory matrices, створені всередині кожного test.

Для цих unit tests не потрібні file input, camera input, IPC, DP2 runtime, run directories, large datasets, snapshots, golden files або external resources.

## Non-goals

- Тестування average binning.
- Тестування max pooling.
- Тестування resize або `INTER_AREA`.
- Тестування hardware binning.
- Тестування ROI або tiles.
- Тестування `Prep`.
- Реалізація тестів для lossy compatibility variant до окремого approval цього variant.
- Тестування DP2 handoff implementation.
- Тестування CameraProSim runtime.
- Тестування OverlayRunner.
- Створення fixtures або golden files.

## Connections

- validates: dp1.stage_spec.input_normalization.sum_binning
- validates: dp1.stage.input_normalization
- validates: dp1.domain.raw.canonical_frame
- validates: dp1.domain.memory_ownership
- validates: dp1.domain.coordinates
- validates: dp1.config.pipeline_configuration_c
- validates: dp1.config.stage_variant_registry
- follows: project-knowledge/05-validation/UNIT_TESTING_GUIDE.md
