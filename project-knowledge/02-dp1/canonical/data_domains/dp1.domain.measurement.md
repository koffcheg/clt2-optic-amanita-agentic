---
id: dp1.domain.measurement
title:
  uk: "Measurement домен DP1"
  en: "DP1 Measurement domain"
tags: [dp1, canonical, data-domain, measurement]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.measurement.md"
status: "draft"
---

## Definition

Measurement domain описує семантичну роль фінального структурованого виходу DP1, який може передаватися через межу DP1 -> DP2.

Цей домен не є контейнером для `cv::Mat`, debug-зображень, внутрішніх масок або тимчасових буферів обробки.

Конкретна структура запису вимірювання описана окремою карткою `dp1.domain.measurement.record`.

## Assumptions

- Measurement domain має містити координати, геометрію, фотометрію та downstream metadata, потрібні для DP2.
- Точна схема payload має бути узгоджена з `protocols.dp1_dp2.measurement_handoff`.
- Конкретна C++ структура має визначатися окремою implementation task.

## Theorem / Contract

Для Measurement domain діють такі правила:

- вихід вимірювань є structured data, а не `cv::Mat`;
- вихід вимірювань є продуктовим результатом DP1, а не debug artifact;
- вихід має містити достатньо identity/time/source/coordinate metadata для інтерпретації в DP2;
- внутрішні маски, visualization images і temporary buffers не входять у canonical handoff;
- measurement records можуть посилатися на source segments або raw/processing дані, використані для фотометрії.

Canonical structure:

- `dp1.domain.measurement.record`.

## Interpretation

Measurement domain є canonical source domain для межі протоколу DP1 -> DP2. Це результат `measurement` stage, а не загальний контейнер для будь-яких structured objects.

## Failure cases

- Внутрішні маски або visualization images передаються як canonical DP2 input.
- Measurement не містить `frame_id`, time/source identity або coordinate metadata.
- Геометрія видається без системи координат.
- Фотометрія рахується з display/debug buffer замість Raw/Processing/Measurement domain.

## Typical misuse

- Кодувати measurement як pixels.
- Трактувати `Candidate` або `Segment` як final measurement без contract `measurement` stage.

## Open questions

- Точна payload schema і versioning policy.
- Обов'язкова calibration metadata.
- Одиниці координат і фотометрії.
- Семантика помилок і partial-frame cases для DP2 handoff.

## Connections

- has_structure: dp1.domain.measurement.record
- produced_by: dp1.stage.measurement
- derived_from: dp1.domain.struct.segment
- feeds: protocols.dp1_dp2.measurement_handoff
