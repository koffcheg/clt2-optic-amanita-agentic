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
status: "draft"
---

## Definition

`BinaryMask` є canonical structure у Mask domain для foreground/background representation після thresholding, candidate extraction або segmentation-related stages.

## Assumptions

- MVP carrier для binary mask — `MaskU8`, зазвичай `CV_8UC1`.
- Label masks мають бути окремою future або stage-specific structure.
- Конкретна C++ representation має визначатися окремою implementation task.

## Theorem / Contract

Для MVP `BinaryMask` має такий contract:

- carrier: `MaskU8`, зазвичай `CV_8UC1`;
- background value: `0`;
- foreground value: `255`, якщо stage spec явно не визначає іншу binary convention;
- geometry: така сама, як у source frame / processing representation, якщо remapping не заданий явно.

`BinaryMask` має мінімально містити або посилатися на:

- `frame_id`;
- `source_ref` — relation до source `ProcessingFrame`, `Candidate` або `Segment`;
- `pixel_format` — `MaskU8`;
- `geometry`;
- optional `foreground_convention`, якщо використовується не `0/255`.

## Interpretation

`BinaryMask` не є grayscale processing image. Це semantic object із mask-specific invariants.

Binary masks можуть використовуватись для connected components, segmentation, filtering і measurement-related ROI extraction.

## Failure cases

- Mask трактується як intensity image.
- Foreground convention відрізняється між stages без явного contract.
- Geometry mask відрізняється від source frame без transform metadata.

## Typical misuse

- Передавати debug visualization як canonical mask.
- Кодувати candidate list тільки через mask pixels без explicit candidate objects.

## Open questions

- Чи всі binary masks мають використовувати `0/255`, чи окремі internal routes можуть використовувати `0/1`.
- Standard label mask carrier type для connected components output.

## Connections

- belongs_to: dp1.domain.mask
- uses: dp1.domain.pixel_format
- derived_from: dp1.domain.processing.frame
- may_produce: dp1.domain.struct.candidate
- used_by: dp1.stage.candidate_extraction
- used_by: dp1.stage.segmentation_refinement
