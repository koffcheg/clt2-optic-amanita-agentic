---
id: dp1.domain.struct.segment
title:
  uk: "Сегмент у Struct домені DP1"
  en: "DP1 segment structure"
tags: [dp1, canonical, data-domain, struct, segment]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.struct.segment.md"
  lines: "1-N"
status: "draft"
---

## Definition

`Segment` є canonical structure у Struct domain для уточненої області, отриманої після segmentation або segmentation refinement.

`Segment` уточнює geometry/shape candidate, але сам по собі ще не є final measurement.

## Assumptions

- `Segment` може посилатися на source `Candidate` і/або source `BinaryMask`.
- Segment representation може бути mask ROI, contour або bounded geometry залежно від stage spec.
- Конкретна C++ representation має визначатися окремою implementation task.

## Theorem / Contract

`Segment` має мінімально містити або посилатися на:

- `segment_id` — stable identifier у межах кадру або stage output.
- `frame_id` — кадр, якому належить segment.
- `candidate_id` — optional relation до source candidate.
- `source_mask_ref` — optional relation до source mask.
- `bbox_px` — bounding box у processing-frame coordinates.
- `area_px` — площа segment region.
- `shape_ref` — mask ROI, contour або equivalent bounded representation.
- `quality_flags` — bounded flags для оцінки якості segmentation.

`Segment` не має містити final measurement payload, призначений для DP2 handoff.

## Interpretation

`Segment` є проміжною structure між candidate-level hypotheses і measurement-level output. Він може використовуватися на етапах object filtering або measurement.

## Failure cases

- `Segment` втрачає relation до source candidate/frame.
- Segment geometry використовує іншу coordinate system без metadata.
- Measurement fields змішуються з segment output.

## Typical misuse

- Трактувати segment як validated object або final measurement.
- Кодувати segment тільки як debug visualization.

## Open questions

- Canonical shape representation для MVP: ROI mask, contour або обидва варіанти.
- Required segmentation quality flags.
- Чи дозволені multi-component segments.

## Connections

- belongs_to: dp1.domain.struct
- derived_from: dp1.domain.struct.candidate
- derived_from: dp1.domain.mask.binary_mask
- used_by: dp1.stage.object_filtering
- used_by: dp1.stage.measurement
- may_produce: dp1.domain.measurement.record
