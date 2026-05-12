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
status: "draft"
---

## Definition

Raw/Input domain описує початкові сенсорні або source-equivalent дані, що входять у DP1 pipeline до обчислювальної обробки.

Домен визначає semantic role даних. Конкретний контейнер кадру описаний окремою structure card `dp1.domain.raw.frame_packet`.

## Assumptions

- Canonical high-quality route очікує, що raw dynamic range зберігається до явного переходу в Processing domain.
- Raw carrier може бути `U16` або `U8` залежно від source/configuration route, але selected pixel format має бути explicit.
- Runtime claims про фактично підтримані camera/source formats мають перевірятися за code/config artifacts.

## Theorem / Contract

Для Raw/Input domain діють такі правила:

- raw data є source/acquisition-level input, а не debug visualization;
- raw dynamic range не має зменшуватися неявно;
- перехід із Raw/Input у Processing має бути явним, profileable і належати конкретному stage;
- raw data може використовуватися Measurement stage для photometry, якщо це вимагає stage spec;
- Raw/Input objects не мають містити masks, candidates, segments, measurements або tile-local working buffers.

Дозволена canonical pixel-format vocabulary визначена в `dp1.domain.pixel_format`.

## Interpretation

Raw/Input domain є джерелом фізичних або source-equivalent intensity data. Він відокремлений від Processing domain, де живуть filtering, residuals, detector responses і normalized representations.

## Failure cases

- Раннє неявне перетворення у `CV_8U`.
- Прихована нормалізація перед формуванням вимірювань.
- Raw data перезаписується processing output.
- Algorithm-specific outputs записуються в raw metadata.

## Typical misuse

- Виконувати фотометричні обчислення після перетворення з втратою даних.
- Трактувати display/debug buffers як raw input.

## Open questions

- Exact sensor bit-depth variants для production camera sources.
- Чи `U8` raw/source route допускається для production input, чи тільки для
  compatibility/test data.
- Raw buffer ownership and lifetime policy.

## Connections

- has_structure: dp1.domain.raw.frame_packet
- uses: dp1.domain.pixel_format
- feeds: dp1.stage.prep
- may_feed: dp1.stage.measurement
