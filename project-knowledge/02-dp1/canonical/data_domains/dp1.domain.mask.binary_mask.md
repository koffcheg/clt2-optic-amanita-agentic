---
id: dp1.domain.mask.binary_mask
title: "Бінарна маска в Mask домені DP1"
tags: [dp1, canonical, data-domain, mask, structure]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.mask.binary_mask.md"
status: "draft"
---

## Definition

`BinaryMask` є canonical structure у Mask domain для foreground/background representation після thresholding, candidate extraction або segmentation-related stages.

## Assumptions

- Canonical carrier для binary mask — `MaskU8`, зазвичай `CV_8UC1`.
- Label masks мають бути окремою future або stage-specific structure.
- Конкретна C++ representation має визначатися окремою implementation task.

## Theorem / Contract

Для canonical binary-mask route `BinaryMask` має такий contract:

- carrier: `MaskU8`, зазвичай `CV_8UC1`;
- background value: `0`;
- foreground value: `255`, якщо stage spec явно не визначає іншу binary convention;
- geometry: така сама, як у source frame / processing representation, якщо remapping не заданий явно.

`BinaryMask` має мінімально містити або посилатися на:

- `frame_id`;
- `source_ref` — relation до source `ProcessingFrame`, `Candidate` або `Segment`;
- `pixel_format` — `MaskU8`;
- `geometry`;
- optional `foreground_convention`, якщо використовується не `0/255`.

Для `prep.variant = "tiles"` full-frame `BinaryMask` не є runtime payload цього
route. Tile-specific mask payload описує `TileBinaryMask`.

Рекомендована full-frame C++ форма, якщо stage spec явно дозволяє full-frame
route:

```cpp
struct BinaryMask {
    std::uint64_t frame_id = 0;
    std::string source_ref;
    cv::Mat mask;
    PixelFormat pixel_format = PixelFormat::MaskU8;
    FrameGeometry geometry;
    std::uint8_t background_value = 0;
    std::uint8_t foreground_value = 255;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `frame_id` | `std::uint64_t` | Source frame identity. | Validation, trace. | 8 B |
| `source_ref` | `std::string` | Relation до processing output або stage output. | Audit, debugging. | ~24 B + payload |
| `mask` | `cv::Mat` | Binary mask payload. | Components, contours, segmentation. | header ~96 B + payload |
| `pixel_format` | `PixelFormat` | Має бути `MaskU8`. | Validation. | 4 B |
| `geometry` | `FrameGeometry` | Geometry mask representation. | Coordinate checks. | ~16 B |
| `background_value` | `std::uint8_t` | Background convention. | Mask validation. | 1 B |
| `foreground_value` | `std::uint8_t` | Foreground convention. | Components/contours. | 1 B |
| `coordinate_space` | `CoordinateSpace` | Frame-global або tile-local. | Merge correctness. | 4 B |

## Пам'ять

Full-frame `MaskU8` для `1440x1080` коштує приблизно 1.6 MB. Для
`prep.variant = "tiles"` такий buffer не створюється цим route; mask memory
цього route живе у `TileContext.mask_buffer`. Для `full_frame`, `roi` і
`adaptive_roi` memory policy має бути задана окремою stage spec.

## Етапи

- Full-frame `BinaryMask` може існувати тільки якщо stage spec/config явно
  вибирає full-frame route.
- Stages у route `prep.variant = "tiles"` використовують `TileBinaryMask`.

## Interpretation

`BinaryMask` не є grayscale processing image. Це semantic object із mask-specific invariants.

Binary masks можуть використовуватись для connected components, segmentation, filtering і measurement-related ROI extraction.

## Failure cases

- Mask трактується як intensity image.
- Foreground convention відрізняється між stages без явного contract.
- Geometry mask відрізняється від source frame без transform metadata.

## Typical misuse

- Передавати debug visualization як canonical mask.
- Кодувати candidate list тільки через mask pixels без explicit candidate objects.

## Open questions

- Чи всі binary masks мають використовувати `0/255`, чи окремі internal routes можуть використовувати `0/1`.
- Standard label mask carrier type для connected components output.

## Connections

- belongs_to: dp1.domain.mask
- uses: dp1.domain.pixel_format
- constrained_by: dp1.domain.coordinates
- derived_from: dp1.domain.processing.frame
- may_produce: dp1.domain.struct.candidate
- used_by: dp1.stage.candidate_extraction
- used_by: dp1.stage.segmentation_refinement
