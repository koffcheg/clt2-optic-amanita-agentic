---
id: dp2.config.dp2_cfg
title:
  uk: "Коренева конфігурація DP2"
  en: "DP2 root configuration"
tags: [dp2, config]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro2/src/dp2_cfg.h"
  lines: "8-26"
status: "draft"
---

## Definition
`dp2::dp2_cfg` - агрегуюча структура конфігу DP2 з полями `dp2_id`, `port`, `strobe_mth`, `turret_exch`, `binocular`; наповнюється з JSON у конструкторі `dp2_cfg(const char* cfg_fname)`.

## Assumptions
- Конструктор очікує коректний JSON-файл і кидає `logic_error` при помилці відкриття/парсингу.
- Наявність ключів у JSON є критичною для більшості параметрів.

## Theorem / Contract
- `dp2_cfg` є єдиною точкою входу параметризації DP2 runtime на старті процесу.
- Внутрішні підструктури (`strobe_mth`, `turret_exch`, `binocular`) відображають відповідні JSON секції.

## Interpretation
Це контейнер policy/config рівня модуля, який формує поведінку server, tracking і зовнішнього обміну.

## Failure cases
- Відсутні або некоректні ключі в JSON зривають ініціалізацію DP2.
- Неконсистентні параметри між секціями можуть проявлятися як runtime деградація (drop треків, reconnect storms).

## Typical misuse
- Передавати шлях до неактуального конфігу для іншого профілю камер/роздільної здатності.

## Open questions
- Чи є стабільний набір optional keys з дефолтами, окрім явного clamping в turret-секції.

## Connections
- uses: dp2.config.dp2strobe_mth_cfg
- uses: dp2.config.dp2_cfg.turret_exch_cfg
- uses: dp2.config.binocular_cfg
- used_by: dp2.runtime.dp2_main
