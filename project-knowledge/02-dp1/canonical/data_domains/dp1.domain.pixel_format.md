---
id: dp1.domain.pixel_format
title:
  uk: "Формати пікселів canonical DP1"
  en: "Canonical DP1 pixel formats"
tags: [dp1, canonical, data-domain, pixel-format]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.pixel_format.md"
  lines: "1-N"
status: "draft"
---

## Definition

Картка визначає canonical vocabulary форматів пікселів для DP1 data domains.

Формат пікселів є властивістю route/config, а не властивістю алгоритму. Stage spec не має зашивати 8-bit або 16-bit поведінку без явного посилання на цей домен і configuration `C`.

## Assumptions

- OpenCV type може бути storage carrier, але semantic format визначається цією карткою і відповідним data-domain contract.
- Фактична підтримка конкретного формату в коді має перевірятися через implementation artifacts.

## Theorem / Contract

Canonical DP1 має мінімально розрізняти такі формати:

- `U8` — unsigned 8-bit grayscale carrier, зазвичай `CV_8UC1`.
- `U16` — unsigned 16-bit grayscale carrier, зазвичай `CV_16UC1`.
- `F32` — single-channel floating processing/response carrier, зазвичай `CV_32FC1`.
- `MaskU8` — carrier для бінарної маски, зазвичай `CV_8UC1`.

Формат має бути явно заданий у data object або route-level metadata. Неявне виведення semantics тільки з `cv::Mat::type()` недостатнє для canonical contract.

## Interpretation

`U8` і `U16` дозволяють запускати той самий stage route на різній бітності, якщо stage spec і config `C` явно визначають threshold/scale/range policy.

`F32` використовується для normalized або detector-response представлень, коли алгоритму потрібен scalar domain, що не є raw pixel domain.

`MaskU8` не є intensity image. Це domain-specific carrier для mask semantics.

## Failure cases

- Алгоритм трактує `CV_8UC1` і `CV_16UC1` однаково без scale/range policy.
- Stage spec описує threshold як абсолютне число без прив'язки до pixel format.
- Бінарна маска трактується як grayscale processing frame.

## Typical misuse

- Вважати OpenCV type повним domain contract.
- Зашивати 8-bit або 16-bit поведінку в алгоритм замість route/config selection.

## Open questions

- Єдина canonical threshold range policy для `U8`, `U16` і `F32`.
- Чи потрібні окремі packed/color formats поза MVP.

## Connections

- constrains: dp1.domain.raw.frame_packet
- constrains: dp1.domain.processing.frame
- constrains: dp1.domain.mask.binary_mask
- constrained_by: dp1.config.pipeline_configuration_c
