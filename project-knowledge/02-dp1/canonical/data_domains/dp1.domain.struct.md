---
id: dp1.domain.struct
title:
  uk: "Struct домен DP1"
  en: "DP1 Struct domain"
tags: [dp1, canonical, data-domain, struct]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.struct.md"
  lines: "1-N"
status: "draft"
---

## Definition

Struct domain описує semantic role структурованих проміжних об'єктів DP1 після mask-level extraction і перед final measurement.

Цей домен містить explicit structures для hypotheses, refined regions, object-like records, and filtering outputs.

## Assumptions

- Struct domain objects are not final DP1 -> DP2 measurement payloads.
- Exact C++ representations are deferred to implementation tasks.
- Candidate and Segment are MVP structures in this domain.

## Theorem / Contract

Struct domain має такі правила:

- objects must have explicit identity and source relation;
- geometry must declare coordinate system or source frame relation;
- candidates are provisional hypotheses, not validated objects;
- segments are refined regions, not final measurements;
- filtering decisions must not erase source traceability unless explicitly defined by a stage spec.

Canonical MVP structures:

- `dp1.domain.struct.candidate`.
- `dp1.domain.struct.segment`.

## Interpretation

Struct domain is the bridge from pixel/mask semantics to measurement semantics. It prevents the pipeline from encoding all object state in masks or unstructured lists.

## Failure cases

- Candidate or segment is treated as final measurement.
- Object geometry loses relation to frame/source mask.
- Stage passes unstructured rectangles without identity/source metadata.

## Typical misuse

- Hiding candidates or segments in `FrameContext`.
- Encoding object state only in mask pixels.

## Open questions

- Whether an explicit `object` structure is needed after filtering or whether filtered `Candidate`/`Segment` structures are enough for MVP.
- Standard quality flags for struct-domain objects.

## Connections

- has_structure: dp1.domain.struct.candidate
- has_structure: dp1.domain.struct.segment
- derived_from: dp1.domain.mask
- feeds: dp1.domain.measurement
