---
id: dp1.domain.struct.candidate
title: "Кандидат у Struct домені DP1"
tags: [dp1, canonical, data-domain, struct, candidate]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.struct.candidate.md"
status: "draft"
---

## Definition

`Candidate` є canonical structure у Struct domain для попередньої гіпотези про об'єкт після candidate extraction.

`Candidate` не є validated object і не є measurement.

## Assumptions

- `Candidate` може бути отриманий через connected components, thresholding або інший candidate extraction route.
- Конкретна C++ representation має визначатися окремою implementation task.
- `Candidate` confidence/score є optional, якщо stage spec не визначає його явно.

## Theorem / Contract

`Candidate` має мінімально містити або посилатися на:

- `candidate_id` — stable identifier у межах кадру або stage output.
- `frame_id` — кадр, з якого отримано candidate.
- `source_mask_ref` або `component_id` — походження з mask/component extraction.
- `bbox_px` — bounding box у processing-frame coordinates.
- `centroid_px` — optional centroid у pixel coordinates.
- `area_px` — площа candidate у pixels.
- `status` — `provisional`; acceptance/rejection після filtering належить
  `ValidatedObject`.
- optional `score` — detector/candidate score, якщо це визначено stage spec.
- optional `quality_flags` — bounded flags, а не free-form debug text.

`Candidate` має бути explicit object. Його не можна кодувати тільки через mask pixels.

Рекомендована C++ форма:

```cpp
struct Candidate {
    std::uint64_t candidate_id = 0;
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    int component_id = -1;
    cv::Rect bbox_px;
    cv::Point2f centroid_px{0.0F, 0.0F};
    int area_px = 0;
    float score = 0.0F;
    CandidateStatus status = CandidateStatus::Provisional;
    std::uint32_t quality_flags = 0;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `candidate_id` | `std::uint64_t` | Stable id hypothesis. | Traceability, relation to segment. | 8 B |
| `frame_id` | `std::uint64_t` | Source frame. | Validation, diagnostics. | 8 B |
| `tile_id` | `int` | Source tile. | Border duplicate handling, profiling. | 4 B |
| `component_id` | `int` | Connected component id у source mask. | Source relation, debugging. | 4 B |
| `bbox_px` | `cv::Rect` | Bounding box candidate. | Area/geometry filtering, ROI for segmentation. | 16 B |
| `centroid_px` | `cv::Point2f` | Центр candidate. | L0 measurement, geometry filter. | 8 B |
| `area_px` | `int` | Площа candidate у pixels. | Min/max area filtering. | 4 B |
| `score` | `float` | Detector/candidate score, якщо stage spec задає. | Ranking, threshold diagnostics. | 4 B |
| `status` | `CandidateStatus` | `provisional` або інший bounded pre-filter state. | Candidate trace. | 4 B |
| `quality_flags` | `std::uint32_t` | Bounded flags замість free-form text. | Reject reasons, border flags, saturation flags. | 4 B |
| `coordinate_space` | `CoordinateSpace` | Tile-local або frame-global coordinates. | Merge correctness. | 4 B |

Орієнтовний розмір одного `Candidate`: 72-96 B залежно від alignment і enum
representation.

## Пам'ять

`Candidate` не містить pixels, contours або `cv::Mat`. У route
`prep.variant = "tiles"` candidate storage має жити у
`TileContext.candidate_buffer` і переноситися в `TileResult.candidate_results`
тільки якщо це потрібно merge/debug policy.

Для пам'яті важливі config limits:

- `max_candidates_per_tile`;
- `min_candidate_area_px`;
- `max_candidate_area_px`;
- політика для noisy masks.

## Етапи

- `candidate_extraction` створює `Candidate[]` з `TileBinaryMask`.
- `segmentation_refinement` може читати `Candidate[]` для уточнення region.
- `object_filtering` читає `Candidate[]` у routes без segment stage і створює
  `ValidatedObject[]`.
- `measurement` не має трактувати `Candidate` як final measurement без
  measurement-stage contract.

## Interpretation

`Candidate` є bridge між mask-level extraction і downstream segmentation/filtering. Він описує hypothesis geometry і мінімальні attributes, але не підтверджує існування об'єкта.

## Failure cases

- `Candidate` трактується як validated object.
- Geometry candidate не узгоджена з source mask geometry.
- Candidate не має `frame_id` або source relation.

## Typical misuse

- Передавати candidates як unstructured list of rectangles без identity/source metadata.
- Ховати candidate details у `FrameContext` замість explicit stage output.

## Open questions

- Exact namespace для `candidate_id`.
- Required score/confidence semantics.
- Мінімальні geometry fields для code generation.

## Connections

- belongs_to: dp1.domain.struct
- derived_from: dp1.domain.mask.binary_mask
- feeds: dp1.stage.segmentation_refinement
- feeds: dp1.stage.object_filtering
- may_produce: dp1.domain.struct.segment
- may_produce: dp1.domain.struct.validated_object
- constrained_by: dp1.domain.identity
- constrained_by: dp1.domain.coordinates
- constrained_by: dp1.domain.quality_flags
