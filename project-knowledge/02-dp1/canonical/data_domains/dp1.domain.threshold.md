---
id: dp1.domain.threshold
title: "Canonical-політика порогів DP1"
tags: [dp1, canonical, data-domain, threshold, range, code-generation]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.threshold.md"
status: "draft"
---

## Definition

Ця картка визначає canonical threshold/range policy для DP1 stages.

Threshold без одиниць і domain semantics не є достатнім code-generation input.

## Assumptions

- Різні stages працюють у різних numeric domains: raw pixels, signed residual,
  normalized float, detector response або noise-relative scale.
- Конкретні threshold values належать configuration `C` і stage specs.

## Theorem / Contract

Canonical threshold має містити units:

```cpp
enum class ThresholdUnits {
    RawUnits,
    Normalized01,
    SignedResidualUnits,
    DetectorResponseUnits,
    SigmaNoiseUnits
};

struct ThresholdConfig {
    double value = 0.0;
    ThresholdUnits units = ThresholdUnits::DetectorResponseUnits;
};
```

Правила:

- `RawUnits` прив'язані до `PixelRange` і `InputBitDepth`.
- `Normalized01` дозволений тільки якщо conversion/range policy явно задає
  normalization.
- `SignedResidualUnits` використовується для residual routes із `S16` або
  `S32`.
- `DetectorResponseUnits` використовується для matched-filter або response-map
  stages.
- `SigmaNoiseUnits` потребує explicit noise/statistics source.

## Interpretation

Ця policy дозволяє одному stage route працювати з `U8`, `U16` і `F32` без
прихованих magic constants.

## Failure cases

- Stage spec або config задає `threshold: 42` без units.
- `U16` route використовує threshold, підібраний для `U8`, без conversion.
- Signed residual threshold трактується як unsigned intensity.

## Typical misuse

- Виводити threshold scale тільки з `cv::Mat::type()`.
- Зберігати threshold у stage code замість `C`.

## Open questions

- Default threshold units для кожного stage variant.
- Чи потрібен окремий hysteresis threshold contract.

## Connections

- constrains: dp1.domain.pixel_format
- constrains: dp1.config.pipeline_configuration_c
- constrains: dp1.stage.candidate_extraction
- constrains: dp1.stage.object_filtering
