---
id: dp1.stage_spec.input_normalization.sum_binning
title:
  uk: "Специфікація Stage0 Input Normalization software_sum_binning"
  en: "Stage0 Input Normalization software_sum_binning policy"
tags: [dp1, canonical, stage-spec, small-tz, input-normalization, binning]
kind: stage-spec-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stage_specs/dp1.stage_spec.input_normalization.sum_binning.md"
status: "draft"
---

## Definition

`Stage0 Input Normalization: software_sum_binning` є малою специфікацією для bounded software sum binning у
`Stage0 Input Normalization`.

Stage0 Input Normalization приймає `FramePacket`, перевіряє його проти `PipelineConfig.input_route`
і формує `CanonicalFrame`. Якщо `kbin = 1`, Stage0 працює як pass-through без
бінування. Якщо `kbin = 2` або `kbin = 4`, Stage0 формує нове забіноване
зображення меншої роздільної здатності.

`software_sum_binning` означає чисте сумування значень пікселів усередині
кожного `kbin x kbin` блоку. Цей variant не виконує усереднення, resize,
interpolation, `INTER_AREA`, max pooling, hardware binning або будь-яку
приховану нормалізацію значень.

Бінування належить Stage0 і виконується перед `Prep`, ROI, tiles або будь-якою
фрагментацією кадру. Забіноване processing image не розгортається назад до
початкової роздільної здатності.

## Scope

- Interface stage: `IInputNormalizationStage`.
- Pipeline slot: `input_normalization`.
- DSL key: `input_normalization`.
- Variant: `software_sum_binning`.
- Level: `L0`.

Підтримувані режими:

- disabled/pass-through: `kbin = 1`;
- software sum binning: `kbin = 2` або `kbin = 4`.

## Source requirements

Ця специфікація спирається на:

- `materials/ТЗ/Бінування.docx`;
- `materials/ТЗ/бінінг_програмування.docx`;
- `dp1.stage.input_normalization`;
- `dp1.domain.raw.canonical_frame`;
- `dp1.domain.memory_ownership`;
- `dp1.domain.coordinates`;
- `dp1.config.pipeline_configuration_c`;
- `dp1.config.stage_variant_registry`.

Факти з ТЗ, які є обов'язковими для Stage0 Input Normalization:

- зараз реалізується лише сумуючий binning;
- `kbin` є єдиним параметром бінування у цьому slice;
- допустимі `kbin`: `2` і `4`;
- `kbin = 1` означає disabled/pass-through;
- Stage0 Input Normalization виконується перед фрагментацією кадру на ROI або tiles;
- назад зображення не повертається;
- координати вимірювань мають бути перераховані у систему координат початкового
  кадру перед записом output або DP2 handoff;
- `INTER_AREA`, resize, average pooling, max pooling і hardware binning не є
  software sum binning.

## Inputs

- `FramePacket`;
- `FrameContext`;
- `PipelineConfig.input_route`;
- `PipelineConfig.input_normalization`;
- single-channel `cv::Mat` image carrier.

Вхідний `FramePacket` має відповідати `input_route` за:

- carrier pixel format;
- input bit depth;
- pixel range;
- geometry;
- stride;
- single-channel shape.

Stage0 Input Normalization не виконує приховане приведення до grayscale. Якщо вхід має більше
одного каналу, stage повертає explicit failure.

## Outputs

Успішний output:

- explicit `CanonicalFrame`;
- `FrameContext.artifacts` запис `canonical_frame`;
- `StageTiming` для `StageKey = "input_normalization"`.

Для `kbin = 1` output semantics збігається зі Stage0.1 pass-through:

- `CanonicalFrame.image` є borrowed read-only view;
- `normalization.binned = false`;
- `normalization.binning_mode = None`;
- `bin_factor_x = 1`;
- `bin_factor_y = 1`.

Для `kbin = 2` або `kbin = 4` output semantics:

- `CanonicalFrame.image` є owned binned stage output;
- `CanonicalFrame.image_ownership = OwnedBinned`;
- `normalization.source = Stage0`;
- `normalization.copied = true`;
- `normalization.converted = false`;
- `normalization.binned = true`;
- `normalization.bin_factor_x = kbin`;
- `normalization.bin_factor_y = kbin`;
- `normalization.binning_mode = Sum`;
- `parent_artifact_id = "raw_frame"`.

## Configuration contract

Канонічний фрагмент `C`:

```json
{
  "input_normalization": {
    "enabled": true,
    "variant": "software_sum_binning",
    "level": "L0",
    "parameters": {
      "kbin": 2
    }
  }
}
```

Правила:

- `input_normalization.enabled = false` вимикає Stage0 і є runtime failure для pipeline,
  який потребує `CanonicalFrame`;
- `variant = "passthrough"` дозволений тільки для no-binning route;
- `variant = "software_sum_binning"` дозволяє `kbin = 1`, `2` або `4`;
- `kbin = 1` означає pass-through/no binning без arithmetic;
- `kbin = 2` або `kbin = 4` вмикає software sum binning;
- відсутній `parameters.kbin` для `software_sum_binning` є configuration error;
- невідомі ключі у `input_normalization.parameters` є configuration error;
- non-sum mode не може бути виражений через цей variant.

Ознаки бінування мають бути відображені у config-driven provenance
`CanonicalFrame.normalization`. Config не замінює runtime provenance.

## Algorithm

Для `kbin = 2` або `kbin = 4` забіноване зображення `Ib` визначається формулою:

```text
Ib(i,j) =
  sum_{m=0..kbin-1} sum_{n=0..kbin-1}
    I(kbin*i + m, kbin*j + n)
```

де:

- `I` є source image у `FramePacket`;
- `Ib` є output image у `CanonicalFrame`;
- `i` є x-index output image;
- `j` є y-index output image;
- `kbin` є factor бінування.

OpenCV implementation mapping:

Документи-джерела використовують математичні індекси `i`, `j` для
забінованого зображення. Для C++/OpenCV implementation ці індекси мають бути
зіставлені явно:

- `i` є binned x-index і відповідає `xb` / output column;
- `j` є binned y-index і відповідає `yb` / output row;
- `m` є source x-offset всередині bin і відповідає `dx` / column offset;
- `n` є source y-offset всередині bin і відповідає `dy` / row offset.

Тому доступ до `cv::Mat` має використовувати:

```text
source_row = kbin * yb + dy
source_col = kbin * xb + dx
output_row = yb
output_col = xb
```

Еквівалентна implementation formula:

```text
Ib[yb, xb] =
  sum_{dy=0..kbin-1} sum_{dx=0..kbin-1}
    I[kbin * yb + dy, kbin * xb + dx]
```

Implementation не має міняти місцями row/column indexes під час використання
`cv::Mat::at(row, col)` або row-pointer доступу.

Implementation approach для першої реалізації:

- explicit summing loops є approved baseline;
- `cv::boxFilter(normalize=false)` може бути використаний тільки після окремого
  explicit approval;
- `cv::resize`, `INTER_AREA`, average pooling і max pooling заборонені.

## Політика чистого сумування

`software_sum_binning` має такі обов'язкові заборони:

- жодного прихованого downcast до source carrier;
- жодного прихованого scaling або normalization;
- жодного прихованого clipping, saturation або clamp;
- жодного повернення original `U8` або `U16` carrier, якщо результат суми
  потребує ширший carrier для lossless representation;
- жодного average/resize compatibility path під назвою
  `software_sum_binning`.

Якщо для сумісності з downstream route потрібен lossy output, він має бути
окремим compatibility variant із власною назвою, config contract і validation
route. Такий variant не є canonical pure sum binning.

Stage0 Input Normalization не виконує average:

```text
Ib(i,j) != (1 / kbin^2) * sum(...)
```

Stage0 Input Normalization не виконує interpolation або geometric resize.

## Політика динамічного діапазону і carrier

Stage0 Input Normalization використовує lossless widening без clipping.

Для `kbin = 2` або `kbin = 4`:

```text
scale = kbin * kbin
output_min = source.pixel_range.min_value * scale
output_max = source.pixel_range.max_value * scale
output_black_level = source.pixel_range.black_level * scale
output_saturation_level = source.pixel_range.saturation_level * scale
```

Ці поля описують accumulated output range. Вони не є source range і не мають
повертатися до source range через приховане scaling або clipping.

### U8 input

Stage0.1 and Stage0.2-pre are implementation/task slice labels, not canonical stage names. The canonical stage remains Stage0 Input Normalization.

Вхід:

- `PixelFormat = U8`;
- `InputBitDepth = Bit8`;
- source carrier type: `CV_8UC1`.

Вихід:

- canonical semantics output carrier: `AccumU32`;
- без saturation/clamping;
- без прихованого scaling;
- без прихованого downcast до `U8`;
- максимально можлива сума для `kbin = 2`: `255 * 4 = 1020`;
- максимально можлива сума для `kbin = 4`: `255 * 16 = 4080`.

Політика `pixel_range`:

- output `pixel_range.min_value = output_min`;
- output `pixel_range.max_value = output_max`;
- output `pixel_range.black_level = output_black_level`;
- output `pixel_range.saturation_level = output_saturation_level`.

### U16 input

Вхід:

- `PixelFormat = U16`;
- `InputBitDepth = Bit10`, `Bit12`, `Bit14` або `Bit16`;
- source carrier type: `CV_16UC1`.

Вихід:

- canonical semantics output carrier: `AccumU32`;
- canonical output carrier має бути невідʼємним accumulated sum carrier;
- transitional storage carrier `CV_32SC1` / `S32` може бути дозволений тільки як
  задокументований перехідний route, якщо metadata явно фіксує невідʼємний accumulated carrier,
  а не signed residual;
- metadata output bit depth: 32-bit accumulated sum output;
- без saturation/clamping;
- без прихованого scaling;
- без прихованого downcast до `U16`;
- максимально можлива сума для `kbin = 2`: `65535 * 4 = 262140`;
- максимально можлива сума для `kbin = 4`: `65535 * 16 = 1048560`.

Політика `pixel_range`:

- output `pixel_range.min_value = output_min`;
- output `pixel_range.max_value = output_max`;
- output `pixel_range.black_level = output_black_level`;
- output `pixel_range.saturation_level = output_saturation_level`.

### Варіанти carrier policy

Допустимі варіанти policy для майбутньої implementation task:

- `AccumU32` є єдиний canonical semantic carrier для pure `software_sum_binning`.
- `S32` transitional carrier дозволений лише як тимчасовий storage route на базі `CV_32SC1`, якщо metadata явно декларує `AccumU32` semantics і забороняє трактування як signed residual.
- `U32` може згадуватися тільки як можливий future enum/storage naming detail, не як окрема canonical semantic альтернатива.
- Lossy compatibility variant (downcast/scaled/clipped output) може існувати лише окремо і не є `software_sum_binning`.

Поточний code-backed C++ vocabulary може не мати `PixelFormat::AccumU32`; це implementation gap для окремої runtime task, а не підстава звужувати canonical policy.

## Сумісність із downstream

`Radiometric` і будь-який downstream stage мають явно оголосити підтримку
розширений accumulated carrier, перш ніж pipeline route з `software_sum_binning`
може виконуватися.

Якщо downstream route не підтримує `AccumU32` semantics або approved transitional
`S32` accumulated carrier, pipeline має повернути explicit failure під час config validation або stage boundary validation. Заборонено:

- приховано приводити accumulated carrier назад до `U8` або `U16`;
- приховано масштабувати accumulated values до source range;
- приховано clipping/saturation до source carrier range;
- підміняти pure sum average/resize output.

## Geometry

For source geometry:

```text
source_width = W
source_height = H
```

Output geometry:

```text
binned_width = W / kbin
binned_height = H / kbin
```

Odd or non-divisible dimensions policy:

- if `W % kbin != 0`, Stage0 Input Normalization returns explicit failure;
- if `H % kbin != 0`, Stage0 Input Normalization returns explicit failure;
- no crop;
- no padding;
- no resize.

Stride policy:

- output `CanonicalFrame.image.step` is defined by the owned output `cv::Mat`;
- output `FrameGeometry` must match output columns and rows;
- source stride is not propagated as output stride after binning.

## Coordinate mapping

Coordinate spaces:

- `FramePacket` uses `SourceFrameGlobal`;
- binned `CanonicalFrame` uses `CanonicalFrameGlobal`;
- final `MeasurementRecord` and DP2-facing handoff must use
  `SourceFrameGlobal`.

Stage0 Input Normalization defines this mapping:

```text
x0 = xoff + kbin * xb
y0 = yoff + kbin * yb
```

For Stage0 Input Normalization full-frame baseline:

```text
xoff = 0
yoff = 0
```

`x0` and `y0` are the coordinates that must be written as measurements. Do not
add `(kbin - 1) / 2`; the source documents define corner-based coordinates.

Stage0 Input Normalization must preserve enough metadata for later measurement globalization:

- `kbin`;
- `xoff`;
- `yoff`;
- source geometry;
- binned geometry;
- relation `CanonicalFrameGlobal -> SourceFrameGlobal`.

If coordinate conversion is not implemented in Stage0 Input Normalization, Stage0 Input Normalization still has to
emit metadata/provenance sufficient for the later measurement or merge boundary
to convert coordinates before output.

## Ownership and lifetime

For binned output:

- `CanonicalFrame.image` is owned by the explicit Stage0 output object;
- `CanonicalFrame.image_ownership = OwnedBinned`;
- `FrameContext.artifacts` does not own the heavy image payload;
- artifact ownership is `OwnedByStageOutput`;
- artifact lifetime is `StageOutputScope`;
- artifact status is `Available`.

If a later runtime task needs the binned image beyond Stage0 output scope, that
task must explicitly define a longer owner or transfer policy. The artifact
registry alone must not extend `cv::Mat` lifetime.

## FrameContext artifact

Successful binned Stage0 Input Normalization registers:

```yaml
artifact:
  id: "canonical_frame"
  semantic_name: "canonical_frame"
  kind: "CanonicalFrame"
  domain: "Raw"
  producer_stage: "input_normalization"
  parent_artifact_id: "raw_frame"
  ownership: "OwnedByStageOutput"
  lifetime: "StageOutputScope"
  status: "Available"
```

Artifact metadata must include:

- output pixel format;
- metadata output bit depth;
- output geometry;
- parent artifact reference;
- producer stage.

## Timing / profiling

Stage0 Input Normalization must record stage-level timing:

- `StageKey = "input_normalization"`;
- variant `software_sum_binning`;
- level `L0`;
- input format;
- output format;
- status;
- explicit reason on failure.

Operation-level timing для allocation, copy і binning loop може бути доданий
лише як follow-up після explicit approval profiling scope. До runtime implementation ця specification все одно вимагає видимої semantics для copy/allocation через `CanonicalFrame.normalization`
і artifact metadata: binned output має `copied = true`, `binned = true` і
owned stage-output lifetime. Якщо майбутній profiling додасть operation timings,
вони мають залишатися у `FrameContext.profiling`, а не в artifact registry.

## Failure cases

Stage0 Input Normalization має повернути explicit failure для таких випадків:

- unsupported `kbin`;
- відсутній `kbin` для `software_sum_binning`;
- unsupported pixel format;
- unsupported input bit depth;
- input не є single-channel;
- invalid geometry або stride;
- width або height не діляться на `kbin`;
- ризик overflow, який не може бути представлений approved widened output type;
- ambiguous source `pixel_range`;
- запитано non-sum binning;
- запитано resize, pooling, interpolation або `INTER_AREA` замість sum binning;
- conversion запитано поза scope Stage0 Input Normalization;
- downstream route не може спожити widened binned output;
- requested lossy compatibility behavior через `software_sum_binning`;
- приховане повернення до original `U8` або `U16` carrier після sum;
- `S32` carrier трактується як signed residual замість невідʼємного accumulated sum carrier.

## Non-goals

- Average binning.
- Max pooling.
- Resize або `INTER_AREA`.
- Hardware binning.
- ROI.
- Tiles.
- Prep behavior.
- DP2 implementation.
- CameraProSim runtime expansion.
- OverlayRunner.
- Downstream stage expansion.
- Measurement coordinate conversion implementation, якщо окрема task явно не включає це у scope.

## Validation route

Unit validation route is defined separately in:

- `validation.dp1.input_normalization.sum_binning`

Automated tests require separate approval under `TESTING_POLICY`.

## Source-of-truth and canonicalization safety

Ця StageSpec є необхідним джерелом для майбутньої Stage0 Input Normalization code generation.
Вона не стверджує, що `software_sum_binning` уже реалізований у
`datapro1_v2`.

Якщо поточний code/config vocabulary не підтримує `U32`,
`software_sum_binning` variant або downstream widened outputs, це є
implementation gap для Stage0 Input Normalization task, а не підстава звужувати цю специфікацію
без explicit approval.

## Connections

- specifies: dp1.stage.input_normalization
- uses: dp1.domain.raw.frame_packet
- produces: dp1.domain.raw.canonical_frame
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- constrained_by: dp1.config.pipeline_configuration_c
- constrained_by: dp1.config.stage_variant_registry
- constrained_by: dp1.pipeline.stage_io_matrix
- constrained_by: dp1.pipeline.stage_domain_bindings
- validated_by: validation.dp1.input_normalization.sum_binning
