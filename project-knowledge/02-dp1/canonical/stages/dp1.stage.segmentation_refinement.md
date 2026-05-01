---
id: dp1.stage.segmentation_refinement
title:
  uk: "Етап уточнення сегментації DP1"
  en: "DP1 Segmentation refinement stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.segmentation_refinement.md"
  lines: "1-130"
status: "draft"
---

## Definition

Уточнює сегментацію через morphology, merging і виділення contour або connected components.

## Interface

`ISegmentationStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Очистити mask, стабілізувати межі кандидатів і сформувати контури або
connected components для подальшої фільтрації.

## Inputs

Домен масок і гіпотези кандидатів.

## Internal computation domain

Домен масок `CV_8UC1` плюс структуровані metadata кандидатів.

## Outputs

Контури або connected components з metadata, достатніми для object filtering.

## Complexity variants

- `L0`: одна морфологічна операція.
- `L1`: open/close плюс contours.
- `L2`: багатокрокова морфологія або складніша схема refinement.

## OpenCV mapping

- `morphologyEx`: `native`.
- `findContours`: `native`.
- `connectedComponents`: `native`.

## Config fragment

Ключ DSL: `segmentation`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `single_morphology`, `open_close_contours`,
`connected_components`, `multi_step_morphology`.

## Timing / profiling

Профілювати час morphology, кількість contours/components, розмір kernel,
кількість проходів і кількість сегментів на виході.

## Must not do

Фотометричне вимірювання або фінальне прийняття цілі.

## Constraints

Морфологічні операції мають зберігати traceability до гіпотез кандидатів.

Критичні інваріанти:
- параметризація kernel має бути явною;
- злиття і розрив сегментів мають контролюватися;
- refinement не повинен приховувати помилки detector stage без валідації.

## Failure cases

Надмірне злиття, надмірне розділення, втрата контуру на межах.

## Typical misuse

Використовувати morphology, щоб приховати помилки детектора без валідації.

## Open questions

Canonical-представлення компоненти.

## Connections

- feeds: dp1.stage.object_filtering
- uses: dp1.domain.mask
