---
id: dp1.types.TFolder
title:
  uk: "TFolder - імена підпапок для дебагу/експорту"
  en: "TFolder - subfolder names for debug/export"
tags: [dp1, struct, io, paths]
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "97-105"
status: "draft"
---

## Definition
Набір “канонічних” назв підпапок для збереження проміжних результатів/логів.

**Поля (типові значення):**
- `data_bin`: бінарні дані/лог.
- `data_pix`: піксельні дані (якщо є).
- `frame_input`: вхідні кадри.
- `frame_background`: фон.
- `frame_diff`: різниця/вихід subtractor.
- `frame_segment`: сегментація/маска.
- `frame_coor`: візуалізація координат/об’єктів.

## Assumptions
- Значення полів — це відносні імена директорій; базовий шлях задається у конфігу (`cfg_test.out_folder`).

## Theorem / Contract
- Використання сталих імен директорій спрощує RAG/трасування, бо downstream може очікувати стандартну структуру.

## Interpretation
Це “домовленість” для організації артефактів експерименту.

## Failure cases
- Некоректний шлях / відсутність прав на запис → silent fail або виняток.
- Колізії імен при паралельних камерах без рознесення по `cam_index`.

## Typical misuse
- Писати в одну папку з різних камер без префікса/окремої гілки.
- Міняти назви папок без оновлення скриптів аналізу.

## Connections
- used_by: dp1.save.save_res, dp1.methods.datapro1 (debug images)
- configured_by: dp1.config.prg_config.cfg_test (out_folder)
