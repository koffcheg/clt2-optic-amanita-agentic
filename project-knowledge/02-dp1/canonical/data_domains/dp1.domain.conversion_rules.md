---
id: dp1.domain.conversion_rules
title:
  uk: "Правила перетворення доменів даних DP1"
  en: "DP1 data-domain conversion rules"
tags: [dp1, canonical, data-domain]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.conversion_rules.md"
  lines: "1-120"
status: "draft"
---

## Definition

Canonical-правила переходів між доменами DP1.

## Assumptions

Кожен етап декларує вхідний тип, внутрішній тип і вихідний тип.

## Theorem / Contract

- Неявні перетворення типів заборонені.
- Фотометричні операції на `CV_8U` заборонені, якщо окремий режим не задекларовано явно.
- Динамічний діапазон raw frame має зберігатися щонайменше до формування вимірювань.
- Переходи типів мають бути явними і врахованими в моделі часу/продуктивності.
- 8-bit tract дозволений лише як окремий режим у конфігурації `C`.

## Interpretation

Перетворення є архітектурними рішеннями, а не випадковими деталями реалізації.

## Failure cases

- Перетворення з втратою даних перед вимірюванням.
- Приховане перетворення всередині викликів OpenCV helper functions.

## Typical misuse

- Вважати тип `cv::Mat` самодокументованим.

## Open questions

- Canonical-формат аудиту перетворень.

## Connections

- constrains: dp1.pipeline.stage_contract
- constrains: dp1.config.pipeline_configuration_c
