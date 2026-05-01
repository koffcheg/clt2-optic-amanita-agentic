---
id: dp2.input.measurement_domain
title:
  uk: "Canonical-вхід вимірювань DP2"
  en: "Canonical DP2 Measurement input"
tags: [dp2, canonical, input, measurement]
kind: protocol-card
source_role: canonical
source:
  file: "project-knowledge/03-dp2/canonical/input_contracts/dp2.input.measurement_domain.md"
  lines: "1-110"
status: "draft"
---

## Definition

Canonical-вхід DP2 починається з домену вимірювань DP1 через canonical-передачу DP1 -> DP2.

## Assumptions

Точна схема корисного навантаження DP2 все ще відкрита.

## Theorem / Contract

DP2 не повинен залежати від візуалізації DP1, debug-зображень, внутрішніх масок або тимчасових буферів обробки як від canonical-входу.

## Interpretation

Ця картка прив’язує DP2 до shared protocol layer.

## Failure cases

DP2 споживає legacy transport details як цільову архітектуру.

## Typical misuse

Трактувати схему десеріалізації як модель домену.

## Open questions

Canonical-правила нормалізації входу DP2.

## Connections

- uses: protocols.dp1_dp2.measurement_handoff
- consumes: dp1.domain.measurement
