---
id: dp1.domain.mask
title:
  uk: "Домен масок DP1"
  en: "DP1 Mask domain"
tags: [dp1, canonical, data-domain, mask]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.mask.md"
  lines: "1-N"
status: "draft"
---

## Definition

`Mask` є canonical DP1 data object для binary або label representation, що використовується у candidate extraction, segmentation і downstream filtering.

## Assumptions

- MVP binary mask carrier is `MaskU8`, typically `CV_8UC1`.
- Label masks may be introduced when stage specs require explicit component labels.
- Exact C++ representation is deferred to implementation tasks.

## Theorem / Contract

Canonical DP1 має мінімально розрізняти:

- `binary_mask` — foreground/background mask.
- `label_mask` — optional integer component labels when exposed by a stage.

For MVP, `binary_mask` contract:

- carrier: `MaskU8`, typically `CV_8UC1`;
- background value: `0`;
- foreground value: `255` unless a stage spec explicitly defines another binary convention;
- geometry: same processing domain geometry as source frame unless remapped explicitly.

`Mask` має містити або посилатися на:

- `frame_id`;
- `source_ref` — source `ProcessingFrame`, `Candidate`, або `Segment` relation;
- `mask_kind` — `binary_mask` або `label_mask`;
- `pixel_format` — `MaskU8` for MVP binary masks;
- `geometry`;
- optional `component_count` for label/component-derived masks.

## Interpretation

`Mask` не є grayscale processing image. It is a semantic object with mask-specific invariants.

Binary masks can feed connected components, segmentation, filtering, and measurement-related ROI extraction.

## Failure cases

- Mask is treated as intensity image.
- Foreground convention is inconsistent across stages.
- Mask geometry differs from source frame without explicit transform metadata.

## Typical misuse

- Passing debug visualization as canonical mask.
- Encoding candidate list only through mask pixels without explicit candidate objects.

## Open questions

- Whether all binary masks must use `0/255` or may use `0/1` in selected internal routes.
- Standard label mask carrier type for connected components output.

## Connections

- uses: dp1.domain.pixel_format
- derived_from: dp1.domain.processing
- may_produce: dp1.domain.candidate
- may_produce: dp1.domain.segment
- used_by: dp1.stage.candidate_extraction
- used_by: dp1.stage.segmentation_refinement
