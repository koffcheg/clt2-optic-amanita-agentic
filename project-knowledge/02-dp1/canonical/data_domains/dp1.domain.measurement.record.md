---
id: dp1.domain.measurement.record
title: "Запис вимірювання в Measurement домені DP1"
tags: [dp1, canonical, data-domain, measurement, structure]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.measurement.record.md"
status: "draft"
---

## Definition

`MeasurementRecord` є canonical structure у Measurement domain для структурованого результату вимірювання, який може передаватися через DP1 -> DP2 handoff.

`MeasurementRecord` не є `cv::Mat`, debug visualization, internal mask або temporary processing buffer.

## Assumptions

- Measurement має містити координати, геометрію, фотометрію та downstream metadata, потрібні DP2.
- Точна payload schema має бути узгоджена з `protocols.dp1_dp2.measurement_handoff`.
- Конкретна C++ структура має визначатися окремою implementation task.

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

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `mean_intensity` | `float` | Середня яскравість object region. | Object quality, DP2 interpretation. | 4 B |
| `max_intensity` | `float` | Максимальна яскравість region. | Peak/saturation checks. | 4 B |
| `stddev_intensity` | `float` | Розкид яскравості. | Contrast/noise quality. | 4 B |
| `source_format` | `PixelFormat` | Carrier, з якого рахували photometry. | Correct interpretation of stats. | 4 B |
| `source_bit_depth` | `InputBitDepth` | Фактична source бітність. | Scale/range interpretation у DP2. | 4 B |

## Поля `MeasurementRecord`

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `measurement_id` | `std::uint64_t` | Stable final id. | DP2 tracking/input relation. | 8 B |
| `frame_id` | `std::uint64_t` | Source frame. | DP2 timeline, validation. | 8 B |
| `camera_id` | `int` | Source camera/DP1 instance. | Multi-instance DP2 handoff. | 4 B |
| `source_object_id` | `std::uint64_t` | Relation до accepted `ValidatedObject`. | Audit/debug, object-filtering trace. | 8 B |
| `source_segment_id` | `std::uint64_t` | Relation до source segment. | Audit/debug. | 8 B |
| `time_ref` | `TimestampRef` | Timestamp для result. | Latency і DP2 temporal logic. | ~16 B |
| `position_px` | `cv::Point2f` | Object position у pixels. | DP2 input, tracking seed. | 8 B |
| `bbox_px` | `cv::Rect` | Bounding geometry. | DP2 filtering/debug, duplicate suppression. | 16 B |
| `area_px` | `int` | Area of measured region. | Quality/filtering. | 4 B |
| `photometry` | `PhotometryStats` | Brightness statistics. | Quality, downstream interpretation. | ~20 B |
| `coordinate_space` | `CoordinateSpace` | Final output має бути frame-global. | Protocol correctness. | 4 B |
| `quality_flags` | `std::uint32_t` | Validity/quality flags. | DP2 decisions, partial/border flags. | 4 B |

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

## Interpretation

Це продуктовий вихід DP1, а не debug artifact. `MeasurementRecord` є source structure для canonical DP1 -> DP2 protocol boundary.

## Failure cases

- Внутрішні маски або visualization images передаються як canonical DP2 input.
- Measurement lacks frame/time/source identity.
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
