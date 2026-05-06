---
id: dp1.domain.mask.tile_binary_mask
title: "Локальна binary mask tile у Mask домені DP1"
tags: [dp1, canonical, data-domain, mask, structure, tile]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.mask.tile_binary_mask.md"
status: "draft"
---

## Definition

`TileBinaryMask` є tile-local Mask-domain structure для binary foreground /
background representation після `candidate_extraction`.

Для `prep.variant = "tiles"` це mask payload route. Full-frame `BinaryMask`
належить іншим prep variants або explicit stage specs.

## Assumptions

- Payload `mask` зазвичай посилається на `TileContext.mask_buffer`.
- Canonical carrier — `MaskU8`, зазвичай `CV_8UC1`.
- Binary convention default: background `0`, foreground `255`.

## Theorem / Contract

Рекомендована C++ форма:

```cpp
struct TileBinaryMask {
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    cv::Mat mask;
    cv::Rect valid_area;
    cv::Point origin_px{0, 0};
    std::uint8_t background_value = 0;
    std::uint8_t foreground_value = 255;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `frame_id` | `std::uint64_t` | Зв'язок із source frame. | Validation, merge trace. | 8 B |
| `tile_id` | `int` | Зв'язок із source tile. | Tile diagnostics, duplicate handling. | 4 B |
| `mask` | `cv::Mat` | Binary pixels одного tile. | Connected components, contours, segmentation refinement. | header ~96 B; payload у `TileContext` |
| `valid_area` | `cv::Rect` | Border-safe mask area. | Crop перед candidates/segments acceptance. | 16 B |
| `origin_px` | `cv::Point` | Local-to-global offset. | Transform geometry. | 8 B |
| `background_value` | `std::uint8_t` | Значення background. | Mask validation. | 1 B |
| `foreground_value` | `std::uint8_t` | Значення foreground. | Components/contours extraction. | 1 B |
| `coordinate_space` | `CoordinateSpace` | Tile-local або frame-global. | Merge correctness. | 4 B |

## Пам'ять

Для tile `256x256` `MaskU8` payload становить ~64 KB. Payload має жити в
`TileContext.mask_buffer` і перевикористовуватися між tiles того самого worker.

Full-frame mask не є частиною route `prep.variant = "tiles"`.

## Етапи

- `candidate_extraction` видає `TileBinaryMask`.
- `segmentation_refinement` читає `TileBinaryMask`.
- `object_filtering` не має читати mask напряму, якщо stage spec не дозволяє.
- `measurement` не має рахувати photometry з `TileBinaryMask`.

## Обмеження

- `mask` не є intensity image.
- `foreground_value` і `background_value` мають бути явними, якщо route
  відрізняється від `0/255`.
- Tile outputs із border area мають бути відкинуті або cropped.

## Failure cases

- Mask використовується для photometry.
- `0/1` і `0/255` conventions змішані без stage spec.
- Full-frame mask створюється в route `prep.variant = "tiles"` без explicit stage spec або config mode.

## Typical misuse

- Передавати debug visualization як mask.
- Кодувати candidates тільки через pixels без `Candidate[]`.

## Open questions

- Чи потрібен окремий tile-local label mask для connected components output.

## Connections

- belongs_to: dp1.domain.mask
- derived_from: dp1.domain.processing.tile_processing_frame
- owned_by: dp1.domain.runtime.tile_context
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- may_produce: dp1.domain.struct.candidate
- used_by: dp1.stage.candidate_extraction
- used_by: dp1.stage.segmentation_refinement
