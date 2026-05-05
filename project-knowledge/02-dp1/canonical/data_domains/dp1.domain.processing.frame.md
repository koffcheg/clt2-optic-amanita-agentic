---
id: dp1.domain.processing.frame
title:
  uk: "Кадр у Processing домені DP1"
  en: "DP1 Processing-domain frame"
tags: [dp1, canonical, data-domain, processing, structure]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.processing.frame.md"
  lines: "1-N"
status: "draft"
---

## Definition

`ProcessingFrame` є canonical structure у Processing domain для кадру або frame-derived payload після одного чи кількох processing stages.

Він використовується, коли semantics вже відрізняються від Raw/Input `FramePacket`.

## Assumptions

- Processing representation може використовувати `U8`, `U16` або `F32` route залежно від configuration `C` і stage spec.
- OpenCV carrier не визначає semantics без explicit pixel format і processing domain.
- Code-level structure має бути підтверджена окремою implementation task.

## Theorem / Contract

`ProcessingFrame` має мінімально містити або посилатися на:

- `frame_id` — ідентичність кадру, узгоджена з `FramePacket`.
- `source_frame_ref` — optional reference на source `FramePacket`.
- `image` — processing payload/storage carrier.
- `pixel_format` — `U8`, `U16` або `F32`, defined by `dp1.domain.pixel_format`.
- `processing_domain` — semantic label, наприклад `radiometric_corrected`, `enhanced`, `detector_response`.
- `geometry` — width, height, coordinate origin policy.
- `range_policy` — interpretation of scalar range, if required by the route.

`ProcessingFrame` не має містити binary mask semantics, candidates, segments або measurements.

## Interpretation

Ця structure дозволяє одному алгоритмічному маршруту працювати з різною бітністю або normalized representation, якщо stage spec явно задає conversion/threshold/range rules.

## Failure cases

- Detector response записується як raw frame без processing semantics.
- `U8` і `U16` routes змішуються без explicit range policy.
- Binary mask передається як `ProcessingFrame`.

## Typical misuse

- Treating every `cv::Mat` between stages as equivalent processing object.
- Hiding algorithm outputs in processing metadata.

## Open questions

- Standard vocabulary for `processing_domain`.
- Exact normalization/range policy for `F32` detector response.
- Whether processing frames should be immutable after stage emission.

## Connections

- belongs_to: dp1.domain.processing
- derived_from: dp1.domain.raw.frame_packet
- uses: dp1.domain.pixel_format
- may_feed: dp1.stage.candidate_extraction
- may_produce: dp1.domain.mask.binary_mask
