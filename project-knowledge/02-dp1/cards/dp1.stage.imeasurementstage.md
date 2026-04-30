---
id: dp1.stage.IMeasurementStage
title: IMeasurementStage interface (canonical)
tags: [dp1, dp1_v2, stage-interface, measurement]
source:
  - project-knowledge/06-tasks/cards/AMNT-0007.md
  - project-knowledge/06-tasks/cards/AMNT-0008.md
  - project-knowledge/02-dp1/cards/dp1.types.TOptionsMeasurement.md
  - project-knowledge/02-dp1/cards/dp1.types.TDrawMeasurement.md
  - project-knowledge/02-dp1/cards/dp1.types.TDataRes.md
  - project-knowledge/02-dp1/cards/dp1.rpc.serialize_dp1_res.md
  - project-knowledge/02-dp1/cards/dp1.net.dp1_tr_res2dp2_connection.md
status: canonical
---

## Definition

`IMeasurementStage` — stage-level interface DP1, який перетворює candidate/segmentation artifacts у measurement-level output, сумісний з `TOptionsMeasurement`, `TDrawMeasurement`, `TDataRes` і downstream boundary DP1 -> DP2.

## Assumptions

- На фазі parity `IMeasurementStage` не змінює payload schema `TDataRes`.
- Canonical input DTO shape наразі не зафіксована окремою card; використовується припущення «segmentation-derived candidates + metadata».
- Exact per-field semantics для measurement attributes мають залишатися сумісними з поточними type/RPC cards.

## Theorem / Contract

1. **Input contract (logical):**
   - frame/context metadata, узгоджені з чинними DP1 structures;
   - segmentation/candidate artifacts (mask/contours/components залежно від upstream implementation);
   - runtime options snapshot (thresholds/filters/profile).

2. **Output contract (logical):**
   - measurement-level output, сумісний із `TDataRes`;
   - optional draw geometry, сумісна з `TDrawMeasurement`;
   - packing-ready payload fragment, сумісний з поточним result packaging flow.

3. **Compatibility contract:**
   - без breaking changes для binary expectations `serialize_dp1_res` на фазі parity;
   - `send_res_to_dp2` boundary зберігає зовнішню integration semantics без змін.

4. **Determinism / QoS contract:**
   - deterministic behavior для фіксованих input+config;
   - відсутність blocking network/disk I/O у stage hot path;
   - контрольовані алокації памʼяті у hot path відповідно до DP1_v2 performance constraints.

## Interpretation

`IMeasurementStage` є мостом між image-space candidates і protocol-stable object measurements, який замикає stage-driven pipeline у форматі, сумісному з DP2 ingest boundary.

## Failure cases

- Порожні або невалідні candidate artifacts -> порожній measurement set із валідним frame envelope.
- Неконсистентні geometry/area descriptors -> candidate reject або safe default mapping згідно policy.
- SLA pressure/overload -> partial/empty output дозволений тільки за явно визначеною QoS policy (неявні дропи заборонені).

## Typical misuse

- Змішування measurement extraction з transport-side serialization в одному компоненті.
- Додавання GUI/debug I/O в measurement hot path.
- Неявна зміна layout або semantics `TOptionsMeasurement` без versioning policy.

## Open questions

1. Який canonical input DTO для `IMeasurementStage`: contour-first, CC-first чи hybrid?
2. Які exact acceptance/rejection criteria на межі `IObjectFilterStage` -> `IMeasurementStage`?
3. Чи потрібне explicit confidence/quality поле, якщо це впливає на layout `TOptionsMeasurement`?
4. Де фіксується межа відповідальності між `ICandidateExtractionStage` і `IMeasurementStage` для derived metrics?

## Connections

- `dp1.types.TOptionsMeasurement`
- `dp1.types.TDrawMeasurement`
- `dp1.types.TDataRes`
- `dp1.rpc.serialize_dp1_res`
- `dp1.net.dp1_tr_res2dp2_connection`
- `dp1.frame.frame_processor`
