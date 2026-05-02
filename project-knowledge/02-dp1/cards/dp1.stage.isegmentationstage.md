---
id: dp1.stage.ISegmentationStage
title: ISegmentationStage interface (canonical)
tags: [dp1, dp1_v2, stage-interface, segmentation]
source:
  - project-knowledge/06-tasks/cards/AMNT-0009.md
  - project-knowledge/02-dp1/cards/dp1.stage.imeasurementstage.md
status: draft
---

## Definition

`ISegmentationStage` — stage-level interface DP1, який перетворює candidate artifacts у segmentation-resolved object artifacts, придатні для policy filtering і measurement extraction.

## Assumptions

- Segmentation artifacts залишаються внутрішніми для DP1 pipeline і не є прямим RPC/file contract.
- Stage output має бути сумісним з downstream очікуваннями `IObjectFilterStage` та `IMeasurementStage`.
- Під час parity phase зміни не повинні вимагати змін у `TDataRes` layout.

## Theorem / Contract

1. **Input contract (logical):**
   - candidate set з `ICandidateExtractionStage`;
   - frame/context metadata для просторово-часової консистентності;
   - runtime segmentation options/profile.

2. **Output contract (logical):**
   - segmentation-resolved object set (mask/contour/components with stable identity within frame scope);
   - geometry/stat descriptors, достатні для downstream filtering;
   - quality/diagnostic attributes (за наявності policy), які не змінюють external payload contract.

3. **Compatibility contract:**
   - stage не змінює semantics existing measurement fields неявним чином;
   - downstream filtering/measurement stages отримують передбачувану, schema-stable логічну форму даних.

4. **Determinism / QoS contract:**
   - deterministic mapping candidate -> segmentation result для фіксованих input+config;
   - контрольована складність обчислень у рамках frame budget;
   - відсутність blocking external I/O у stage hot path.

## Interpretation

`ISegmentationStage` — це boundary, де candidate-level припущення уточнюються до object-level сегментів із геометричними властивостями, на яких базуються downstream policy decisions.

## Failure cases

- Порожній candidate set -> порожній segmentation set без порушення frame-level envelope.
- Degenerate geometry (self-intersections/noise fragments) -> reject або safe normalization згідно stage policy.
- Конфлікт runtime profile/config -> controlled fallback mode або empty output.

## Typical misuse

- Використання stage як місця для бізнес-policy фільтрації замість геометричної сегментації.
- Залежність від нестабільних debug-only атрибутів як контракту downstream stages.
- Непрозоре змішування coordinate spaces без явного conversion policy.

## Open questions

1. Яка canonical форма segmentation artifact: mask-first чи contour-first?
2. Чи потрібен обов'язковий confidence score на рівні segment для downstream filter contracts?
3. Які exact invariants мають гарантуватися між segmentation output і measurement input?

## Connections

- `dp1.stage.ICandidateExtractionStage`
- `dp1.stage.IObjectFilterStage`
- `dp1.stage.IMeasurementStage`
