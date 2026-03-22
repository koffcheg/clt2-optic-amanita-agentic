---
id: validation.amnt0004.dp1_binning
title:
  uk: "Валідація AMNT-0004: sum-binning у DP1"
  en: "Validation AMNT-0004: DP1 sum-binning"
tags: [validation, dp1, binning, manual-test, performance]
source:
  file: "datapro1/src/dp1_frame_proc.cpp"
  lines: "1-420"
status: "draft"
---

## Definition

Картка описує фактичний manual validation сценарій для фічі `binning` у DP1: порівняння режимів OFF/ON на однаковому датасеті, оцінка продуктивності та перевірка guardrails конфіга.

## Assumptions

- Валідація проводиться в debug build через `builder/build_dp1_dp2.sh Debug`.
- Для поточної ітерації використовується single-channel датасет `SWIR_Camera`.
- DP2 може бути не запущений під час валідації DP1 output.

## Theorem / Contract

- `binning.switched=false` має зберігати legacy path без додаткового preprocessing.
- `binning.switched=true, factor=2, mode=sum` має виконувати preprocessing перед тайлінгом і повертати координати вимірів у СК початкового кадру перед експортом.
- Невалідні значення `binning.factor` і `binning.mode` повинні завершувати запуск із помилкою валідації конфіга.

## Interpretation

Ця валідація є evidence для задачі AMNT-0004 та прикладом базового шаблону перевірки нових алгоритмічних feature-toggle змін.

## Failure cases

- Запуск із `factor`, що не підтримується (наприклад, 3).
- Запуск із `mode`, що не підтримується (наприклад, avg).
- Непридатний датасет (multi-channel без відповідної підтримки preprocessing).

## Typical misuse

- Порівнювати OFF/ON на різних датасетах або з різними конфігами сегментації.
- Робити висновок про якість детекції лише по FPS без аналізу метрик `Size`/координат.

## Open questions

- Чи потрібно окремо нормалізувати параметри сегментації для режиму binning за замовчуванням.
- Чи слід додати average/max binning як окремі режими в майбутньому.

## Connections

- used_by: AMNT-0004
- overlaps_with: dp1.config.prg_config
- overlaps_with: dp1.frame.frame_processor
- produces: AMNT-0004 external test report (`/mnt/D/AmanitaResources/AMNT-0004_binning_test_report_2026-03-22_uk.txt`)
