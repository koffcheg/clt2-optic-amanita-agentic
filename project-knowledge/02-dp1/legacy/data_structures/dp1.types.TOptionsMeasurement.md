---
id: dp1.types.TOptionsMeasurement
title:
  uk: "TOptionsMeasurement - виміри одного об’єкта"
  en: "TOptionsMeasurement - measurements of one object"
tags: [dp1, struct, measurement, segmentation]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "43-59"
status: "draft"
---

## Definition
Набір числових характеристик сегментованого об’єкта (контур/маска), який є виходом DP1 для подальшої обробки (DP2, трекінг, аналітика).

**Поля (узагальнено):**
- `id_obj`: ID об’єкта в межах кадра (не глобальний).
- `num_pix_obj`: кількість пікселів об’єкта (площа в px).
- `x_weight, y_weight`: центроїд/центр мас (у пікселях) - назва натякає на moments-weighted.
- `x_rec, y_rec`: координати прямокутника (bbox) або його центру (залежить від реалізації).
- `rec_width, rec_height`: розміри bbox (px).
- `mean_brightness_obj`, `std_brightness_obj`: статистика яскравості по об’єкту.
- `mean_brightness_im`, `std_brightness_im`: статистика яскравості по кадру/тайлу.
- `obj_angel`: кут орієнтації (ймовірно з RotatedRect).
- `obj_angel_moment`: кут з моментів (альтернатива).
- `obj_eccentricity`: ексцентриситет (форма).

## Assumptions
- Координати (`x_*`, `y_*`) знаходяться в координатах **повного кадра**, а не локального тайлу (якщо є shift_x/shift_y, має бути враховано).
- `id_obj` унікальний в межах `index_frame`.
- Статистики яскравості відповідають типу кадра (8-bit vs 16-bit). Якщо кадр CV_16U, а розрахунок робиться на CV_8U - це потрібно фіксувати.

## Theorem / Contract
- Один запис `TOptionsMeasurement` відповідає одному валідному контуру/сегменту після фільтрації за розмірами (`min_segment_size`, `max_segment_size`, ratio).
- `num_pix_obj > 0`.

## Interpretation
Це “рядок таблиці вимірів” для одного знайденого об’єкта.

## Failure cases
- Змішування систем координат (тайл vs повний кадр).
- Невизначена домовленість: `x_rec/y_rec` - це кут чи центр → помилки при візуалізації/трекингу.

## Typical misuse
- Вважати `id_obj` стабільним між кадрами (він не трековий).
- Інтерпретувати `mean_brightness_*` без знання, чи кадр був нормалізований 16→8.

## Connections
- produced_by: dp1.methods.calcMeasurement (datarpoSegmentation.h)
- linked_to: dp1.types.TDrawMeasurement (геометрія для відрисовки)
- aggregated_in: dp1.types.TDataRes
