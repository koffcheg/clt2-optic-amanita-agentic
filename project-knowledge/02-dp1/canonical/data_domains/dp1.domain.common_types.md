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

Profiling і diagnostics vocabulary:

```cpp
struct StageTiming {
    std::string stage_name;
    std::string variant;
    std::string level;
    double elapsed_ms = 0.0;
};

struct StageProfile {
    std::string stage_name;
    std::string variant;
    std::string level;
    std::size_t input_bytes = 0;
    std::size_t output_bytes = 0;
    std::size_t conversion_bytes = 0;
};

struct ProfileEvent {
    StageTiming timing;
    StageProfile profile;
};

struct DiagnosticMessage {
    StageStatus status = StageStatus::Ok;
    std::string stage_name;
    std::string message_code;
    std::string detail;
};

struct TileProfilingSummary {
    int tile_id = -1;
    std::vector<StageTiming> stage_timings;
    std::size_t peak_tile_bytes = 0;
};
```

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
    types: ["StageTiming", "StageProfile", "ProfileEvent", "TileProfilingSummary"]
    used_by: ["FrameContext", "TileContext", "TileResult"]
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
- `StageTiming` і `StageProfile` мають розрізняти elapsed time і memory/conversion
  accounting.

## Failure cases

- `BorderPolicy` використовується у `TileDesc`, але не має canonical values.
- `ProfileEvent` існує як поле у `FrameContext`, але не має semantics.
- `ParameterMap` використовується для threshold без units.
- AI-кодер створює різні enum values у різних modules.

## Typical misuse

- Зберігати candidate reject reason тільки як string.
- Використовувати `StageStatus::Ok` для skipped stage без окремого статусу.
- Змішувати timing event і diagnostic message в одному untyped vector.

## Open questions

- Exact typed-value schema для `ParameterMap`.
- Error taxonomy і message_code registry.
- Чи потрібно винести profiling types в окрему `dp1.domain.profiling` card,
  якщо обсяг profiling contract зросте.

## Connections

- used_by: dp1.config.pipeline_configuration_c
- used_by: dp1.domain.runtime.frame_context
- used_by: dp1.domain.runtime.tile_context
- used_by: dp1.domain.runtime.tile_desc
- used_by: dp1.domain.runtime.tile_result
- used_by: dp1.domain.struct.candidate
- used_by: dp1.validation.stage_contract_checks
