---
id: dp1.domain.segment
title:
  uk: "Сегмент canonical DP1"
  en: "Canonical DP1 segment domain"
tags: [dp1, canonical, data-domain, segment]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.segment.md"
  lines: "1-N"
status: "draft"
---

## Definition

`Segment` є canonical DP1 data object для уточненої області, отриманої після segmentation або segmentation refinement.

Segment уточнює candidate geometry/shape, але сам по собі ще не є final measurement.

## Assumptions

- Segment may reference a source `Candidate` and/or source `Mask`.
- Segment representation may be mask ROI, contour, or bounded geometry depending on stage spec.
- Exact C++ representation is deferred to implementation tasks.

## Theorem / Contract

`Segment` має мінімально містити або посилатися на:

- `segment_id` — stable identifier у межах кадру або stage output.
- `frame_id` — кадр, якому належить segment.
- `candidate_id` — optional source candidate relation.
- `source_mask_ref` — optional source mask relation.
- `bbox_px` — bounding box у processing-frame coordinates.
- `area_px` — area of segmented region.
- `shape_ref` — mask ROI, contour, або equivalent bounded representation.
- `quality_flags` — bounded flags about segmentation quality.

Segment must not contain final measurement payload intended for DP2 handoff.

## Interpretation

Segment є проміжним domain object між candidate-level hypotheses і measurement-level output. Він може бути використаний object filtering або measurement stage.

## Failure cases

- Segment loses relation to source candidate/frame.
- Segment geometry uses a different coordinate system without metadata.
- Measurement fields are mixed into segment output.

## Typical misuse

- Treating segment as validated object or final measurement.
- Encoding segment only as debug visualization.

## Open questions

- Canonical shape representation for MVP: ROI mask, contour, or both.
- Required segmentation quality flags.
- Whether multi-component segments are allowed.

## Connections

- derived_from: dp1.domain.candidate
- derived_from: dp1.domain.mask
- used_by: dp1.stage.object_filtering
- used_by: dp1.stage.measurement
- may_produce: dp1.domain.measurement
