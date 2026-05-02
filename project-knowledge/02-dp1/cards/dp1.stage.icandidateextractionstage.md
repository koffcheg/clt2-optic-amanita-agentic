---
id: dp1.stage.ICandidateExtractionStage
title: ICandidateExtractionStage interface (canonical)
tags: [dp1, dp1_v2, stage-interface, candidate-extraction]
source:
  - project-knowledge/06-tasks/cards/AMNT-0009.md
  - project-knowledge/02-dp1/cards/dp1.stage.imeasurementstage.md
  - project-knowledge/02-dp1/cards/dp1.frame.frame_processor.md
  - project-knowledge/02-dp1/cards/dp1.preproc.binning_sum.md
status: draft
---

## Definition

`ICandidateExtractionStage` — stage-level interface DP1, який формує candidate artifacts із preprocessed frame domain для подальшої сегментації та фільтрації в downstream stages.

## Assumptions

- На phase parity `ICandidateExtractionStage` не змінює зовнішні result contracts `TDataRes` і RPC payload boundaries.
- Upstream frame/context metadata надходять у форматі, сумісному з поточним `frame_processor` path.
- Вихід stage є внутрішнім pipeline artifact і не є прямим network/file contract.

## Theorem / Contract

1. **Input contract (logical):**
   - frame image domain (оригінальний або preprocessed, включно з binning path за наявності);
   - frame/camera metadata snapshot, потрібний для геометричної/часової консистентності;
   - runtime options snapshot для extraction thresholds/modes.

2. **Output contract (logical):**
   - candidate set (regions/components/contour seeds) для `ISegmentationStage`;
   - optional per-candidate auxiliary descriptors (area/intensity/basic geometry), достатні для downstream decisioning;
   - stable mapping до frame context (frame index/timestamp/cam context).

3. **Compatibility contract:**
   - stage не вносить breaking assumptions у downstream `IMeasurementStage` щодо очікуваного candidate semantics;
   - без side-effects, що змінюють transport contracts DP1 -> DP2.

4. **Determinism / QoS contract:**
   - deterministic output для фіксованих input+config;
   - відсутність blocking network/disk I/O у hot path;
   - bounded memory growth для candidate buffers у межах runtime constraints DP1.

## Interpretation

`ICandidateExtractionStage` є першим downstream extraction boundary після frame preprocessing, який переводить pixel-level content у структурований candidate-level representation для наступних stage-інтерфейсів.

## Failure cases

- Некоректний/порожній frame input -> порожній candidate set із валідним frame envelope.
- Надмірний noise/illumination drift -> деградація candidate quality або over-generation, що має обмежуватись stage policy.
- Неконсистентний runtime config snapshot -> safe fallback або controlled empty output.

## Typical misuse

- Перенесення segmentation-level логіки у candidate extraction stage без явного contract update.
- Змішування candidate extraction із transport serialization або file output.
- Додавання debug/export I/O в hot path stage.

## Open questions

1. Який canonical candidate DTO є цільовим: contour-first, CC-first чи hybrid representation?
2. Які мінімальні mandatory descriptors повинні гарантуватися для downstream segmentation/filtering?
3. Де фіксувати policy для candidate cap/throttling під SLA pressure?

## Connections

- `dp1.stage.ISegmentationStage`
- `dp1.stage.IObjectFilterStage`
- `dp1.stage.IMeasurementStage`
- `dp1.frame.frame_processor`
- `dp1.preproc.binning_sum`
