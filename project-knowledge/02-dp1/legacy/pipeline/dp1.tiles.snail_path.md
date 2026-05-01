---
id: dp1.tiles.snail_path
title:
  uk: "snail_path - порядок обходу тайлів (спіраль/равлик)"
  en: "snail_path - tile traversal order (snail/spiral)"
tags: [dp1, class, tiling, algorithm]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_snail_path.h"
  lines: "12-55"
status: "draft"
---

## Definition
Клас, який генерує послідовність (row,col) для обходу прямокутної сітки тайлів за “спіральною/равликовою” траєкторією.

API:
- ctor(num_rows, num_cols)
- `get_path() -> vector<pair<size_t,size_t>>`

## Assumptions
- `num_rows, num_cols > 0`.
- Внутрішній механізм “marked_rows/marked_cols” визначає, коли ряд/колонка вже “закриті” для подальшого руху.

## Theorem / Contract
- `get_path()` повертає список довжини `num_rows*num_cols` без повторів (очікувана властивість; потрібно підтверджувати тестом).
- Порядок визначає “пріоритет” тайлів, якщо DP1 робить partial processing.

## Interpretation
Практичний сенс: у realtime сценаріях можна обробляти тайли в “найважливішому” порядку (наприклад, від центру).

## Failure cases
- Помилки логіки marked_* → пропуски або дублікати.
- Невизначена домовленість: з якого тайла стартує і в якому напрямку.

## Typical misuse
- Використати порядок як гарантований “за важливістю” без доменного обґрунтування.
- Не логувати/не тестувати, якщо порядок впливає на результати.

## Connections
- often_used_with: dp1.tiles.i_calc_tile_limit (як policy “які тайли першими”)
- used_in: dp1.runtime.dp1_th_proc_par (якщо черга tiles_to_proc заповнюється за цим порядком)
