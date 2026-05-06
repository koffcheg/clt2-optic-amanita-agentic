---
id: dp1.stage.measurement
title:
  uk: "Етап вимірювання DP1"
  en: "DP1 Measurement stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.measurement.md"
  lines: "1-130"
status: "draft"
---

## Definition

Обчислює координати, геометрію, фотометрію і готує дані для DP2.

## Interface

`IMeasurementStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Перетворити валідовані об’єкти на structured Measurement domain: координати,
геометрія, фотометрія, frame/source metadata і дані, потрібні DP2.

## Inputs

Accepted `ValidatedObject[]`, metadata перетворення координат і потрібні
references з raw/processing. `Segment[]` може подаватися як source geometry або
shape reference, якщо measurement route потребує contour/moments.

## Internal computation domain

Структурований домен обчислення вимірювань. Photometry має виконуватися у
`CV_16UC1` raw або `CV_32FC1` processing domain; `CV_8UC1` допускається лише
як явно позначений допоміжний режим.

## Outputs

Домен вимірювань.

## Complexity variants

- `L0`: centroid плюс bounding box.
- `L1`: `L0` плюс mean/max intensity.
- `L2`: rotated bounding box, moments, subpixel уточнення.

## OpenCV mapping

- `moments`: `native`.
- `boundingRect`: `native`.
- `minAreaRect`: `native`.
- Subpixel або custom photometry rules: `custom` або `wrapped`.

## Config fragment

Ключ DSL: `measurement`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `centroid_bbox`, `photometry_basic`,
`moments_subpixel`.

## Timing / profiling

Профілювати час обчислення координат, геометрії, фотометрії, кількість
фінальних об’єктів і витрати на доступ до raw/processing references.

## Must not do

Надсилати візуалізацію, debug-зображення, внутрішні маски або тимчасові буфери обробки як canonical payload DP2.

## Constraints

Вимірювання мають зберігати одиниці, системи координат і metadata, потрібні downstream.

Критичні інваріанти:
- координати мають бути узгоджені з metadata `local <-> global`;
- photometry domain має бути явно вибраний (`raw16`, `proc32` або допоміжний
  `proc8`);
- measurement має читати тільки accepted `ValidatedObject`, якщо stage spec не
  визначає diagnostic route для rejected records;
- результат є structured data, а не `cv::Mat`.

## Failure cases

Невідповідність координат, втрата фотометрії, відсутня metadata.

## Typical misuse

Кодувати результат вимірювання як image buffer.

## Open questions

Точна схема корисного навантаження Measurement.

## Connections

- produces: dp1.domain.measurement
- uses: dp1.domain.struct.validated_object
- feeds: protocols.dp1_dp2.measurement_handoff
- constrained_by: dp1.domain.conversion_rules
