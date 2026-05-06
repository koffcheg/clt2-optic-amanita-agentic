---
id: dp1.domain.struct.segment
title: "Сегмент у Struct домені DP1"
tags: [dp1, canonical, data-domain, struct, segment]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.struct.segment.md"
status: "draft"
---

## Definition

`Segment` є canonical structure у Struct domain для уточненої області, отриманої після segmentation або segmentation refinement.

`Segment` уточнює geometry/shape candidate, але сам по собі ще не є validated
object або final measurement.

## Assumptions

- `Segment` може посилатися на source `Candidate` і/або source `BinaryMask`.
- Segment representation може бути mask ROI, contour або bounded geometry залежно від stage spec.
- Конкретна C++ representation має визначатися окремою implementation task.

## Theorem / Contract

`Segment` має мінімально містити або посилатися на:

- `segment_id` — stable identifier у межах кадру або stage output.
- `frame_id` — кадр, якому належить segment.
- `candidate_id` — optional relation до source candidate.
- `source_mask_ref` — optional relation до source mask.
- `bbox_px` — bounding box у processing-frame coordinates.
- `area_px` — площа segment region.
- `shape_ref` — mask ROI, contour або equivalent bounded representation.
- `quality_flags` — bounded flags для оцінки якості segmentation.

`Segment` не має містити final measurement payload, призначений для DP2 handoff.

Рекомендована C++ форма:

```cpp
struct Segment {
    std::uint64_t segment_id = 0;
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    std::uint64_t candidate_id = 0;
    cv::Rect bbox_px;
    int area_px = 0;
    std::vector<cv::Point> contour_px;
    std::uint32_t quality_flags = 0;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `segment_id` | `std::uint64_t` | Stable id refined region. | Traceability, measurement source. | 8 B |
| `frame_id` | `std::uint64_t` | Source frame. | Validation, diagnostics. | 8 B |
| `tile_id` | `int` | Source tile. | Border duplicate handling, profiling. | 4 B |
| `candidate_id` | `std::uint64_t` | Relation до source candidate. | Audit, debugging, source trace. | 8 B |
| `bbox_px` | `cv::Rect` | Bounding box segment. | Geometry filtering, measurement ROI. | 16 B |
| `area_px` | `int` | Площа segment region. | Filtering, measurement. | 4 B |
| `contour_px` | `std::vector<cv::Point>` | Shape representation. | Moments, rotated bbox, geometry filters. | ~24 B + 8 B per point |
| `quality_flags` | `std::uint32_t` | Bounded quality/reject flags. | Segmentation/filtering diagnostics. | 4 B |
| `coordinate_space` | `CoordinateSpace` | Tile-local або frame-global. | Merge/measurement correctness. | 4 B |

Base size без contour payload: приблизно 80 B. `contour_px` є потенційно
дорогим: 100 points коштують приблизно 800 B payload.

## Пам'ять

Segment storage має жити у `TileContext.segment_buffer`. Для noisy masks треба
configuration limits:

- `max_segments_per_tile`;
- `max_contour_points_per_segment`;
- `min_segment_area_px`;
- policy для contour simplification, якщо stage spec це дозволяє.

## Етапи

- `segmentation_refinement` створює `Segment[]`.
- `object_filtering` читає `Segment[]` і створює `ValidatedObject[]`.
- `measurement` може читати source `Segment[]` разом із `ValidatedObject[]`,
  якщо measurement route потребує contour або shape source.
- `merge` перетворює tile-local segment coordinates у frame-global, якщо
  segment results переносяться за межі tile.

## Interpretation

`Segment` є проміжною structure між candidate-level hypotheses і validated
object. Він може використовуватися на етапах object filtering або measurement
як source geometry/shape, але не як accepted object сам по собі.

## Failure cases

- `Segment` втрачає relation до source candidate/frame.
- Segment geometry використовує іншу coordinate system без metadata.
- Measurement fields змішуються з segment output.

## Typical misuse

- Трактувати segment як validated object або final measurement.
- Кодувати segment тільки як debug visualization.

## Open questions

- Canonical shape representation: ROI mask, contour або compact shape reference.
- Required segmentation quality flags.
- Чи дозволені multi-component segments.
- Чи потрібен окремий compact shape representation без повного contour.

## Connections

- belongs_to: dp1.domain.struct
- derived_from: dp1.domain.struct.candidate
- derived_from: dp1.domain.mask.binary_mask
- used_by: dp1.stage.object_filtering
- used_by: dp1.stage.measurement
- may_produce: dp1.domain.struct.validated_object
- may_produce: dp1.domain.measurement.record
- constrained_by: dp1.domain.identity
- constrained_by: dp1.domain.coordinates
- constrained_by: dp1.domain.quality_flags
