---
id: dp2.config.dp2_cfg.turret_exch_cfg
title:
  uk: "Параметри обміну DP2 з turret"
  en: "DP2 to turret exchange parameters"
tags: [dp2, config, turret, network]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro2/src/dp2_cfg.h"
  lines: "9-16"
status: "draft"
---

## Definition
`dp2_cfg::turret_exch_cfg` описує параметри каналу DP2 -> turret: `enabled`, `host`, `port`, `tr_interval_ms`, `reconn_interval_s`.

## Assumptions
- Значення читаються з JSON секції `turret_exch` у конструкторі `dp2_cfg`.
- `tr_interval_ms` і `reconn_interval_s` мають runtime clamping (мінімальні значення) у `dp2_cfg.cpp`.

## Theorem / Contract
- Якщо `enabled == false`, відправка треків у turret не повинна ініціюватись.
- `host:port` має вказувати на доступний endpoint споживача повідомлень.

## Interpretation
Це мережевий конфіг-контракт для зовнішнього інтеграційного каналу керування/наведення.

## Failure cases
- Невалідний host/port призводить до циклів reconnect і втрати outbound telemetry.

## Typical misuse
- Встановлювати надто малий `tr_interval_ms`, що створює зайве навантаження на канал.

## Open questions
- Чи потрібні додаткові backoff-параметри reconnect поза фіксованим інтервалом.

## Connections
- used_by: dp2.config.dp2_cfg
- used_by: dp2.net.dp2_tr_to_turret
