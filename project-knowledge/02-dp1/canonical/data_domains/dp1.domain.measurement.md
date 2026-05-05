---
id: dp1.domain.measurement
title:
  uk: "Measurement домен DP1"
  en: "DP1 Measurement domain"
tags: [dp1, canonical, data-domain, measurement]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.measurement.md"
  lines: "1-N"
status: "draft"
---

## Definition

Measurement domain описує semantic role фінального структурованого виходу DP1, який може бути переданий через DP1 -> DP2 handoff.

Конкретний запис вимірювання описаний окремою structure card `dp1.domain.measurement.record`.

## Assumptions

- Measurement domain зберігає координати, геометрію, photometry і downstream metadata, потрібні DP2.
- Exact payload schema має бути узгоджена з `protocols.dp1_dp2.measurement_handoff`.
- Exact C++ representation is deferred to implementation tasks.

## Theorem / Contract

Measurement domain має такі правила:

- measurement output is structured data, not `cv::Mat`;
- measurement output is product output, not debug artifact;
- measurement output must include enough identity/time/source/coordinate metadata for DP2 interpretation;
- internal masks, visualization images, and temporary buffers must not be part of canonical handoff;
- measurement records may reference source segments or raw/processing data used for photometry.

Canonical MVP structure:

- `dp1.domain.measurement.record`.

## Interpretation

Measurement domain є canonical source domain for DP1 -> DP2 protocol boundary. It is the result of measurement stage, not a generic structure bucket.

## Failure cases

- Надсилання внутрішніх масок або visualization images як canonical DP2 input.
- Measurement lacks frame/time/source identity.
- Geometry is emitted without coordinate-system metadata.
- Photometry is computed from display/debug buffer instead of measurement domain.

## Typical misuse

- Кодувати measurement як pixels.
- Treating candidate or segment as final measurement without measurement-stage contract.

## Open questions

- Exact payload schema and versioning policy.
- Required calibration metadata.
- Required units for coordinates and photometry.
- Error/partial-frame semantics for DP2 handoff.

## Connections

- has_structure: dp1.domain.measurement.record
- produced_by: dp1.stage.measurement
- derived_from: dp1.domain.struct.segment
- feeds: protocols.dp1_dp2.measurement_handoff
