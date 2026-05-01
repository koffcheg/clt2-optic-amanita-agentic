---
id: dp1.domain.measurement
title:
  uk: "Домен вимірювань DP1"
  en: "DP1 Measurement domain"
tags: [dp1, canonical, data-domain]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.measurement.md"
  lines: "1-120"
status: "draft"
---

## Definition

Структуровані дані вимірювань, не `cv::Mat`.

## Assumptions

Домен вимірювань зберігає координати, геометрію, фотометрію і downstream metadata, потрібні DP2.

## Theorem / Contract

Домен вимірювань є canonical-джерелом для передачі DP1 -> DP2.

## Interpretation

Це продуктовий вихід DP1, а не debug artifact.

## Failure cases

- Надсилання внутрішніх масок або зображень візуалізації як canonical-входу DP2.

## Typical misuse

- Кодувати вимірювання як pixels.

## Open questions

- Точні поля корисного навантаження та одиниці вимірювання.

## Connections

- produced_by: dp1.stage.measurement
- feeds: protocols.dp1_dp2.measurement_handoff
