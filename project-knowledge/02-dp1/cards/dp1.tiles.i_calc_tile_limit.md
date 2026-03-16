---
id: dp1.tiles.i_calc_tile_limit
title:
  uk: "i_calc_tile_limit - інтерфейс ліміту обробки тайлів"
  en: "i_calc_tile_limit - tile processing limiter interface"
tags: [dp1, class, interface, tiling, performance]
source:
  file: "datapro1/src/dp1_calc_limit.h"
  lines: "12-31"
status: "draft"
---

## Definition
Інтерфейс, який вирішує: чи потрібно обробляти конкретний тайл у наступному кадрі, і дозволяє оновлювати статистику часу обробки.

Методи:
- `need_proc_tile(size_t tileIndex) const -> bool`
- `set_last_frame_proc_time_mc(unsigned int frame_proc_time)`

## Assumptions
- `need_proc_tile` може викликатись з кількох потоків.
- `set_last_frame_proc_time_mc` **не можна** викликати конкурентно ні з чим (ні з need_proc_tile, ні з іншими set_...).

## Theorem / Contract
- Якщо limiter повертає `false` для тайла, DP1 пропускає обробку цього тайла (або використовує попередні/порожні результати - залежить від реалізації).
- Вхід `tileIndex` має відповідати лінійній нумерації тайлів (домовленість: i*numFragX + j або інша).

## Interpretation
Це “контролер навантаження” для тайлової обробки.

## Failure cases
- Порушити умову неконкурентності `set_last_frame_proc_time_mc` → data race.
- Неправильне мапування tileIndex ↔ (i,j) → випадкові пропуски.

## Typical misuse
- Не документувати policy limiter-а (складно відтворити експерименти).
- Використовувати limiter як “фільтр якості” (це лише про час).

## Connections
- configured_by: dp1.config.calc_tile_lim_cfg_t
- used_by: dp1.methods.datapro1 (параметр calc_tile_limit у datapro1.h)
