---
id: dp1.domain.raw
title:
  uk: "Домен сирих даних DP1"
  en: "DP1 Raw domain"
tags: [dp1, canonical, data-domain]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.raw.md"
  lines: "1-120"
status: "draft"
---

## Definition

Домен сирих сенсорних даних. Тип: `CV_16UC1` або еквівалент.

## Assumptions

Сирі дані надходять із сенсора або джерела, еквівалентного сенсору.

## Theorem / Contract

Домен сирих даних зберігає максимальний динамічний діапазон і не містить попередньої обробки.

## Interpretation

Сирі кадри є вихідними вимірюваннями, а не буферами візуалізації.

## Failure cases

- Раннє перетворення у `CV_8U`.
- Прихована нормалізація перед формуванням вимірювань.

## Typical misuse

- Виконувати фотометричні обчислення після перетворення з втратою даних.

## Open questions

- Точні варіанти розрядності сенсора.

## Connections

- feeds: dp1.stage.prep
