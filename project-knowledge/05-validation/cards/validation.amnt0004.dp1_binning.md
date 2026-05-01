---
id: validation.amnt0004.dp1_binning
title:
  uk: "Валідація AMNT-0004: sum-binning у DP1"
  en: "Validation AMNT-0004: DP1 sum-binning"
tags: [validation, dp1, binning, manual-test, performance]
kind: validation-card
source_role: verification
source:
  file: "datapro1/src/dp1_frame_proc.cpp"
  lines: "1-420"
status: "draft"
---

## Definition

Ця картка описує ручний сценарій валідації для DP1-функції `binning`: порівняти режими OFF/ON на одному й тому самому dataset, оцінити продуктивність і перевірити guardrails конфігурації.

## Assumptions

- Валідація виконується у debug build через `builder/build_dp1_dp2.sh Debug`.
- Поточний сценарій використовує одноканальний dataset `SWIR_Camera`.
- DP2 може бути зупинений під час валідації виходу DP1.

## Theorem / Contract

- `binning.switched=false` має зберігати legacy-шлях без додаткового preprocessing.
- `binning.switched=true, factor=2, mode=sum` має запускати preprocessing перед tiling і перед export повертати координати вимірювань у початкову систему координат кадру.
- Некоректні значення `binning.factor` і `binning.mode` мають зупиняти виконання з помилкою валідації конфігурації.

## Interpretation

Ця валідація є evidence для `AMNT-0004` і прикладом базового validation pattern для нових алгоритмічних змін, керованих feature toggle.

## Failure cases

- Непідтримуваний `factor`, наприклад `3`.
- Непідтримуваний `mode`, наприклад `avg`.
- Непридатний dataset, наприклад multi-channel data без відповідної підтримки preprocessing.

## Typical misuse

- Порівнювати режими OFF/ON на різних datasets або з різними segmentation configs.
- Робити висновки про якість detection лише за FPS, без перевірки `Size` і coordinate metrics.

## Open questions

- TODO: confirm with user whether segmentation parameters should be normalized separately for default binning mode.
- TODO: confirm with user whether average/max binning should become separate future modes.

## Connections

- used_by: AMNT-0004
- overlaps_with: dp1.config.prg_config
- overlaps_with: dp1.frame.frame_processor
- produces: AMNT-0004 external test report (`/mnt/D/AmanitaResources/AMNT-0004_binning_test_report_2026-03-22_uk.txt`)
