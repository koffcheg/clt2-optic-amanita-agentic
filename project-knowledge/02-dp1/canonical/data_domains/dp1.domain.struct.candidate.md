---
id: dp1.domain.struct.candidate
title:
  uk: "Кандидат у Struct домені DP1"
  en: "DP1 candidate structure"
tags: [dp1, canonical, data-domain, struct, candidate]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.struct.candidate.md"
status: "draft"
---

## Definition

`Candidate` є canonical structure у Struct domain для попередньої гіпотези про об'єкт після candidate extraction.

`Candidate` не є validated object і не є measurement.

## Assumptions

- `Candidate` може бути отриманий через connected components, thresholding або інший candidate extraction route.
- Конкретна C++ representation має визначатися окремою implementation task.
- `Candidate` confidence/score є optional, якщо stage spec не визначає його явно.

## Theorem / Contract

`Candidate` має мінімально містити або посилатися на:

- `candidate_id` — stable identifier у межах кадру або stage output.
- `frame_id` — кадр, з якого отримано candidate.
- `source_mask_ref` або `component_id` — походження з mask/component extraction.
- `bbox_px` — bounding box у processing-frame coordinates.
- `centroid_px` — optional centroid у pixel coordinates.
- `area_px` — площа candidate у pixels.
- `status` — `provisional` для MVP.
- optional `score` — detector/candidate score, якщо це визначено stage spec.
- optional `quality_flags` — bounded flags, а не free-form debug text.

`Candidate` має бути explicit object. Його не можна кодувати тільки через mask pixels.

## Interpretation

`Candidate` є bridge між mask-level extraction і downstream segmentation/filtering. Він описує hypothesis geometry і мінімальні attributes, але не підтверджує існування об'єкта.

## Failure cases

- `Candidate` трактується як validated object.
- Geometry candidate не узгоджена з source mask geometry.
- Candidate не має `frame_id` або source relation.

## Typical misuse

- Передавати candidates як unstructured list of rectangles без identity/source metadata.
- Ховати candidate details у `FrameContext` замість explicit stage output.

## Open questions

- Exact namespace для `candidate_id`.
- Required score/confidence semantics.
- Мінімальні geometry fields для MVP code.

## Connections

- belongs_to: dp1.domain.struct
- derived_from: dp1.domain.mask.binary_mask
- feeds: dp1.stage.segmentation_refinement
- feeds: dp1.stage.object_filtering
- may_produce: dp1.domain.struct.segment
