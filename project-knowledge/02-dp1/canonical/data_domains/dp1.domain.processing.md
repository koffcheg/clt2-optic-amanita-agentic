---
id: dp1.domain.processing
title:
  uk: "Processing домен DP1"
  en: "DP1 Processing domain"
tags: [dp1, canonical, data-domain, processing]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.processing.md"
status: "draft"
---

## Definition

Processing domain описує canonical DP1 data object для кадру або похідного від кадру payload після одного чи кількох processing stages, коли semantics вже відрізняються від raw/input `FramePacket`.

Цей домен відповідає на питання: у якому представленні кадр обробляється всередині DP1 перед mask/candidate/measurement stages.

## Assumptions

- Canonical processing representation за замовчуванням використовує `F32` /
  `CV_32FC1`, як визначено у `dp1.config.pipeline_configuration_c`,
  `dp1.domain.pixel_format` і `dp1.domain.processing.frame`.
- `S16` і `S32` дозволені тільки як explicit signed residual route.
- `U8` і `U16` у Processing domain дозволені тільки як explicit
  fast/compatibility route, якщо це прямо дозволено configuration `C`,
  stage-interface card або stage spec.
- OpenCV carrier не визначає semantics без explicit pixel format і processing domain.
- Code-level structure має бути підтверджена окремою implementation task.

## Theorem / Contract

Processing data object має мінімально містити або посилатися на:

- `frame_id` — ідентичність кадру, узгоджена з `FramePacket`.
- `source_frame_ref` — optional reference на source `FramePacket`.
- `image` — processing payload/storage carrier.
- `pixel_format` — default `F32`; `S16` або `S32` тільки для explicit signed
  residual route; `U8` або `U16` тільки для explicit fast/compatibility route,
  defined by `dp1.domain.pixel_format`.
- `processing_domain` — semantic label, наприклад `radiometric_corrected`, `enhanced`, `detector_response`.
- `geometry` — width, height, coordinate origin policy.
- `range_policy` — interpretation of scalar range, якщо це потрібно для route.

Фотометричні операції не повинні непомітно виконуватися на `CV_8U`; такий режим потребує явного route/config у `C`.

Processing domain не має містити binary mask semantics, candidates, segments або measurements.

## Interpretation

Цей домен дозволяє одному algorithm route працювати з різною бітністю або normalized representation, якщо stage spec явно задає conversion/threshold/range rules.

Processing domain є внутрішнім станом обчислень, а не вихідним протоколом.

## Failure cases

- Неявне зниження розрядності з raw до 8-bit.
- Detector response записується як raw frame без processing semantics.
- `U8` і `U16` routes змішуються без explicit range policy.
- Binary mask передається як processing domain object.
- Processing domain card використовується як дозвіл на `U8/U16` processing без
  explicit route у `C`.

## Typical misuse

- Трактувати debug-зображення як вхід для обробки.
- Трактувати кожен `cv::Mat` між stages як еквівалентний processing object.
- Ховати algorithm outputs у processing metadata.

## Open questions

- Які fast modes можуть дозволяти `CV_8UC1`.
- Standard vocabulary for `processing_domain`.
- Exact normalization/range policy for `F32` detector response.
- Чи processing data objects мають бути immutable після stage emission.

## Connections

- derived_from: dp1.domain.raw.frame_packet
- uses: dp1.domain.pixel_format
- uses: dp1.domain.processing.frame
- constrained_by: dp1.config.pipeline_configuration_c
- constrained_by: dp1.domain.opencv_invariants
- used_by: dp1.stage.radiometric_correction
- used_by: dp1.stage.matched_filtering
- may_feed: dp1.stage.candidate_extraction
- may_produce: dp1.domain.mask.binary_mask
