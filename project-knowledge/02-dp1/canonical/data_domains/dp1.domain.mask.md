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

- MVP binary mask carrier is `MaskU8`, typically `CV_8UC1`.
- Label masks may be introduced when stage specs require explicit component labels.
- Exact C++ representation is deferred to implementation tasks.

## Theorem / Contract

Mask domain має такі правила:

- mask values encode selection/classification, not photometric intensity;
- binary mask and label mask semantics must be explicit;
- mask geometry must be tied to its source frame/processing representation;
- masks may feed candidate extraction, segmentation, filtering, and ROI-based measurement;
- masks must not be used as grayscale processing frames or visualization images.

Canonical MVP structure:

- `dp1.domain.mask.binary_mask`.

## Interpretation

Mask domain є переходом від pixel/response processing до structural hypotheses. It should not carry object identity by itself when explicit Candidate or Segment structures are required.

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

- has_structure: dp1.domain.mask.binary_mask
- uses: dp1.domain.pixel_format
- may_produce: dp1.domain.struct.candidate
- may_produce: dp1.domain.struct.segment
- used_by: dp1.stage.candidate_extraction
- used_by: dp1.stage.segmentation_refinement
