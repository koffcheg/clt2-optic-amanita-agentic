---
id: dp2.config.dp2strobe_mth_cfg
title:
  uk: "Конфіг strobe-методу і трекінгу DP2"
  en: "DP2 strobe-method and tracking configuration"
tags: [dp2, config, tracking, strobe]
source:
  file: "datapro2/src/dp2_srobe_mth_chg.h"
  lines: "17-52"
status: "draft"
---

## Definition
`dp2strobe_mth_cfg` містить пороги і ліміти для життєвого циклу треків: speed/acceleration/std_dev межі, drop/show критерії, параметри noisy-region фільтра, output folder та flags збереження результатів, параметри моделі (`linear_model`, `k_std`, `std_measurement`, `k_dist`).

## Assumptions
- Значення зчитуються з JSON конфігу через `dp2_cfg`.
- Межі drop/show мають бути узгоджені з частотою кадрів і масштабом координат.

## Theorem / Contract
- Ця структура є джерелом параметрів для конструктора `StrobeMethod`.
- Некоректні або надто агресивні пороги змінюють поведінку lifecycle треків без змін коду.

## Interpretation
Це policy-layer DP2 tracking: визначає, які треки вважати валідними, коли їх показувати і коли скидати.

## Failure cases
- Невдале поєднання порогів може повністю приглушити показ треків або, навпаки, пропускати шум.

## Typical misuse
- Налаштовувати пороги без урахування одиниць виміру та FPS вхідних потоків.

## Open questions
- Чи всі поля мають hard-fail при відсутності в JSON, чи частина може мати implicit defaults через reader.

## Connections
- used_by: dp2.config.dp2_cfg
- used_by: dp2.rpc.dp2_init_measure_proc_algo
- used_by: dp2.types.Trajectory
