---
id: dp1.config.prg_config
title:
  uk: "prg_config - конфіг DP1 (JSON → структури)"
  en: "prg_config - DP1 configuration (JSON → structs)"
tags: [dp1, class, config, json, runtime]
source:
  file: "datapro1/src/dp1_config.h"
  lines: "17-170"
status: "draft"
---

## Definition
Клас конфігурації програми DP1, який читає JSON (`config_datapro1.json`) і тримає типізовані секції конфігу.

Вміщує вкладені структури:
- `cfg_one_filter`, `cfg_median_filter`, `cfg_filters`
- `cfg_segment`
- `cfg_median` (median background)
- `cfg_subtractor` (KNN/MOG2)
- `cfg_source_frame` (IPC/URI)
- `cfg_display`, `cfg_save2file`, `cfg_test`
- `cfg_multiproc` (threads/tiles)
- `cfg_binocular`
- `calc_tile_lim_cfg_t`
- `ipc_name_cfg` (зовнішній тип з m_cfg_if.h)

## Assumptions
- Конфіг файл має коректну схему (ключі/типи). Якщо ні - `check_cfg` має виявити і зупинити запуск (або застосувати дефолти, якщо так задумано).
- Всі розміри/пороги узгоджені з типом вхідного кадра (8-bit vs 16-bit) і з доменом.

## Theorem / Contract
- `get_frame_src_type()` визначає тип джерела кадра (IPC/URI) на основі `cfg_source_frame`.
- `multiproc.tiles_factor` впливає на `numFragX/numFragY` через `calcNumberFrag`.
- `def_border` використовується як fallback для border_x/y.

## Interpretation
Це “єдиний вхід” для параметрів конвеєра: фільтри, сегментація, субтрактор, тестовий режим, візуалізація, продуктивність.

## Failure cases
- Погані дефолти в полях (0 або 1) маскують помилки конфігу, але дають некоректний результат.
- Невідомі/зайві поля JSON ігноруються → конфіг “не діє”, а користувач думає інакше.

## Typical misuse
- Змінити конфіг і забути зафіксувати версію (schema_version) в артефактах експерименту.
- Використовувати один конфіг на різні камери без параметрів per-camera (camera_index).

## Connections
- used_by: dp1.runner.run_ipc_src / dp1.runner.run_uri_src
- controls: dp1.types.TDataproConfig initialization, dp1.methods.datapro1 parameters
- includes: dp1.config.calc_tile_lim_cfg_t
