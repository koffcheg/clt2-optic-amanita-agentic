---
id: dp1.canonical.product_definition
title:
  uk: "Canonical DP1 як інженерний продукт"
  en: "Canonical DP1 as an engineering product"
tags: [dp1, canonical, product]
kind: governance-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/product/dp1.canonical.product_definition.md"
  lines: "1-120"
status: "draft"
---

## Definition

Canonical DP1 - це інженерний продукт, визначений через:
- формальну модель конвеєра `Π`;
- простір реалізацій `{V_i,j}`;
- конфігурацію конвеєра `C`;
- формалізований набір `Cards + Stage_Spec`.

## Assumptions

Legacy DP1 є лише довідковим описом старої поведінки під час виконання. Він не є цільовою архітектурою.

## Theorem / Contract

DP1 має забезпечувати керовану варіативність обробки, контроль обчислювальної складності, відтворюваність результатів, готовність до генерації коду та вихід у домені вимірювань для DP2.

Canonical DP1 не можна реалізовувати лише з неформального опису.

## Interpretation

DP1 є конфігурованою системою обробки. Його архітектура визначається доменами даних, інтерфейсами етапів, конфігурацією, межами протоколу та правилами перевірки відповідності.

## Failure cases

- Legacy-конвеєр трактується як цільова архітектура.
- Код етапу генерується без специфікації етапу.
- Буфери візуалізації використовуються як джерело обчислень.

## Typical misuse

- Читати `legacy/` першим під час розробки canonical DP1.
- Трактувати OpenCV як архітектуру, а не як бібліотеку низькорівневих примітивів.

## Open questions

- Точна схема корисного навантаження вимірювань.
- Повні специфікації етапів.

## Connections

- uses: dp1.pipeline.formal_model
- uses: dp1.config.pipeline_configuration_c
- produces: protocols.dp1_dp2.measurement_handoff
