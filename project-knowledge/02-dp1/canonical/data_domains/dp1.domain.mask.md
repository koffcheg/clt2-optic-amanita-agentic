---
id: dp1.domain.mask
title:
  uk: "Домен масок DP1"
  en: "DP1 Mask domain"
tags: [dp1, canonical, data-domain]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.mask.md"
  lines: "1-120"
status: "draft"
---

## Definition

Домен масок. Тип: `CV_8UC1`.

## Assumptions

Використовується для масок, результатів thresholding і сегментації.

## Theorem / Contract

Значення маски кодують класифікацію або вибір, а не фотометричні вимірювання.

## Interpretation

Домен масок може керувати виділенням кандидатів і уточненням сегментації.

## Failure cases

- Використання інтенсивності маски як джерела фотометрії.

## Typical misuse

- Змішування буферів масок із буферами обробки.

## Open questions

- Стандартний словник значень маски.

## Connections

- used_by: dp1.stage.candidate_extraction
- used_by: dp1.stage.segmentation_refinement
