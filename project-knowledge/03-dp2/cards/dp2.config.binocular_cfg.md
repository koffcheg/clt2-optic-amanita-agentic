---
id: dp2.config.binocular_cfg
title:
  uk: "Конфігурація бінокулярного режиму DP2"
  en: "DP2 binocular mode configuration"
tags: [dp2, config, binocular]
source:
  file: "datapro2/src/dp2_srobe_mth_chg.h"
  lines: "6-15"
status: "draft"
---

## Definition
`binocular_cfg` задає параметри для обробки двокамерного потоку: ввімкнення режиму, політику match, межі черг кадрів, тестовий режим точки та його параметри.

## Assumptions
- Параметри `min_size_queue_frame`/`max_size_queue_frame` задають допустимі розміри черг синхронізації камер.
- `match_distance` використовується як поріг просторового зіставлення вимірів між камерами.

## Theorem / Contract
- Якщо `enabled == false`, DP2 працює в монокулярному режимі.
- Якщо `test_point == true`, runtime може додавати синтетичні виміри для тестового сценарію.

## Interpretation
Це спеціалізована конфіг-секція, що керує pipeline branch для multi-vision.

## Failure cases
- Занадто малий `max_size_queue_frame` викликає часті скидання черг.
- Некоректний `match_distance` дає false match або пропуски match.

## Typical misuse
- Вмикати `test_point` у production-сценарії без явної потреби.

## Open questions
- Формальна одиниця виміру `match_distance` у всіх перетвореннях координат.

## Connections
- used_by: dp2.config.dp2_cfg
- used_by: dp2.rpc.dp2_init_measure_proc_algo
