---
id: dp1.domain.visualization
title:
  uk: "Домен візуалізації DP1"
  en: "DP1 Visualization domain"
tags: [dp1, canonical, data-domain]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.visualization.md"
  lines: "1-120"
status: "draft"
---

## Definition

Домен візуалізації DP1 є необов'язковим human-facing представленням
canonical-стану обробки або вимірювань.

Формат visualization frame має відповідати формату вхідного кадру поточного
DP1 instance route: `U8` / `CV_8UC1` або `U16` / `CV_16UC1`. Canonical
visualization domain не вводить окремий RGB/BGR carrier.

## Assumptions

Використовується для відображення, налагодження та перегляду людиною.

## Theorem / Contract

Домен візуалізації не можна використовувати як джерело обчислень.

Overlay, якщо він потрібен, має бути описаний як metadata або route-specific
debug representation без зміни canonical image carrier на RGB/BGR.

## Interpretation

Візуалізація є необов’язковим представленням canonical-стану обробки або вимірювань.

## Failure cases

- Передача намальованих overlay у виявлення.
- Перетворення input `CV_16UC1` у `CV_8UC3` як canonical visualization output.

## Typical misuse

- Трактувати debug-вихід як canonical-дані.

## Open questions

- Стандартні назви debug-представлень.
- Exact overlay metadata schema.

## Connections

- derived_from: dp1.domain.measurement
- constrained_by: dp1.domain.pixel_format
- constrained_by: dp1.domain.opencv_invariants
