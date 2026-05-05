---
id: dp1.domain.measurement
title:
  uk: "Домен вимірювань DP1"
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

`Measurement` є canonical DP1 data object для структурованого результату вимірювання, який може бути переданий через DP1 -> DP2 handoff.

Measurement не є `cv::Mat`, debug visualization, internal mask або тимчасовий processing buffer.

## Assumptions

- Measurement зберігає координати, геометрію, photometry і downstream metadata, потрібні DP2.
- Exact payload schema має бути узгоджена з `protocols.dp1_dp2.measurement_handoff`.
- Exact C++ representation is deferred to implementation tasks.

## Theorem / Contract

`Measurement` має мінімально містити або посилатися на:

- `measurement_id` — stable identifier measurement output.
- `frame_id` — кадр, з якого сформовано measurement.
- `source_id` або `camera_id` — джерело/camera relation.
- `time_ref` — acquisition або processing timestamp reference.
- `position_px` — canonical object position in pixel coordinates.
- `bbox_px` — object bounding geometry when applicable.
- `area_px` або equivalent geometry metric.
- optional `photometry` — intensity/statistical measurements defined by stage spec.
- `coordinate_system` і units metadata where applicable.
- `quality_flags` — bounded flags for measurement validity/quality.
- optional `source_segment_id` — relation to segment used for measurement.

Measurement має бути sufficient для downstream DP2 interpretation without requiring DP1 debug images, masks, or temporary buffers.

## Interpretation

Це продуктовий вихід DP1, а не debug artifact. Measurement є source domain for canonical DP1 -> DP2 protocol boundary.

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

- produced_by: dp1.stage.measurement
- derived_from: dp1.domain.segment
- feeds: protocols.dp1_dp2.measurement_handoff
