---
id: dp1.domain.processing
title:
  uk: "Домен обробки DP1"
  en: "DP1 Processing domain"
tags: [dp1, canonical, data-domain]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.processing.md"
  lines: "1-120"
status: "draft"
---

## Definition

Домен обчислювальної обробки. Типи: `CV_32FC1` для точних обчислень; `CV_8UC1` лише для явно налаштованих швидких режимів.

## Assumptions

Обробка може включати фільтрацію, нормалізацію, обчислення різниць і кореляцію.

## Theorem / Contract

Фотометричні операції не повинні непомітно виконуватися на `CV_8U`; такий режим потребує явного режиму в конфігурації `C`.

## Interpretation

Домен обробки є внутрішнім станом обчислень, а не вихідним протоколом.

## Failure cases

- Неявне зниження розрядності з raw до 8-bit.

## Typical misuse

- Трактувати debug-зображення як вхід для обробки.

## Open questions

- Які швидкі режими можуть дозволяти `CV_8UC1`.

## Connections

- used_by: dp1.stage.radiometric_correction
- used_by: dp1.stage.matched_filtering
