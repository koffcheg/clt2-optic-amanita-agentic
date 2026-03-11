---
id: dp1.types.TDataproConfig
title:
  uk: "TDataproConfig - параметри тайлінгу та обробки"
  en: "TDataproConfig - tiling & processing parameters"
tags: [dp1, struct, tiling, config]
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "79-89"
status: "draft"
---

## Definition
Параметри, які визначають як кадр ділиться на тайли та як обробка працює на межах тайлів.

**Поля:**
- `numFragX`, `numFragY`: кількість тайлів по X/Y.
- `border_x`, `border_y`: ширина перекриття (overlap) між тайлами.
- `sizePartY`, `sizePartX`: базовий розмір тайла.
- `sizePartYend`, `sizePartXend`: розмір “останнього” тайла (може бути більший/менший).
- `update_bg_model`: чи оновлювати модель фону (для subtractor apply).

## Assumptions
- `numFragX*numFragY` відповідає кількості елементів у `TDataproVar.vec_*`.
- `border_x/border_y` достатні, щоб фільтри/морфологія не давали швів на стиках.
- Якщо `update_bg_model == false`, фонова модель “заморожена” (адаптація не відбувається).

## Theorem / Contract
- Правильний вибір `border_*` зменшує артефакти на межах тайлів, але збільшує обчислення.
- `sizePart*` + `border_*` визначають ROI тайла для збереження/дебагу.

## Interpretation
Це “геометричний конфіг” для тайлової обробки + прапорець керування адаптацією фону.

## Failure cases
- Невірні `sizePartXend/sizePartYend` → вихід за межі матриці.
- Малі border при великих ядрах → розриви контурів на стиках.

## Typical misuse
- Змінити `numFragX/numFragY` без переініціалізації `TDataproVar` (вектора роз’їдуться).
- Забути синхронізувати `update_bg_model` з режимом тестування/відлагодження.

## Connections
- initialized_by: dp1.methods.initializingParamDatapro1 (datapro1.h)
- used_by: dp1.runtime.dp1_th_proc_par (multithreading)
- paired_with: dp1.types.TDataproVar
