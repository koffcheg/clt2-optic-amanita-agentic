---
id: dp2.types.PTPoint
title:
  uk: "Кандидат точки трекінгу DP2"
  en: "DP2 tracking candidate point"
tags: [dp2, types, tracking]
source:
  file: "datapro2/src/datapro2Types.h"
  lines: "18-21"
status: "draft"
---

## Definition
`PTPoint` обгортає `Measurement` і додає `passed_frames` - лічильник кадрів, протягом яких точка існує в буфері асоціації.

## Assumptions
- `passed_frames` збільшується логікою DP2 під час життєвого циклу точки.
- Значення за замовчуванням `1` означає, що точка щойно створена.

## Theorem / Contract
- `PTPoint` є проміжною сутністю між сирими вимірами і побудованими траєкторіями.
- Без валідного `measurement` елемент `PTPoint` не має сенсу.

## Interpretation
Це стан-контейнер для етапу short-term tracking до включення в `Trajectory`.

## Failure cases
- Неправильний облік `passed_frames` призводить до передчасного drop або зависання точки.

## Typical misuse
- Повторно використовувати `PTPoint` між камерами без перевірки системи координат.

## Open questions
- Які точні умови переходу з `PTPoint` в `Trajectory` для всіх режимів DP2.

## Connections
- uses: dp2.types.Measurement
- produces: dp2.types.Trajectory
