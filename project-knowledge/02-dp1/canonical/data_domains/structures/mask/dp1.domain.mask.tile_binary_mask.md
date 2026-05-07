---
id: dp1.domain.mask.tile_binary_mask
title: "Локальна binary mask tile у Mask домені DP1"
tags: [dp1, canonical, data-domain, mask, structure, tile]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/mask/dp1.domain.mask.tile_binary_mask.md"
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

```yaml
fields:
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Зв'язок із source frame."
    used_for: "Validation, merge trace."
    memory: "8 B"
  - name: "`tile_id`"
    type: "`int`"
    purpose: "Зв'язок із source tile."
    used_for: "Tile diagnostics, duplicate handling."
    memory: "4 B"
  - name: "`mask`"
    type: "`cv::Mat`"
    purpose: "Binary pixels одного tile."
    used_for: "Connected components, contours, segmentation refinement."
    memory: "header ~96 B; payload у `TileContext`"
  - name: "`valid_area`"
    type: "`cv::Rect`"
    purpose: "Border-safe mask area."
    used_for: "Crop перед candidates/segments acceptance."
    memory: "16 B"
  - name: "`origin_px`"
    type: "`cv::Point`"
    purpose: "Local-to-global offset."
    used_for: "Transform geometry."
    memory: "8 B"
  - name: "`background_value`"
    type: "`std::uint8_t`"
    purpose: "Значення background."
    used_for: "Mask validation."
    memory: "1 B"
  - name: "`foreground_value`"
    type: "`std::uint8_t`"
    purpose: "Значення foreground."
    used_for: "Components/contours extraction."
    memory: "1 B"
  - name: "`coordinate_space`"
    type: "`CoordinateSpace`"
    purpose: "Tile-local або frame-global."
    used_for: "Merge correctness."
    memory: "4 B"
```

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
