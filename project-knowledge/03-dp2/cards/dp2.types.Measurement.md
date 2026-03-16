---
id: dp2.types.Measurement
title:
  uk: "Базовий вимір об'єкта в DP2"
  en: "Base object measurement in DP2"
tags: [dp2, types, tracking, measurement]
source:
  file: "datapro2/src/datapro2Types.h"
  lines: "8-16"
status: "draft"
---

## Definition
`Measurement` описує одне спостереження об'єкта: 2D координати (`x`,`y`), 3D координати (`Xp`,`Yp`,`Zp`), кадр (`iframe`), час (`time`), кути (`Az`,`El`) та оцінки дисперсій.

## Assumptions
- `time` має бути узгоджений по джерелу часу з іншими вимірами в межах треку.
- `Xp/Yp/Zp` можуть бути нульовими або службовими до етапу тріангуляції/перетворення.
- `dispersion*` заповнюються алгоритмами оцінювання і за замовчуванням ініціалізовані нулем.

## Theorem / Contract
- Елемент `Measurement` є атомом для побудови `PTPoint` і `Trajectory`.
- Алгоритми DP2 читають поля як mutable POD-подібний стан без інкапсуляції.

## Interpretation
Це базова одиниця даних трекінгу в DP2 між етапами асоціації, фільтрації та оцінки параметрів руху.

## Failure cases
- Змішування часових шкал між камерами призводить до некоректного прогнозу треків.
- Неконсистентні 2D/3D координати псують strobe-фільтрацію та оцінки швидкості.

## Typical misuse
- Використовувати неініціалізовані 3D поля як валідні просторові координати.
- Передавати `Measurement` з невірним `iframe` при асоціації між кадрами.

## Open questions
- Чи допускається sentinel-значення для відсутніх 3D координат, окрім нуля.

## Connections
- used_by: dp2.types.PTPoint
- used_by: dp2.types.Trajectory
- overlaps_with: dp1.types.TDataRes
