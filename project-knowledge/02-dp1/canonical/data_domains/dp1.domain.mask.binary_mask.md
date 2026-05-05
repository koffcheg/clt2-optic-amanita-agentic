---
id: dp1.domain.mask.binary_mask
title:
  uk: "Бінарна маска в Mask домені DP1"
  en: "DP1 binary mask structure"
tags: [dp1, canonical, data-domain, mask, structure]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.mask.binary_mask.md"
  lines: "1-N"
status: "draft"
---

## Definition

`BinaryMask` є canonical structure у Mask domain для foreground/background representation після thresholding, candidate extraction або segmentation-related stages.

## Assumptions

- MVP binary mask carrier is `MaskU8`, typically `CV_8UC1`.
- Label masks are a separate future or stage-specific structure.
- Exact C++ representation is deferred to implementation tasks.

## Theorem / Contract

For MVP, `BinaryMask` contract:

- carrier: `MaskU8`, typically `CV_8UC1`;
- background value: `0`;
- foreground value: `255` unless a stage spec explicitly defines another binary convention;
- geometry: same processing domain geometry as source frame unless remapped explicitly.

`BinaryMask` має містити або посилатися на:

- `frame_id`;
- `source_ref` — source `ProcessingFrame`, `Candidate`, або `Segment` relation;
- `pixel_format` — `MaskU8`;
- `geometry`;
- optional `foreground_convention` if different from `0/255`.

## Interpretation

`BinaryMask` не є grayscale processing image. It is a semantic object with mask-specific invariants.

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

- belongs_to: dp1.domain.mask
- uses: dp1.domain.pixel_format
- derived_from: dp1.domain.processing.frame
- may_produce: dp1.domain.struct.candidate
- used_by: dp1.stage.candidate_extraction
- used_by: dp1.stage.segmentation_refinement
