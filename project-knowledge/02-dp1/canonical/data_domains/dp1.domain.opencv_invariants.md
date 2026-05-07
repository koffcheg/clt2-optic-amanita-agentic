---
id: dp1.domain.opencv_invariants
title:
  uk: "OpenCV-інваріанти доменів DP1"
  en: "DP1 OpenCV domain invariants"
tags: [dp1, canonical, data-domain, opencv, invariants, code-generation]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.opencv_invariants.md"
status: "draft"
---

## Definition

Ця картка визначає canonical інваріанти для всіх DP1 structures, які несуть
OpenCV `cv::Mat` payload або ROI view.

## Assumptions

- OpenCV є storage/primitive layer, а не архітектура DP1.
- `cv::Mat` може бути owning buffer, shallow header або ROI/submatrix view.
- Stage specs можуть додавати суворіші вимоги, але не мають послаблювати ці
  інваріанти без окремого canonical рішення.

## Theorem / Contract

Для кожного `cv::Mat` carrier перед обробкою:

```yaml
opencv_mat_invariants:
  - rule: "mat.empty() == false"
    reason: "Stage не має обробляти порожній payload."
  - rule: "mat.channels() matches domain"
    reason: "Raw, Processing, Mask і Visualization у canonical DP1 route мають бути single-channel."
  - rule: "mat.type() matches PixelFormat"
    reason: "Storage carrier має відповідати route metadata."
  - rule: "mat.cols/mat.rows match declared geometry or tile view extent"
    reason: "Geometry contract не має виводитися тільки з Mat header."
  - rule: "ROI/tile Mat may be submatrix"
    reason: "`FramePacket.image(rect)` створює shallow ROI view."
  - rule: "mat.isContinuous() must not be assumed"
    reason: "ROI/submatrix часто має stride source frame."
  - rule: "Stage code must use row-wise access or check isContinuous() explicitly"
    reason: "AI-кодер не має писати unsafe continuous-buffer loops."
  - rule: "Visualization Mat must not feed computation"
    reason: "Visualization domain не є Raw, Processing або Mask domain."
```

Canonical mapping `PixelFormat -> OpenCV type`:

```yaml
opencv_type_mapping:
  - pixel_format: "U8"
    opencv_type: "CV_8UC1"
    domain: "Raw/Input або explicit fast route"
  - pixel_format: "U16"
    opencv_type: "CV_16UC1"
    domain: "Raw/Input"
  - pixel_format: "S16"
    opencv_type: "CV_16SC1"
    domain: "Explicit signed residual route"
  - pixel_format: "S32"
    opencv_type: "CV_32SC1"
    domain: "Explicit signed residual route"
  - pixel_format: "F32"
    opencv_type: "CV_32FC1"
    domain: "Processing"
  - pixel_format: "MaskU8"
    opencv_type: "CV_8UC1"
    domain: "Mask"
  - pixel_format: "U8 або U16"
    opencv_type: "CV_8UC1 або CV_16UC1"
    domain: "Visualization, same as input frame route"
```

`cv::Rect` у canonical DP1 використовує OpenCV half-open convention:

```text
x <= px < x + width
y <= py < y + height
```

Right/bottom boundary не включається.

Binary mask convention:

```yaml
mask_convention:
  background: 0
  canonical_foreground: 255
  opencv_foreground_interpretation: "OpenCV functions usually treat any non-zero as foreground."
  canonical_rule: "Stage output має нормалізувати foreground до 255, якщо stage spec явно не дозволяє 0/1 route."
```

## Fields / Interface

Кожна structure з `cv::Mat` має декларувати або посилатися на:

```yaml
required_metadata:
  - "PixelFormat або mask_format"
  - "FrameGeometry або tile-local extent"
  - "CoordinateSpace"
  - "ownership/lifetime policy"
  - "source relation або source frame id"
```

## Input / Output

Input:

- `FramePacket.image`;
- `TileRawView.image`;
- `ProcessingFrame.image`;
- `TileProcessingFrame.image`;
- `BinaryMask.mask`;
- `TileBinaryMask.mask`;
- `VisualizationFrame.image`, якщо visualization route буде описаний окремо.

Output:

- validation preconditions для future stage implementations;
- constraints для stage contract checks.

## Constraints

- AI-кодер не має робити implicit conversion тільки для того, щоб задовольнити
  OpenCV primitive.
- `cv::Mat::type()` не є повним domain contract.
- `CV_8UC1` може бути Raw U8 або MaskU8; semantics задаються `PixelFormat` і
  domain card.
- `cv::Mat` ROI view не має outlive source owner.

## Failure cases

- ROI tile обробляється як continuous buffer без перевірки stride.
- `CV_8UC1` mask трактується як grayscale processing image.
- `cv::Rect` обробляється як inclusive rectangle.
- Stage приймає `mat.empty()` і повертає empty output без diagnostic status.
- Visualization image використовується для photometry або thresholding.

## Typical misuse

- Виводити bit depth з `CV_16UC1` без `InputBitDepth`.
- Використовувати `.clone()` для кожного tile, щоб отримати continuous memory,
  без explicit stage spec.
- Змішувати `0/1` і `0/255` foreground convention між stages.

## Open questions

- Exact validation behavior: exception, status object або diagnostic record.
- Чи потрібен canonical wrapper `ConstMatView` для read-only OpenCV views.

## Connections

- constrains: dp1.domain.pixel_format
- constrains: dp1.domain.raw.frame_packet
- constrains: dp1.domain.raw.tile_raw_view
- constrains: dp1.domain.processing.frame
- constrains: dp1.domain.processing.tile_processing_frame
- constrains: dp1.domain.mask.binary_mask
- constrains: dp1.domain.mask.tile_binary_mask
- constrains: dp1.domain.coordinates
- constrains: dp1.pipeline.stage_io_matrix
- checked_by: dp1.validation.stage_contract_checks
