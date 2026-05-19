---
id: dp1.domain.raw.canonical_frame
title:
  uk: "CanonicalFrame як нормалізований вхід DP1"
  en: "CanonicalFrame as normalized DP1 input"
tags: [dp1, canonical, data-domain, raw, canonical-frame, ownership]
kind: data-structure-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/raw/dp1.domain.raw.canonical_frame.md"
status: "draft"
---

## Definition

`CanonicalFrame` є explicit data carrier нормалізованого входу DP1 після
`Stage0 Input Normalization`. Він відділяє source/raw boundary `FramePacket`
від кадру, який готовий для `Stage1 Prep` і подальших stages.

У Stage0.1 `CanonicalFrame` є pass-through borrowed/read-only view на payload
`FramePacket`, якщо source frame відповідає `PipelineConfig.input_route`.

## Assumptions

- `CanonicalFrame` є output Stage0, а не прихованим payload у `FrameContext`.
- `FrameContext.artifacts` відображає `CanonicalFrame` як authoritative
  artifact, але не стає власником heavy image payload за замовчуванням.
- Binning/conversion fields можуть бути присутні в conceptual structure, але
  Stage0.1 встановлює no-op provenance.
- Майбутній route Stage0.2 `software_sum_binning` може emit-ити розширений
  accumulated carrier. Такий carrier описує суму, а не source pixel range.

## Theorem / Contract

Канонічна концептуальна форма:

```cpp
enum class BinningMode {
    None,
    Sum,
    Average
};

enum class NormalizationSource {
    None,
    Camera,
    CameraProSim,
    Stage0,
    External
};

enum class PayloadOwnership {
    BorrowedReadOnly,
    OwnedCopy,
    OwnedConverted,
    OwnedBinned
};

struct NormalizationProvenance {
    NormalizationSource source = NormalizationSource::None;
    bool copied = false;
    bool converted = false;
    bool binned = false;
    int bin_factor_x = 1;
    int bin_factor_y = 1;
    BinningMode binning_mode = BinningMode::None;
    PixelFormat source_pixel_format;
    InputBitDepth source_bit_depth;
    PixelRange source_pixel_range;
};

struct CanonicalFrame {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string source_id;

    cv::Mat image;
    PayloadOwnership image_ownership = PayloadOwnership::BorrowedReadOnly;

    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    PixelRange pixel_range;

    FrameGeometry geometry;
    CoordinateSpace coordinate_space = CoordinateSpace::CanonicalFrameGlobal;

    TimestampRef ingest_time;
    std::optional<TimestampRef> acquisition_time;

    std::string parent_artifact_id;
    NormalizationProvenance normalization;
};
```

Цей C++ fragment є canonical semantic sketch, а не ABI-вимога. Реальна
implementation може використовувати інші type names, якщо зберігає semantic
fields і ownership/lifetime contract.

Stage0.1 обов'язково встановлює:

```yaml
stage0_1_defaults:
  image_ownership: "BorrowedReadOnly"
  normalization.source: "Stage0"
  normalization.copied: false
  normalization.converted: false
  normalization.binned: false
  normalization.bin_factor_x: 1
  normalization.bin_factor_y: 1
  normalization.binning_mode: "None"
  parent_artifact_id: "raw_frame або equivalent FramePacket artifact id"
```

`BinningMode` не має default-итися в `Sum`, якщо `binned = false`.

## Interpretation

`CanonicalFrame` означає, що DP1 має єдиний input carrier після Stage0. Для
Stage0.1 цей carrier не змінює pixels і не змінює geometry. Він тільки робить
межу input normalization явною, перевіреною і відображеною у `FrameContext`.

Майбутній Stage0.2 може створити `CanonicalFrame` з `OwnedBinned` або
`OwnedConverted` payload, але це потребує окремої StageSpec і окремої задачі.

Для `software_sum_binning` значення `OwnedBinned` означає owned output Stage0,
який містить accumulated sum carrier. `CanonicalFrame.pixel_range` після такого
route описує accumulated dynamic range:

```text
scale = bin_factor_x * bin_factor_y
output_min = source_min * scale
output_max = source_max * scale
black_level = source_black_level * scale
saturation_level = source_saturation_level * scale
```

`CanonicalFrame` після pure sum binning не має приховано повертати source
`U8`/`U16` range, виконувати scaling, clipping або downcast. Якщо runtime
тимчасово використовує `S32` storage для accumulated sum, metadata має явно
відрізняти його від signed residual route.

## Failure cases

- `CanonicalFrame.image` має borrowed payload, але artifact позначений як owned.
- `CanonicalFrame` не містить parent artifact reference.
- `binned = false`, але `binning_mode = Sum`.
- Pixel format або bit depth відрізняються від source, але `converted = false`.
- Payload reference використовується після завершення declared lifetime.
- `FrameContext` вважається власником `CanonicalFrame.image` без explicit
  ownership transfer.
- `binned = true`, але `pixel_range` залишено source range без множення на
  `bin_factor_x * bin_factor_y`.
- Pure sum output приховано downcast-иться, scale-иться або clip-иться до
  source carrier range.
- Transitional `S32` carrier для accumulated sum трактується як signed residual.

## Typical misuse

- Трактувати `CanonicalFrame` як копію кадру за замовчуванням.
- Трактувати `CanonicalFrame` як дозвіл мутувати input image.
- Ховати `CanonicalFrame` тільки в context registry і не повертати explicit
  output зі Stage0.
- Виконувати ROI/tiles на рівні `CanonicalFrame`.

## Open questions

- Exact C++ enum names для `PayloadOwnership` і `NormalizationSource`.
- Чи має `parent_artifact_id` бути string id або typed `FrameArtifactRef`.
- Чи потрібен окремий `CanonicalBinnedFrame` card після Stage0.2.
- Остаточна назва canonical accumulated carrier: `AccumU32` або `U32`.

## Connections

- produced_by: dp1.stage.input_normalization
- consumes: dp1.domain.raw.frame_packet
- feeds: dp1.stage.prep
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- constrained_by: dp1.domain.pixel_format
- reflected_by: dp1.domain.runtime.frame_context
