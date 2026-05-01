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

Домен візуалізації. Тип: `CV_8UC3`.

## Assumptions

Використовується для відображення, налагодження та перегляду людиною.

## Theorem / Contract

Домен візуалізації не можна використовувати як джерело обчислень.

## Interpretation

Візуалізація є необов’язковим представленням canonical-стану обробки або вимірювань.

## Failure cases

- Передача намальованих overlay у виявлення.

## Typical misuse

- Трактувати debug-вихід як canonical-дані.

## Open questions

- Стандартні назви debug-представлень.

## Connections

- derived_from: dp1.domain.measurement
