---
id: dp1.config.calc_tile_lim_cfg_t
title:
  uk: "calc_tile_lim_cfg_t - конфіг обмеження тайлів за часом"
  en: "calc_tile_lim_cfg_t - tile limiting config by timing"
tags: [dp1, struct, config, performance]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_config.h"
  lines: "12-16"
status: "draft"
---

## Definition
Конфіг для адаптивного пропуску/обмеження обробки тайлів (performance governor), щоб утримувати заданий time budget.

**Поля:**
- `enable` (bool): увімк/вимк логіку.
- `ref_frame_proc_time_ms` (double): еталонний час обробки кадра (мс), від якого рахується rate.
- `rates` (map<double,double>): piecewise mapping “реальний_коефіцієнт_часу → частка/стратегія обробки”.

## Assumptions
- `rates` визначає доменну політику (наприклад, якщо повільніше в 2× → обробляємо тільки N% тайлів).
- Потрібен механізм пріоритетів тайлів (frame_priorities) для розумного пропуску.

## Theorem / Contract
- Якщо `enable==true`, DP1 може навмисно **не обробити всі тайли** кадра, щоб не зірвати realtime.
- Downstream повинен бути готовий до “часткового кадра” (неповного набору об’єктів).

## Interpretation
Це knob для продуктивності: “краще частково, ніж запізно”.

## Failure cases
- Неправильно підібрані `rates` → нестабільна детекція (флікер).
- Використання без логування “які тайли пропущено” → немає трасованості.

## Typical misuse
- Вмикати governor в режимі збору датасету/еталонних вимірів.
- Не пояснити в документації, що результати можуть бути неповні.

## Connections
- used_by: dp1.tiles.i_calc_tile_limit
- created_by: dp1.tiles.get_tile_calc_limiter
