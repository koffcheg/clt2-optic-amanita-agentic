---
id: dp1.domain.measurement.record
title: "Запис вимірювання в Measurement домені DP1"
tags: [dp1, canonical, data-domain, measurement, structure]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/measurement/dp1.domain.measurement.record.md"
status: "draft"
---

## Definition

`MeasurementRecord` є canonical structure у Measurement domain для структурованого результату вимірювання, який може передаватися через DP1 -> DP2 handoff.

`MeasurementRecord` не є `cv::Mat`, debug visualization, internal mask або temporary processing buffer.

## Assumptions

- Measurement має містити координати, геометрію, фотометрію та downstream metadata, потрібні DP2.
- Точна payload schema має бути узгоджена з `protocols.dp1_dp2.measurement_handoff`.
- Конкретна C++ структура має визначатися окремою implementation task.
- Frame-level `MeasurementRecord[]` output одного DP1 instance належить одному
  `camera_id` / `source_id`.

## Theorem / Contract

`MeasurementRecord` має мінімально містити або посилатися на:

- `measurement_id` — stable identifier measurement output.
- `frame_id` — кадр, з якого сформовано measurement.
- `source_id` або `camera_id` — зв'язок із джерелом/camera.
- `source_object_id` — зв'язок із `ValidatedObject`, з якого сформовано
  measurement.
- `time_ref` — acquisition або processing timestamp reference.
- `position_px` — canonical object position in pixel coordinates.
- `bbox_px` — object bounding geometry, якщо застосовно.
- `area_px` або equivalent geometry metric.
- optional `photometry` — intensity/statistical measurements, визначені stage spec.
- `coordinate_system` і units metadata, де це потрібно.
- `quality_flags` — bounded flags for measurement validity/quality.
- optional `source_segment_id` — зв'язок із segment, використаним для measurement.

Measurement має бути sufficient для downstream DP2 interpretation without requiring DP1 debug images, masks, or temporary buffers.

`camera_id` у `MeasurementRecord` позначає active source camera поточного DP1
instance. Якщо DP2 отримує measurements від кількох камер, це має бути результатом
кількох DP1 instances або окремої downstream aggregation boundary.

Рекомендована C++ форма:

```cpp
struct PhotometryStats {
    float mean_intensity = 0.0F;
    float max_intensity = 0.0F;
    float stddev_intensity = 0.0F;
    PixelFormat source_format = PixelFormat::U16;
    InputBitDepth source_bit_depth = InputBitDepth::Bit16;
};

struct MeasurementRecord {
    std::uint64_t measurement_id = 0;
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::uint64_t source_object_id = 0;
    std::uint64_t source_segment_id = 0;
    TimestampRef time_ref;
    cv::Point2f position_px{0.0F, 0.0F};
    cv::Rect bbox_px;
    int area_px = 0;
    PhotometryStats photometry;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;
    std::uint32_t quality_flags = 0;
};
```

## Поля `PhotometryStats`

```yaml
fields:
  - name: "`mean_intensity`"
    type: "`float`"
    purpose: "Середня яскравість object region."
    used_for: "Object quality, DP2 interpretation."
    memory: "4 B"
  - name: "`max_intensity`"
    type: "`float`"
    purpose: "Максимальна яскравість region."
    used_for: "Peak/saturation checks."
    memory: "4 B"
  - name: "`stddev_intensity`"
    type: "`float`"
    purpose: "Розкид яскравості."
    used_for: "Contrast/noise quality."
    memory: "4 B"
  - name: "`source_format`"
    type: "`PixelFormat`"
    purpose: "Carrier, з якого рахували photometry."
    used_for: "Correct interpretation of stats."
    memory: "4 B"
  - name: "`source_bit_depth`"
    type: "`InputBitDepth`"
    purpose: "Фактична source бітність."
    used_for: "Scale/range interpretation у DP2."
    memory: "4 B"
```

## Поля `MeasurementRecord`

```yaml
fields:
  - name: "`measurement_id`"
    type: "`std::uint64_t`"
    purpose: "Stable final id."
    used_for: "DP2 tracking/input relation."
    memory: "8 B"
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Source frame."
    used_for: "DP2 timeline, validation."
    memory: "8 B"
  - name: "`camera_id`"
    type: "`int`"
    purpose: "Active source camera поточного DP1 instance."
    used_for: "Multi-instance DP2 handoff."
    memory: "4 B"
  - name: "`source_object_id`"
    type: "`std::uint64_t`"
    purpose: "Relation до accepted `ValidatedObject`."
    used_for: "Audit/debug, object-filtering trace."
    memory: "8 B"
  - name: "`source_segment_id`"
    type: "`std::uint64_t`"
    purpose: "Relation до source segment."
    used_for: "Audit/debug."
    memory: "8 B"
  - name: "`time_ref`"
    type: "`TimestampRef`"
    purpose: "Timestamp для result."
    used_for: "Latency і DP2 temporal logic."
    memory: "~16 B"
  - name: "`position_px`"
    type: "`cv::Point2f`"
    purpose: "Object position у pixels."
    used_for: "DP2 input, tracking seed."
    memory: "8 B"
  - name: "`bbox_px`"
    type: "`cv::Rect`"
    purpose: "Bounding geometry."
    used_for: "DP2 filtering/debug, duplicate suppression."
    memory: "16 B"
  - name: "`area_px`"
    type: "`int`"
    purpose: "Area of measured region."
    used_for: "Quality/filtering."
    memory: "4 B"
  - name: "`photometry`"
    type: "`PhotometryStats`"
    purpose: "Brightness statistics."
    used_for: "Quality, downstream interpretation."
    memory: "~20 B"
  - name: "`coordinate_space`"
    type: "`CoordinateSpace`"
    purpose: "Final output має бути frame-global."
    used_for: "Protocol correctness."
    memory: "4 B"
  - name: "`quality_flags`"
    type: "`std::uint32_t`"
    purpose: "Validity/quality flags."
    used_for: "DP2 decisions, partial/border flags."
    memory: "4 B"
```

Орієнтовний розмір одного `MeasurementRecord`: 100-140 B залежно від alignment.

## Пам'ять

`MeasurementRecord` не містить `cv::Mat`, mask, contour payload або debug image.
У route `prep.variant = "tiles"` measurements спочатку живуть у `TileContext.measurement_buffer`,
потім переносяться в `TileResult.measurement_results`, після merge — у
frame-level `std::vector<MeasurementRecord>`.

## Етапи

- `measurement` створює tile-local `MeasurementRecord[]` з accepted
  `ValidatedObject[]` і raw/processing photometry reference.
- `merge` перетворює coordinates у `FrameGlobal`, прибирає border duplicates і
  формує frame-level measurements.
- DP1 -> DP2 handoff споживає тільки frame-level `MeasurementRecord[]`.
- Один frame-level output не має змішувати measurements різних камер без
  окремого protocol/aggregation contract.

## Interpretation

Це продуктовий вихід DP1, а не debug artifact. `MeasurementRecord` є source structure для canonical DP1 -> DP2 protocol boundary.

## Failure cases

- Внутрішні маски або visualization images передаються як canonical DP2 input.
- Measurement lacks frame/time/source identity.
- Frame-level measurement batch змішує кілька `camera_id` як один DP1 output.
- Геометрія видається без coordinate-system metadata.
- Фотометрія рахується з display/debug buffer замість measurement domain.

## Typical misuse

- Кодувати measurement як pixels.
- Трактувати candidate або segment як final measurement без measurement-stage contract.

## Open questions

- Exact payload schema and versioning policy.
- Required calibration metadata.
- Required units for coordinates and photometry.
- Error/partial-frame semantics for DP2 handoff.

## Connections

- belongs_to: dp1.domain.measurement
- produced_by: dp1.stage.measurement
- derived_from: dp1.domain.struct.validated_object
- may_reference: dp1.domain.struct.segment
- constrained_by: dp1.domain.identity
- constrained_by: dp1.domain.time
- constrained_by: dp1.domain.coordinates
- constrained_by: dp1.domain.quality_flags
- feeds: protocols.dp1_dp2.measurement_handoff
