---
id: dp1.domain.measurement.record
title:
  uk: "Запис вимірювання в Measurement домені DP1"
  en: "DP1 measurement record structure"
tags: [dp1, canonical, data-domain, measurement, structure]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.measurement.record.md"
  lines: "1-N"
status: "draft"
---

## Definition

`MeasurementRecord` є canonical structure у Measurement domain для структурованого результату вимірювання, який може бути переданий через DP1 -> DP2 handoff.

`MeasurementRecord` не є `cv::Mat`, debug visualization, internal mask або тимчасовий processing buffer.

## Assumptions

- Measurement зберігає координати, геометрію, photometry і downstream metadata, потрібні DP2.
- Exact payload schema має бути узгоджена з `protocols.dp1_dp2.measurement_handoff`.
- Exact C++ representation is deferred to implementation tasks.

## Theorem / Contract

`MeasurementRecord` має мінімально містити або посилатися на:

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

Це продуктовий вихід DP1, а не debug artifact. MeasurementRecord є source structure for canonical DP1 -> DP2 protocol boundary.

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

- belongs_to: dp1.domain.measurement
- produced_by: dp1.stage.measurement
- derived_from: dp1.domain.struct.segment
- feeds: protocols.dp1_dp2.measurement_handoff
