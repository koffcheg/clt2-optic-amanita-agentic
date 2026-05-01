---
id: dp2.types.Trajectory
title:
  uk: "Агрегований стан треку в DP2"
  en: "Aggregated track state in DP2"
tags: [dp2, types, tracking, trajectory]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro2/src/datapro2Types.h"
  lines: "30-52"
status: "draft"
---

## Definition
`Trajectory` зберігає історію вимірів (`deque<Measurement>`), ідентифікацію треку, оцінені параметри руху (позиція/швидкість/прискорення), статистику якості, прапор `updated` і `strobe_trj`.

## Assumptions
- `time0` є опорною часовою точкою для оцінених параметрів (`x0_hat`, `Vx_hat`, `ax_hat`, ...).
- Поля статистики (`std_dev_*`) та динаміки оновлюються алгоритмами обчислення в DP2 runtime.
- `measurements` впорядкований відповідно до часу надходження.

## Theorem / Contract
- `Trajectory` є канонічним контейнером стану треку для DP2 processing та для обміну з turret.
- Значення `id` має бути стабільним упродовж життя треку.
- `strobe_trj` повинен відповідати поточному стану оцінки траєкторії.

## Interpretation
Це центральна доменна сутність DP2: об'єднує observation history, prediction state і критерії фільтрації/показу.

## Failure cases
- Пошкоджена часово-упорядкована послідовність `measurements` робить least-squares оцінки нестабільними.
- Несинхронне оновлення `updated`/`unconfermed_frames_count` викликає помилки drop/show логіки.

## Typical misuse
- Інтерпретувати `speed`/`acceleration` як завжди валідні без перевірки мінімальної кількості точок.
- Серіалізувати трек без узгодження системи координат для споживача.

## Open questions
- Формальна умова переходу `unconfermed_frames_count` у drop для всіх режимів конфігурації.

## Connections
- uses: dp2.types.Measurement
- uses: dp2.types.TStrobe
- used_by: dp2.rpc.serialize_dp2_res
- used_by: dp2.config.dp2strobe_mth_cfg
- overlaps_with: dp1.types.TDataRes
