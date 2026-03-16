---
id: dp1.types.TDrawMeasurement
title:
  uk: "TDrawMeasurement - геометрія для відрисовки"
  en: "TDrawMeasurement - drawing geometry"
tags: [dp1, struct, visualization, opencv]
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "61-64"
status: "draft"
---

## Definition
Структура для візуалізації одного об’єкта: axis-aligned bbox + rotated bbox.

**Поля:**
- `box` (cv::RotatedRect): повернутий прямокутник (центр, розміри, кут).
- `rect` (cv::Rect): звичайний AABB прямокутник.

## Assumptions
- `rect` та `box` описують **той самий** сегмент/контур.
- Кут `box.angle` має OpenCV-конвенцію (діапазон/знак).

## Theorem / Contract
- Якщо `rect` виходить за межі кадра, його треба “clip”-нути перед доступом до ROI.
- `box` може бути виродженим для дуже малих/лінійних контурів.

## Interpretation
Цей тип існує, щоб розділити “виміри для алгоритмів” і “геометрію для малювання/дебагу”.

## Failure cases
- Неправильний shift (тайл→кадр) → bbox малюється “не там”.
- Використання `RotatedRect` без перевірки на валідність (NaN після некоректних контурів).

## Typical misuse
- Виводити тільки `rect` і ігнорувати `box`, якщо downstream очікує кут.
- Переплутати “кут” і “поворот у градусах/радіанах”.

## Connections
- produced_by: dp1.methods.calc_contour_param (datarpoSegmentation.h)
- consumed_by: dp1.methods.showing (datapro1.h)
- paired_with: dp1.types.TOptionsMeasurement
