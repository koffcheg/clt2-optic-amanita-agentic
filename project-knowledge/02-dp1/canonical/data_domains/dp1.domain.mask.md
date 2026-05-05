---
id: dp1.domain.mask
title:
  uk: "Mask домен DP1"
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

Mask domain описує semantic role масок у DP1: foreground/background, label або segmentation-related representations, які не є intensity images.

Конкретна MVP-структура бінарної маски описана окремою structure card `dp1.domain.mask.binary_mask`.

## Assumptions

- MVP carrier для binary mask — `MaskU8`, зазвичай `CV_8UC1`.
- Label masks можуть бути введені, коли stage specs потребуватимуть explicit component labels.
- Конкретна C++ representation має визначатися окремою implementation task.

## Theorem / Contract

Для Mask domain діють такі правила:

- mask values encode selection/classification, not photometric intensity;
- binary mask і label mask semantics мають бути explicit;
- mask geometry має бути прив'язана до source frame / processing representation;
- masks можуть подаватися в candidate extraction, segmentation, filtering і ROI-based measurement;
- masks не мають використовуватися як grayscale processing frames або visualization images.

Canonical MVP structure:

- `dp1.domain.mask.binary_mask`.

## Interpretation

Mask domain є переходом від pixel/response processing до structural hypotheses. Він не має переносити object identity сам по собі, якщо потрібні explicit `Candidate` або `Segment` structures.

## Failure cases

- Mask трактується як intensity image.
- Foreground convention неузгоджена між stages.
- Geometry mask відрізняється від source frame без explicit transform metadata.

## Typical misuse

- Передавати debug visualization як canonical mask.
- Кодувати candidate list тільки через mask pixels без explicit candidate objects.

## Open questions

- Чи всі binary masks мають використовувати `0/255`, чи окремі internal routes можуть використовувати `0/1`.
- Standard label mask carrier type для connected components output.

## Connections

- has_structure: dp1.domain.mask.binary_mask
- uses: dp1.domain.pixel_format
- may_produce: dp1.domain.struct.candidate
- may_produce: dp1.domain.struct.segment
- used_by: dp1.stage.candidate_extraction
- used_by: dp1.stage.segmentation_refinement
