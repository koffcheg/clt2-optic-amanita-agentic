---
id: dp2.types.TStrobe
title:
  uk: "Strobe-область траєкторії"
  en: "Trajectory strobe region"
tags: [dp2, types, strobe, tracking]
source:
  file: "datapro2/src/datapro2Types.h"
  lines: "23-28"
status: "draft"
---

## Definition
`TStrobe` задає параметри просторового вікна для треку: ідентифікатор траєкторії та межі області (`min/max`) у 3D.

## Assumptions
- Межі `min/max` мають бути узгоджені з тією ж системою координат, що і `Trajectory`.
- `id_trajectory` відповідає `Trajectory::id`.

## Theorem / Contract
- `TStrobe` використовується для перевірки приналежності нових вимірів до треку.
- Межі визначають допустиму область асоціації, а не геометрію об'єкта.

## Interpretation
Це геометричний фільтр-контракт між оціненим станом траєкторії і вхідними вимірами.

## Failure cases
- Інверсія `min/max` або різні системи координат ведуть до втрати релевантних точок.

## Typical misuse
- Вважати `TStrobe` фізичними габаритами цілі замість області пошуку.

## Open questions
- Де саме у поточній реалізації нормалізуються/перевіряються межі strobe при зміні моделі руху.

## Connections
- used_by: dp2.types.Trajectory
- used_by: dp2.config.dp2strobe_mth_cfg
