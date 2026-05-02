---
id: dp1.stage.IObjectFilterStage
title: IObjectFilterStage interface (canonical)
tags: [dp1, dp1_v2, stage-interface, object-filter]
source:
  - project-knowledge/06-tasks/cards/AMNT-0009.md
  - project-knowledge/02-dp1/cards/dp1.stage.imeasurementstage.md
  - project-knowledge/02-dp1/cards/dp1.types.TOptionsMeasurement.md
  - project-knowledge/02-dp1/cards/dp1.types.TDataRes.md
status: draft
---

## Definition

`IObjectFilterStage` — stage-level interface DP1, який застосовує policy-driven acceptance/rejection до segmentation-resolved objects перед передачею у measurement extraction boundary.

## Assumptions

- Фільтрація виконується в межах одного кадра (frame-local semantics) без міжкадрового stateful tracking.
- Stage не модифікує external transport contracts напряму; він керує лише складом downstream object set.
- Acceptance policy має бути сумісною з поточними очікуваннями `IMeasurementStage`.

## Theorem / Contract

1. **Input contract (logical):**
   - segmentation-resolved object set з `ISegmentationStage`;
   - frame/context metadata і runtime filter profile;
   - optional quality/stat descriptors, потрібні для policy evaluation.

2. **Output contract (logical):**
   - filtered object set, який допускається до `IMeasurementStage`;
   - explicit reject outcomes (reason classes/logical categories) за наявності policy;
   - збереження frame-level consistency для downstream packaging в `TDataRes`.

3. **Compatibility contract:**
   - stage не порушує очікувану semantics measurement-level fields у `TOptionsMeasurement`/`TDataRes`;
   - допустимі only non-breaking policy refinements на parity phase.

4. **Determinism / QoS contract:**
   - deterministic accept/reject decision для фіксованих input+config;
   - bounded per-object evaluation cost у межах frame SLA;
   - відсутність blocking external I/O у stage hot path.

## Interpretation

`IObjectFilterStage` є policy gate між segmentation і measurement: він обмежує набір об'єктів до контрактно-прийнятного, знижуючи downstream noise та зберігаючи сумісність із результатним DP1 payload.

## Failure cases

- Неконсистентні або неповні descriptors -> safe reject або fallback policy path.
- Over-strict thresholds -> деградація recall (аж до empty output), що має бути контрольованим і прозорим.
- Невалідний filter profile -> controlled fallback profile або empty pass-through за явно визначеною policy.

## Typical misuse

- Підміна object filtering бізнес-логікою, яка повинна жити в DP2/післяпроцесингу.
- Неявна зміна unit/coordinate semantics перед measurement stage.
- Інтеграція side-effect logging/export як обов'язкової частини stage contract.

## Open questions

1. Який canonical набір reject-reason categories потрібен для трасування якості?
2. Чи потрібен soft-filter режим (annotate-only) для валідаційних прогонів?
3. Де фіксується межа між geometric cleanup (`ISegmentationStage`) і policy filtering (`IObjectFilterStage`)?

## Connections

- `dp1.stage.ICandidateExtractionStage`
- `dp1.stage.ISegmentationStage`
- `dp1.stage.IMeasurementStage`
- `dp1.types.TOptionsMeasurement`
- `dp1.types.TDataRes`
