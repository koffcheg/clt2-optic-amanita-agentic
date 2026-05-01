---
id: dp1.preproc.binning_sum
title:
  uk: "sum-binning preprocessing у DP1"
  en: "DP1 sum-binning preprocessing"
tags: [dp1, preprocessing, binning, config, performance]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_frame_proc.cpp"
  lines: "17-420"
status: "draft"
---

## Definition

`sum-binning` у DP1 — це опційний preprocessing етап, який агрегує блоки пікселів розміру `factor x factor` перед основним pipeline тайлінгу/сегментації.

## Assumptions

- Поточна реалізація підтримує лише `mode = sum`.
- Підтримувані `factor`: `1`, `2`, `4`.
- Вхідний кадр має бути single-channel для binning path.
- Розміри кадра мають бути кратні `factor`.

## Theorem / Contract

- При `binning.switched=false` або `factor=1` preprocessing не змінює кадр (legacy behavior).
- При `binning.switched=true` і валідному конфізі застосовується sum-binning до виклику `datapro1(...)`.
- Після обробки координати/розміри вимірів масштабуються назад у СК початкового кадра до експорту в `.blob/.json` і до передачі в DP2.
- Невалідні `factor/mode` спричиняють явну помилку конфіга.

## Interpretation

Це performance-oriented feature toggle, який дозволяє перемикати алгоритмічний режим без зміни коду та порівнювати поведінку OFF/ON на однакових даних.

## Failure cases

- `factor` не з дозволеного набору.
- `mode` відмінний від `sum`.
- Некратні розміри кадра.
- Запуск із багатоканальним кадром у binning path.

## Typical misuse

- Вмикати binning і очікувати незмінні детекційні метрики без калібрування порогів сегментації.
- Порівнювати OFF/ON на різних джерелах або з різними `segment` параметрами.

## Open questions

- Потрібність підтримки average/max/pooling режимів.
- Політика автоматичного підбору `segment.level` для binning режиму.

## Connections

- used_by: dp1.frame.frame_processor
- configured_by: dp1.config.prg_config
- affects: dp1.types.TOptionsMeasurement
- validated_by: validation.amnt0004.dp1_binning
