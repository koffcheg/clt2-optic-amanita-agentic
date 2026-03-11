---
id: dp1.types.TDataproVar
title:
  uk: "TDataproVar - робочі буфери DP1"
  en: "TDataproVar - DP1 working buffers"
tags: [dp1, struct, buffers, state]
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "91-95"
status: "draft"
---

## Definition
Набір робочих буферів, що зберігають проміжні дані по тайлах і стан фонових віднімачів.

**Поля:**
- `data_draw`: (vector<TDrawMeasurement>) геометрія об’єктів для малювання (може агрегуватись по тайлах/кадру).
- `vec_frag`: (vector<cv::Mat>) тайли кадра (фрагменти).
- `vec_bgmask`: (vector<cv::Mat>) маски після background subtractor по тайлах.
- `vec_bgsubtractor`: (vector<cv::Ptr<cv::BackgroundSubtractor>>) по одному subtractor на тайл.

## Assumptions
- Всі вектори мають довжину `numFragX*numFragY`.
- `vec_bgsubtractor[k]` ініціалізований (KNN або MOG2) до обробки кадрів.
- `cv::Mat` у `vec_frag/vec_bgmask` мають узгоджені типи (8U/16U/1ch згідно з реалізацією).

## Theorem / Contract
- “Окремий subtractor на тайл” дає локальну адаптацію фону, але створює ризик різних станів на межах тайлів.

## Interpretation
Це “stateful” частина DP1: без неї обробка кадрів не відтворювана (бо subtractor має пам’ять).

## Failure cases
- Перевикористання `TDataproVar` з іншою геометрією кадра без реініціалізації.
- Змішування кадрів з різними типами/масштабом → некоректні маски.

## Typical misuse
- Очищати/пересоздавати `vec_bgsubtractor` кожен кадр (втрата історії).
- Доступ до `vec_*` з кількох потоків без синхронізації, якщо реалізація мутаційна.

## Connections
- configured_by: dp1.types.TDataproConfig
- filled_by: dp1.methods.datapro1 (datapro1.cpp)
- consumes: OpenCV BackgroundSubtractor
