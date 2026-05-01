---
id: protocols.dp1_dp2.measurement_handoff
title:
  uk: "Canonical-передача вимірювань DP1 -> DP2"
  en: "Canonical DP1 -> DP2 Measurement handoff"
tags: [protocols, dp1, dp2, canonical, measurement]
kind: protocol-card
source_role: canonical
source:
  file: "project-knowledge/04-protocols/cards/protocols.dp1_dp2.measurement_handoff.md"
  lines: "1-150"
status: "draft"
---

## Definition

Canonical-межа протоколу для передачі від DP1 до DP2.

## Assumptions

Ця картка визначає межу продукту, а не legacy-реалізацію транспорту.

## Theorem / Contract

DP1 передає DP2 дані з домену вимірювань.

Корисне навантаження вимірювань має містити смисли, потрібні downstream:
- ідентичність кадру та час;
- ідентичність камери або джерела;
- координати об’єкта;
- геометрію;
- фотометрію;
- metadata, потрібну для інтерпретації в DP2;
- систему координат і одиниці вимірювання, де це застосовно.

Canonical-протокол не повинен включати:
- зображення візуалізації;
- debug-зображення;
- внутрішні маски;
- тимчасові буфери обробки;
- буфери, що існують лише в legacy runtime.

Відповідальність DP1 завершується після формування canonical-виходу в домені вимірювань і доставки через погоджену межу передачі.

Відповідальність DP2 починається зі споживання та інтерпретації canonical-входу в домені вимірювань.

## Interpretation

Legacy TCP/RPC cards описують лише стару поведінку транспорту. Вони можуть допомагати аналізу міграції, але не є визначенням canonical-протоколу.

## Failure cases

- DP2 вимагає debug-артефакти DP1 як canonical-вхід.
- Legacy-серіалізація `cv::Mat` трактується як цільовий протокол.
- Корисне навантаження вимірювань не містить metadata координат або часу.

## Typical misuse

- Називати будь-який byte stream DP1 -> DP2 canonical-протоколом.

## Open questions

- Точна схема корисного навантаження.
- Політика версіонування і сумісності.
- Обов’язкова calibration metadata.
- Семантика помилок і часткових кадрів.

## Connections

- source_domain: dp1.domain.measurement
- consumed_by: dp2.input.measurement_domain
- legacy_reference: dp1.net.dp1_tr_res2dp2_connection
- legacy_reference: dp2.net.dp1_to_dp2_receive_path
