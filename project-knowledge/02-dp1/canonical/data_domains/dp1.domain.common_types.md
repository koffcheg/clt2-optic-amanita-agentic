---
id: dp1.domain.common_types
title:
  uk: "Спільні canonical-типи DP1"
  en: "DP1 common canonical types"
tags: [dp1, canonical, data-domain, common-types, status, profiling, code-generation]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.common_types.md"
status: "draft"
---

## Definition

Ця картка визначає спільний vocabulary типів, які використовуються кількома
canonical DP1 cards і мають бути відомі AI-кодеру до генерації DTO або stage
interfaces.

## Assumptions

- Це knowledge-level contract, а не готовий C++ header.
- Future implementation task може змінити exact storage layout, але не має
  змінювати semantics без оновлення цієї картки.
- Типи нижче потрібні для замкнутості canonical model, навіть якщо частина з
  них буде реалізована як enum, struct або alias.

## Theorem / Contract

Спільні status/error типи:

```cpp
enum class StageStatus : std::uint8_t {
    Ok,
    Skipped,
    EmptyInput,
    InvalidInputFormat,
    InvalidGeometry,
    ConfigError,
    RuntimeError
};

enum class CandidateStatus : std::uint8_t {
    Provisional,
    RejectedByArea,
    RejectedByThreshold,
    RejectedByGeometry,
    AcceptedForSegmentation
};
```

Спільні route/geometry типи:

```cpp
enum class BorderPolicy : std::uint8_t {
    None,
    Replicate,
    Reflect101,
    ConstantZero
};

enum class MaskForegroundConvention : std::uint8_t {
    ZeroBackground255Foreground,
    ZeroBackgroundOneForeground
};
```

`FrameGeometry` canonical semantics:

```cpp
struct FrameGeometry {
    int width = 0;
    int height = 0;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;
    cv::Point origin_px{0, 0};
};
```

Profiling vocabulary визначає окрема canonical-картка
`dp1.domain.profiling`. `common_types` не має дублювати profiling DTO, бо
profiling contract включає time semantics, bounded trace, aggregation,
cardinality metrics, memory metrics і stage identity extension policy.

```cpp
struct DiagnosticMessage {
    StageStatus status = StageStatus::Ok;
    std::string stage_name;
    std::string message_code;
    std::string detail;
};
```

Структури profiling `StageKey`, `StageTiming`, `OperationTiming`,
`ProfileEvent`, `ProfilingTrace`, `CardinalityMetrics`, `MemoryMetrics`,
`FrameProfiling`, `TileProfiling`, `TileProfilingResult` і
`StageProfileSummary` визначає
`dp1.domain.profiling`.

Configuration parameter vocabulary:

```cpp
using ParameterMap = /* structured key-value map with typed values */;
```

`ParameterMap` не має бути free-form string bag у future implementation.
Threshold-like values мають використовувати `ThresholdConfig`, а enum-like
values мають проходити validation against registry.

## Fields / Interface

```yaml
type_groups:
  - group: "Status"
    types: ["StageStatus", "CandidateStatus"]
    used_by: ["FrameContext", "Candidate", "validation checks"]
  - group: "Route"
    types: ["BorderPolicy", "MaskForegroundConvention"]
    used_by: ["TileDesc", "BinaryMask", "TileBinaryMask"]
  - group: "Geometry"
    types: ["FrameGeometry"]
    used_by: ["FramePacket", "ProcessingFrame", "BinaryMask", "MeasurementRecord"]
  - group: "Profiling"
    types: ["defined_by: dp1.domain.profiling"]
    used_by: ["FrameContext", "TileContext", "TileResult", "stage contract checks"]
  - group: "Diagnostics"
    types: ["DiagnosticMessage"]
    used_by: ["FrameContext", "TileContext", "TileResult"]
  - group: "Configuration"
    types: ["ParameterMap"]
    used_by: ["PipelineConfig", "StageConfig"]
```

## Input / Output

Input:

- canonical data-domain structure cards;
- stage-interface cards;
- configuration cards.

Output:

- shared type vocabulary for future DTO generation;
- constraint source for validation and code-generation tasks.

## Constraints

- `StageStatus` має бути bounded enum, не free-form text.
- `BorderPolicy` має бути explicit для tile/filter/morphology routes.
- `CandidateStatus` не замінює `quality_flags`.
- `DiagnosticMessage.detail` не має бути єдиним machine-readable source.
- Profiling DTO мають братися з `dp1.domain.profiling`, а не локально
  вигадуватися у runtime або stage cards.

## Failure cases

- `BorderPolicy` використовується у `TileDesc`, але не має canonical values.
- Runtime card посилається на profiling structure, якої немає у
  `dp1.domain.profiling`.
- `ParameterMap` використовується для threshold без units.
- AI-кодер створює різні enum values у різних modules.

## Typical misuse

- Зберігати candidate reject reason тільки як string.
- Використовувати `StageStatus::Ok` для skipped stage без окремого статусу.
- Змішувати timing event і diagnostic message в одному untyped vector.

## Open questions

- Exact typed-value schema для `ParameterMap`.
- Error taxonomy і message_code registry.

## Connections

- used_by: dp1.config.pipeline_configuration_c
- used_by: dp1.domain.runtime.frame_context
- used_by: dp1.domain.runtime.tile_context
- used_by: dp1.domain.runtime.tile_desc
- used_by: dp1.domain.runtime.tile_result
- used_by: dp1.domain.struct.candidate
- used_by: dp1.validation.stage_contract_checks
- separates_profiling_to: dp1.domain.profiling
