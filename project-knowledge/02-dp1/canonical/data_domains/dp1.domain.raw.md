---
id: dp1.domain.raw
title:
  uk: "Raw/Input домен DP1"
  en: "DP1 Raw/Input domain"
tags: [dp1, canonical, data-domain, raw]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.raw.md"
  lines: "1-N"
status: "draft"
---

## Definition

Raw/Input domain описує початкові сенсорні або source-equivalent дані, що входять у DP1 pipeline до обчислювальної обробки.

Домен визначає semantic role даних. Конкретний контейнер кадру описаний окремою structure card `dp1.domain.raw.frame_packet`.

## Assumptions

- Canonical high-quality route expects raw dynamic range to be preserved until explicit conversion into Processing domain.
- Raw carrier may be `U16` or `U8` depending on source/configuration route, but the selected pixel format must be explicit.
- Runtime claims about actual supported camera/source formats must be verified against code/config artifacts.

## Theorem / Contract

Raw/Input domain має такі правила:

- raw data is source/acquisition-level input, not debug visualization;
- raw dynamic range must not be reduced silently;
- conversion from Raw/Input to Processing must be explicit, profileable, and stage-owned;
- raw data may be used by Measurement stage for photometry if the stage spec requires it;
- Raw/Input objects must not contain masks, candidates, segments, measurements, or tile-local working buffers.

Allowed canonical pixel-format vocabulary is defined by `dp1.domain.pixel_format`.

## Interpretation

Raw/Input domain is the source of physical or source-equivalent intensity data. It is separate from Processing domain, where filtering, residuals, detector responses, and normalized representations live.

## Failure cases

- Раннє неявне перетворення у `CV_8U`.
- Прихована нормалізація перед формуванням вимірювань.
- Raw data is overwritten by processing output.
- Algorithm-specific outputs are stored in raw metadata.

## Typical misuse

- Виконувати фотометричні обчислення після перетворення з втратою даних.
- Treating display/debug buffers as raw input.

## Open questions

- Exact sensor bit-depth variants for production camera sources.
- Whether `U8` raw/source route is accepted only for MVP/test data or also for production.
- Raw buffer ownership and lifetime policy.

## Connections

- has_structure: dp1.domain.raw.frame_packet
- uses: dp1.domain.pixel_format
- feeds: dp1.stage.prep
- may_feed: dp1.stage.measurement
