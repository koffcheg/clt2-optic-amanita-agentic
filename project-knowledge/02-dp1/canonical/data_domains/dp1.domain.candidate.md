---
id: dp1.domain.candidate
title:
  uk: "Кандидат canonical DP1"
  en: "Canonical DP1 candidate domain"
tags: [dp1, canonical, data-domain, candidate]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.candidate.md"
  lines: "1-N"
status: "draft"
---

## Definition

`Candidate` є canonical DP1 data object для попередньої гіпотези про об'єкт після candidate extraction.

Candidate не є валідованим об'єктом і не є measurement.

## Assumptions

- Candidate може бути отриманий із connected components, thresholding або іншого candidate extraction route.
- Exact C++ representation is deferred to implementation tasks.
- Candidate confidence/score є optional, якщо stage spec не визначає його явно.

## Theorem / Contract

`Candidate` має мінімально містити або посилатися на:

- `candidate_id` — stable identifier у межах кадру або stage output.
- `frame_id` — кадр, з якого отримано candidate.
- `source_mask_ref` або `component_id` — походження з mask/component extraction.
- `bbox_px` — bounding box у processing-frame coordinates.
- `centroid_px` — optional centroid у pixel coordinates.
- `area_px` — площа candidate у pixels.
- `status` — `provisional` for MVP.
- optional `score` — detector/candidate score if defined by the stage spec.
- optional `quality_flags` — bounded flags, not free-form debug text.

Candidate має бути explicit object. Його не можна кодувати лише через mask pixels.

## Interpretation

Candidate є bridge між mask-level extraction і downstream segmentation/filtering. Він описує hypothesis geometry і мінімальні attributes, але не підтверджує існування об'єкта.

## Failure cases

- Candidate трактується як валідований object.
- Candidate geometry не узгоджена з source mask geometry.
- Candidate lacks `frame_id` or source relation.

## Typical misuse

- Передавати candidates як unstructured list of rectangles без identity/source metadata.
- Ховати candidate details у `FrameContext` замість explicit stage output.

## Open questions

- Exact `candidate_id` namespace.
- Required score/confidence semantics.
- Minimum geometry fields for MVP code.

## Connections

- derived_from: dp1.domain.mask
- feeds: dp1.stage.segmentation_refinement
- feeds: dp1.stage.object_filtering
- may_produce: dp1.domain.segment
