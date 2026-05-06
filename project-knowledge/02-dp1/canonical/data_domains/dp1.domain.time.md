---
id: dp1.domain.time
title: "Canonical-політика часу DP1"
tags: [dp1, canonical, data-domain, time, timestamp, code-generation]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.time.md"
status: "draft"
---

## Definition

Ця картка визначає canonical policy timestamp semantics для DP1 domain
structures і DP1 -> DP2 handoff.

## Assumptions

- Runtime implementation може використовувати `steady_clock` для profiling і
  latency.
- Payload або persisted records мають мати explicit clock semantics, а не
  неявний C++ clock type.
- Camera acquisition time може бути відсутнім.

## Theorem / Contract

Canonical timestamp reference має явно задавати clock semantics:

```cpp
enum class ClockType {
    SteadyRuntime,
    SystemUtc,
    CameraHardware,
    ExternalSync
};

struct TimestampRef {
    ClockType clock_type = ClockType::SteadyRuntime;
    std::int64_t value_ns = 0;
};
```

Canonical roles:

- `acquisition_time` — час експозиції/source frame, якщо доступний.
- `ingest_time` — час прийому кадру DP1 runtime.
- `processing_time` або `time_ref` — час формування result.
- profiling events можуть мати runtime-local monotonic timestamps.

`MeasurementRecord` має бути sufficient для downstream temporal interpretation
без припущення, що DP2 знає internal C++ clock type DP1.

## Interpretation

Internal DTO може тимчасово зберігати `std::chrono::steady_clock::time_point`,
але domain/protocol contract має бути expressible через `TimestampRef`.

## Failure cases

- DP2 отримує timestamp без clock type.
- Acquisition time і processing time змішуються в одному полі без role metadata.
- Wall-clock time використовується для latency/profiling без monotonic source.

## Typical misuse

- Називати будь-який integer timestamp canonical time.
- Вважати `steady_clock` portable payload representation.

## Open questions

- Exact mapping між runtime clock і serialized payload.
- Чи потрібен frame-level clock synchronization status.

## Connections

- constrains: dp1.domain.raw.frame_packet
- constrains: dp1.domain.runtime.frame_context
- constrains: dp1.domain.measurement.record
- informs: protocols.dp1_dp2.measurement_handoff
