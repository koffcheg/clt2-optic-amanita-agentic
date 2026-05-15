---
id: validation.dp1.config.canonical
title:
  uk: "Unit validation route for canonical DP1 configuration"
  en: "Unit validation route for canonical DP1 configuration"
tags: [validation, dp1, canonical, config, unit]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/05-validation/cards/validation.dp1.config.canonical.md"
status: "draft"
---

## Definition

This card defines the future unit-test plan for canonical `dp1_v2`
configuration parsing and validation.

It does not create automated tests. Test implementation belongs to a separate
approved task.

## Scope

Target unit under test:

- canonical JSON parsers for separate `ApplicationConfig` and `PipelineConfig` files;
- canonical `ApplicationConfig` validator;
- canonical `PipelineConfig` validator;
- stage-key, variant, level, input route, generic `parameters`, and resolved
  `inverse_median` parameter validation.

Current AMNT-0023 validation scope intentionally excludes `PrepRouteConfig` and
`RuntimeLimitsConfig`; they remain canonical `PipelineConfig C` items for a later
approved implementation slice.

Target future test file:

- `tests/unit/dp1/t_dp1v2_config.cpp`

Target future build integration:

- extend `dp1v2_unit_tests` in `tests/CMakeLists.txt` with config parser source
  and Jansson linkage, or extract config parsing into a small testable library
  target before adding tests.

## Unit Test Cases

Required future GoogleTest cases:

- `LoadDp1Config_WhenCanonicalApplicationAndPipelineFilesAreValid_ReturnsTypedDp1Config`
- `LoadApplicationConfig_WhenPipelineWrapperExists_ThrowsConfigError`
- `LoadPipelineConfig_WhenApplicationWrapperExists_ThrowsConfigError`
- `LoadDp1Config_WhenApplicationSchemaVersionMissing_ThrowsConfigError`
- `LoadDp1Config_WhenPipelineSchemaVersionUnsupported_ThrowsConfigError`
- `LoadDp1Config_WhenLegacyRootConfigExists_ThrowsConfigError`
- `LoadDp1Config_WhenLegacyDp2ConnExists_ThrowsConfigError`
- `LoadDp1Config_WhenSourceModeIsFileAndPathExistsInConfig_ReturnsFileSourceConfig`
- `LoadDp1Config_WhenSourceModeIsCameraPro_ThrowsForFirstIteration`
- `LoadDp1Config_WhenSourceFilePathMissing_ThrowsConfigError`
- `LoadDp1Config_WhenDp2PortIsZero_ThrowsConfigError`
- `LoadDp1Config_WhenIntegerFieldExceedsIntRange_ThrowsConfigError`
- `LoadDp1Config_WhenDp2EnabledWithDisabledMode_ThrowsConfigError`
- `LoadDp1Config_WhenInputRouteIsU16Bit12WithOrderedRange_ReturnsBit12Route`
- `LoadDp1Config_WhenBitDepthIsString_ThrowsConfigError`
- `LoadDp1Config_WhenInputRouteIsU8Bit12_ThrowsConfigError`
- `LoadDp1Config_WhenPixelRangeIsNotOrdered_ThrowsConfigError`
- `LoadDp1Config_WhenRequiredStageKeyIsMissing_ThrowsConfigError`
- `LoadDp1Config_WhenFullStageInterfaceNameIsUsedAsConfigKey_ThrowsConfigError`
- `LoadDp1Config_WhenPipelineConfigCStageKeyRadiometricIsUsed_ReturnsStageConfig`
- `LoadDp1Config_WhenStageVariantIsNotRegistered_ThrowsConfigError`
- `LoadDp1Config_WhenStageLevelIsInvalid_ThrowsConfigError`
- `LoadDp1Config_WhenInverseMedianParametersObjectMissing_ThrowsConfigError`
- `LoadDp1Config_WhenInverseMedianOptionalFieldsAreOmitted_AppliesCanonicalDefaults`
- `LoadDp1Config_WhenInverseMedianStrideIsZero_ThrowsConfigError`
- `LoadDp1Config_WhenParameterIntegerExceedsIntRange_ThrowsConfigError`
- `LoadDp1Config_WhenInverseMedianUnknownKeyExists_ThrowsConfigError`
- `LoadDp1Config_WhenLoggingConfigFileMissing_ThrowsConfigError`
- `LoadDp1Config_WhenLoggingConfigFileEmpty_ThrowsConfigError`
- `LoadDp1Config_WhenProfilingLevelIsDuplicated_ThrowsConfigError`
- `LoadDp1Config_WhenExternalTraceIsEnabled_ThrowsConfigError`
- `LoadDp1Config_WhenLoggingAsyncBlocksRtSafeProfile_ThrowsConfigError`

## Test Data

Use paired small temporary JSON files generated inside each test or helper
functions that write to per-test temporary paths. Application config tests should
write only an `ApplicationConfig` root; pipeline config tests should write only a
`PipelineConfig` root.

No image fixtures, camera input, IPC, network, DP2 runtime, or full pipeline
execution are required for these unit tests.

## Assertions

Each test should assert one of the following:

- the parsed raw `StageConfig.parameters` and resolved typed enum/value equal the canonical expected value;
- required defaults are applied exactly where the canonical config card allows
  defaults;
- invalid authoring shapes throw `std::logic_error`;
- legacy authoring shapes are rejected instead of silently mapped.

## Non-goals

- Testing DP1 algorithms.
- Testing file source frame reading.
- Testing DP1 -> DP2 transport.
- Testing `PrepRouteConfig` and `RuntimeLimitsConfig` parsing; these are deferred
  from AMNT-0023 by explicit task decision.
- Testing log4cxx XML loading.
- Testing external profiling integrations.
- Running Amanita + Comparator.

## Connections

- validates: dp1.config.application
- validates: dp1.config.pipeline_configuration_c
- validates: dp1.config.stage_variant_registry
- validates: dp1.config.complexity_levels
- validates: dp1.stage_spec.radiometric_correction.inverse_median
- follows: project-knowledge/05-validation/UNIT_TESTING_GUIDE.md
