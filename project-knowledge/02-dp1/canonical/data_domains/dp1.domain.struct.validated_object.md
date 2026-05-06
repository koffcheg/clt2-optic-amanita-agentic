---
id: dp1.domain.struct.validated_object
title: "Валідований об'єкт у Struct домені DP1"
tags: [dp1, canonical, data-domain, struct, validated-object]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.struct.validated_object.md"
status: "draft"
---

## Definition

`ValidatedObject` є canonical structure у Struct domain для результату
`object_filtering`: компактного запису про object-like region, який пройшов або
не пройшов правила валідації.

`ValidatedObject` не є `MeasurementRecord` і не є payload DP1 -> DP2. Він
зберігає traceability до source `Candidate` або `Segment` і є входом для
`measurement`.

## Assumptions

- `ValidatedObject` створюється тільки stage `object_filtering`.
- Geometry у `ValidatedObject` має бути компактною; дорогі shape payloads
  лишаються в `Segment` і доступні через source relation.
- Конкретна C++ representation має визначатися окремою implementation task.
- У route `prep.variant = "tiles"` `ValidatedObject` може бути tile-local до
  merge/globalization step.

## Theorem / Contract

`ValidatedObject` має мінімально містити або посилатися на:

- `object_id` — stable local identifier validated-object output.
- `frame_id` — source frame.
- `camera_id` — source camera або DP1 instance.
- `tile_id` — source tile для tile route або `-1` для frame-global route.
- `source_kind` — чи object походить із `Candidate`, `Segment` або майбутнього
  canonical source.
- `source_candidate_id` і/або `source_segment_id` — traceability до upstream
  structures.
- `bbox_px`, `centroid_px`, `area_px` — compact geometry.
- `detection_score` — optional detector/candidate score, якщо upstream route
  його задає.
- `validation_score` — optional score object-filtering route.
- `status` — bounded validation result.
- `quality_flags` і `reject_flags` — canonical bit flags, а не free-form text.
- `coordinate_space` — tile-local або frame-global.

`ValidatedObject` не має містити `cv::Mat`, mask payload, full contour payload,
debug image або final measurement fields.

Рекомендована C++ форма:

```cpp
enum class ObjectValidationStatus : std::uint8_t {
    Accepted,
    Rejected
};

enum class ObjectSourceKind : std::uint8_t {
    Candidate,
    Segment
};

struct ValidatedObject {
    std::uint64_t object_id = 0;
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    int tile_id = -1;
    ObjectSourceKind source_kind = ObjectSourceKind::Segment;
    std::uint64_t source_candidate_id = 0;
    std::uint64_t source_segment_id = 0;
    cv::Rect bbox_px;
    cv::Point2f centroid_px{0.0F, 0.0F};
    int area_px = 0;
    float detection_score = 0.0F;
    float validation_score = 0.0F;
    ObjectValidationStatus status = ObjectValidationStatus::Accepted;
    std::uint32_t quality_flags = 0;
    std::uint32_t reject_flags = 0;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `object_id` | `std::uint64_t` | Stable id validated-object output. | Measurement relation, trace. | 8 B |
| `frame_id` | `std::uint64_t` | Source frame. | Validation, diagnostics. | 8 B |
| `camera_id` | `int` | Source camera/DP1 instance. | Multi-camera trace. | 4 B |
| `tile_id` | `int` | Source tile. | Border duplicate handling. | 4 B |
| `source_kind` | `ObjectSourceKind` | Upstream source type. | Correct source lookup. | 1 B |
| `source_candidate_id` | `std::uint64_t` | Relation до source candidate. | Audit/debug. | 8 B |
| `source_segment_id` | `std::uint64_t` | Relation до source segment. | Measurement geometry/shape lookup. | 8 B |
| `bbox_px` | `cv::Rect` | Compact object bounds. | Filtering, measurement ROI. | 16 B |
| `centroid_px` | `cv::Point2f` | Compact object center. | L0 measurement, merge. | 8 B |
| `area_px` | `int` | Object area. | Filtering, quality. | 4 B |
| `detection_score` | `float` | Upstream detector score, якщо є. | Ranking, diagnostics. | 4 B |
| `validation_score` | `float` | Filtering confidence/score. | Acceptance diagnostics. | 4 B |
| `status` | `ObjectValidationStatus` | Accepted/rejected result. | Measurement selection. | 1 B |
| `quality_flags` | `std::uint32_t` | Bounded quality flags. | DP1 diagnostics, DP2 interpretation if propagated. | 4 B |
| `reject_flags` | `std::uint32_t` | Bounded reject reasons. | Filtering trace. | 4 B |
| `coordinate_space` | `CoordinateSpace` | Tile-local або frame-global coordinates. | Merge correctness. | 4 B |

Орієнтовний розмір одного `ValidatedObject`: 96-128 B залежно від alignment.

## Пам'ять

`ValidatedObject` не містить image або contour payload. У tile route storage
має жити у `TileContext.validated_object_buffer` і переноситися в `TileResult`
тільки якщо merge або diagnostics потребують object-level records.

Для пам'яті важливі config limits:

- `max_validated_objects_per_tile`;
- `max_rejected_objects_per_tile`, якщо rejected records зберігаються для
  diagnostics;
- policy для того, чи rejected records потрапляють у production `TileResult`.

## Етапи

- `object_filtering` створює `ValidatedObject[]` з `Candidate[]` або `Segment[]`.
- `measurement` читає accepted `ValidatedObject[]` і, якщо потрібно, source
  `Segment[]` / raw / processing references.
- `merge` може globalize tile-local `ValidatedObject[]` перед measurement або
  після tile-local measurement route, залежно від stage spec.

## Interpretation

`ValidatedObject` відокремлює decision output object filtering від final
Measurement domain. Це дозволяє не трактувати `Candidate` або `Segment` як
прийнятий object і не змішувати filtering із DP2 payload.

## Failure cases

- `ValidatedObject` передається напряму в DP2 як canonical payload.
- Shape payload дублюється з `Segment` у кожен validated object без потреби.
- Rejected object втрачає `reject_flags`, тому filtering decision не
  трасується.
- Tile-local coordinates використовуються як frame-global.

## Typical misuse

- Замінювати `MeasurementRecord` на `ValidatedObject`.
- Зберігати debug text або image payload у validated object.
- Викидати source relation до `Candidate` або `Segment`.

## Open questions

- Чи rejected `ValidatedObject` мають зберігатися в production output або тільки
  в diagnostics.
- Exact namespace для `object_id`.
- Canonical score semantics для `validation_score`.

## Connections

- belongs_to: dp1.domain.struct
- derived_from: dp1.domain.struct.candidate
- derived_from: dp1.domain.struct.segment
- produced_by: dp1.stage.object_filtering
- used_by: dp1.stage.measurement
- constrained_by: dp1.domain.identity
- constrained_by: dp1.domain.coordinates
- constrained_by: dp1.domain.quality_flags
