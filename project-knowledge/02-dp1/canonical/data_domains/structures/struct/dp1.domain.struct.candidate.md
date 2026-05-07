---
id: dp1.domain.struct.candidate
title: "Кандидат у Struct домені DP1"
tags: [dp1, canonical, data-domain, struct, candidate]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/struct/dp1.domain.struct.candidate.md"
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

```yaml
fields:
  - name: "`candidate_id`"
    type: "`std::uint64_t`"
    purpose: "Stable id hypothesis."
    used_for: "Traceability, relation to segment."
    memory: "8 B"
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Source frame."
    used_for: "Validation, diagnostics."
    memory: "8 B"
  - name: "`tile_id`"
    type: "`int`"
    purpose: "Source tile."
    used_for: "Border duplicate handling, profiling."
    memory: "4 B"
  - name: "`component_id`"
    type: "`int`"
    purpose: "Connected component id у source mask."
    used_for: "Source relation, debugging."
    memory: "4 B"
  - name: "`bbox_px`"
    type: "`cv::Rect`"
    purpose: "Bounding box candidate."
    used_for: "Area/geometry filtering, ROI for segmentation."
    memory: "16 B"
  - name: "`centroid_px`"
    type: "`cv::Point2f`"
    purpose: "Центр candidate."
    used_for: "L0 measurement, geometry filter."
    memory: "8 B"
  - name: "`area_px`"
    type: "`int`"
    purpose: "Площа candidate у pixels."
    used_for: "Min/max area filtering."
    memory: "4 B"
  - name: "`score`"
    type: "`float`"
    purpose: "Detector/candidate score, якщо stage spec задає."
    used_for: "Ranking, threshold diagnostics."
    memory: "4 B"
  - name: "`status`"
    type: "`CandidateStatus`"
    purpose: "`provisional` або інший bounded pre-filter state."
    used_for: "Candidate trace."
    memory: "4 B"
  - name: "`quality_flags`"
    type: "`std::uint32_t`"
    purpose: "Bounded flags замість free-form text."
    used_for: "Reject reasons, border flags, saturation flags."
    memory: "4 B"
  - name: "`coordinate_space`"
    type: "`CoordinateSpace`"
    purpose: "Tile-local або frame-global coordinates."
    used_for: "Merge correctness."
    memory: "4 B"
```

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
