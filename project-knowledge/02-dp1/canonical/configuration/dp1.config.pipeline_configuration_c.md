---
id: dp1.config.pipeline_configuration_c
title:
  uk: "Canonical-конфігурація конвеєра DP1 C"
  en: "Canonical DP1 pipeline configuration C"
tags: [dp1, canonical, config]
kind: config-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/configuration/dp1.config.pipeline_configuration_c.md"
  lines: "1-130"
status: "draft"
---

## Definition

`C` - це canonical-конфігурація конвеєра DP1.

## Assumptions

- Повна JSON-схема ще не визначена.
- Конфігурація має бути відтворюваною і достатньою для повторення поведінки
  конвеєра без зміни коду.

## Theorem / Contract

`C` визначає:
- порядок етапів;
- Stage0 input normalization contract;
- вибрану реалізацію для кожного етапу;
- рівень складності;
- параметри;
- input route DP1 instance: `U8` або `U16` carrier і фактичну бітність;
- stage specs and stage parameters define processing-domain outputs and allowed internal formats;
- дозволені домени даних;
- обмеження під час виконання;
- processing-profile requirements, які потрібні profiling domain для
  відтворюваності measurement;
- режим валідації;
- дозволені швидкі шляхи;
- заборону неявних перетворень.

Назви `stage`, дозволені `variant` і розділення `variant` / `level`
визначає `dp1.config.stage_variant_registry`. Ця картка задає форму `C`, але
не є registry допустимих algorithm families.

Базова форма DSL:

```json
{
  "schema_version": "1.0",
  "profile": "RT-5|RT-20|custom",
  "input_route": {
    "pixel_format": "U8|U16",
    "bit_depth": "8|10|12|14|16",
    "pixel_range": {
      "min_value": 0,
      "max_value": 255,
      "black_level": 0,
      "saturation_level": 255
    }
  },
  "pipeline": {
    "input_normalization": {
      "enabled": true,
      "variant": "passthrough",
      "level": "L0",
      "parameters": {}
    },
    "prep": {
      "enabled": true,
      "variant": "full_frame|roi|tiles|adaptive_roi",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "radiometric": {
      "enabled": true,
      "variant": "mean_subtraction|gaussian_subtraction|median|inverse_median|adaptive_background|band_pass|per_tile_background",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "enhancement": {
      "enabled": true,
      "variant": "gaussian|dog|bilateral|guided|multi_scale",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "matched_filter": {
      "enabled": true,
      "variant": "gaussian|kernel|template|adaptive_kernel|psf_fit",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "candidate_extraction": {
      "enabled": true,
      "variant": "global_threshold|adaptive_threshold|absdiff|mog2|knn",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "segmentation": {
      "enabled": true,
      "variant": "single_morphology|open_close_contours|connected_components|multi_step_morphology",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "object_filtering": {
      "enabled": true,
      "variant": "area|geom_basic|shape_photometry",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "measurement": {
      "enabled": true,
      "variant": "centroid_bbox|photometry_basic|rotated_bbox_moments_subpixel",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    }
  }
}
```

Кожен етап має містити `enabled`, `variant`, `level`, `parameters`.

Для Stage0.1 `pipeline.input_normalization.variant` має бути `passthrough`. Binning
parameters і arithmetic policy належать майбутній Stage0.2 StageSpec і не є
частиною цього baseline contract.
Профіль `RT-5` допускає лише `L0` і частково доведені `L1`-варіанти. Профіль
`RT-20` допускає `L1` і `L2`-варіанти за умови проходження часової валідації.

Typed canonical config model для code generation має існувати незалежно від
JSON representation. JSON є serialization/authoring form, але C++ генерація
має спиратися на typed fields:

```cpp
struct StageConfig {
    bool enabled = false;
    std::string variant;
    std::string level;
    ParameterMap parameters;
};

struct InputRouteConfig {
    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    PixelRange pixel_range;
};

struct PrepRouteConfig {
    std::string variant;
    int tile_width = 0;
    int tile_height = 0;
    int overlap_x = 0;
    int overlap_y = 0;
};

struct RuntimeLimitsConfig {
    int worker_count = 1;
    int max_candidates_per_tile = 0;
    int max_segments_per_tile = 0;
    int max_validated_objects_per_tile = 0;
    int max_measurements_per_frame = 0;
};

struct PipelineConfig {
    std::string schema_version;
    std::string profile;
    InputRouteConfig input_route;
    PrepRouteConfig prep_route;
    RuntimeLimitsConfig runtime_limits;
    StageConfig input_normalization;
    StageConfig prep;
    StageConfig radiometric;
    StageConfig enhancement;
    StageConfig matched_filter;
    StageConfig candidate_extraction;
    StageConfig segmentation;
    StageConfig object_filtering;
    StageConfig measurement;
};
```

`C` конфігурує тільки processing pipeline DP1. Runtime-рівень застосунку,
включно з operational logging switches, operational profiling switches, raw
trace retention, report emission, logging bridge і external trace boundary,
визначається окремо в `dp1.config.application`, з деталями у
`dp1.config.application.logging` і `dp1.config.application.profiling`.

Profiling records використовують `C` як active configuration reference:
stage order, `variant`, `level`, input route і parameters потрібні для
відтворюваності runtime measurement. Але `C` не має містити hidden switches на
кшталт `profiling_enabled`, report paths або external profiler backend.

Threshold-like parameters мають використовувати `ThresholdConfig` із
`dp1.domain.threshold`, а не untyped numeric values.

Canonical processing-domain carriers and range semantics are defined by data-domain cards and stage specs.

`input_route` фіксує Raw/Input carrier, bit depth і pixel range DP1 instance на
startup. Для Stage0.1 `input_route` є validation contract: source frame має
відповідати цьому contract, інакше Stage0 повертає explicit failure замість
прихованої conversion. Спосіб просторової підготовки кадру задає
`pipeline.prep.variant`.

`prep.variant` підтримує кілька варіантів:

- `full_frame` — обробка всього кадру як одного processing unit;
- `roi` — обробка одного або кількох явно заданих ROI;
- `tiles` — обробка через `TileDesc[]`, tile-local buffers і merge;
- `adaptive_roi` — динамічний вибір ROI за окремою stage spec.

Кожен варіант `prep` має мати власне мале ТЗ / stage spec з memory policy,
coordinate policy і validation rules.

Приклад `U8` instance:

```json
{
  "input_route": {
    "pixel_format": "U8",
    "bit_depth": 8,
    "pixel_range": {
      "min_value": 0,
      "max_value": 255,
      "black_level": 0,
      "saturation_level": 255
    }
  },
  "pipeline": {
    "input_normalization": {
      "enabled": true,
      "variant": "passthrough",
      "level": "L0",
      "parameters": {}
    },
    "prep": {
      "enabled": true,
      "variant": "tiles",
      "level": "L1",
      "parameters": {
        "tile_width": 256,
        "tile_height": 256,
        "overlap_x": 16,
        "overlap_y": 16
      }
    }
  }
}
```

Приклад `U16` carrier з 12-bit sensor data:

```json
{
  "input_route": {
    "pixel_format": "U16",
    "bit_depth": 12,
    "pixel_range": {
      "min_value": 0,
      "max_value": 4095,
      "black_level": 0,
      "saturation_level": 4095
    }
  },
  "pipeline": {
    "input_normalization": {
      "enabled": true,
      "variant": "passthrough",
      "level": "L0",
      "parameters": {}
    },
    "prep": {
      "enabled": true,
      "variant": "tiles",
      "level": "L1",
      "parameters": {
        "tile_width": 256,
        "tile_height": 256,
        "overlap_x": 16,
        "overlap_y": 16
      }
    }
  }
}
```

Route selection не є algorithm implementation. Stage implementation має явно
оголошувати supported route, наприклад `RadiometricU8` або `RadiometricU16`,
але використовувати ті самі canonical structures.

Для `radiometric.variant = "inverse_median"` параметри визначає
`dp1.stage_spec.radiometric_correction.inverse_median`.

Фрагмент конфігурації `inverse_median`:

```json
{
  "radiometric": {
    "enabled": true,
    "variant": "inverse_median",
    "level": "L1",
    "parameters": {
      "inverse_median": {
        "enabled": true,
        "mode": "FixedK3|FixedK5",
        "stride": 1,
        "output_median_frame": false,
        "output_dynamic_range_mode": "RawSigned|ClipToInputRange"
      }
    }
  }
}
```

Для `inverse_median`:
- `stride >= 1`;
- внутрішній pipeline output за замовчуванням - `RawSigned`;
- рекомендований користувацький або compatibility output - `ClipToInputRange`;
- якщо `inverse_median` вимкнено у конфігурації при старті програми, пам'ять під
  циклічний буфер кадрів за контрактом
  `dp1.domain.runtime.cyclic_frame_buffer` не виділяється;
- runtime disable/re-enable потребує окремого рішення у
  `dp1.stage_spec.radiometric_correction.inverse_median`.

## Interpretation

Вибір реалізації належить конфігурації, а не прихованим гілкам коду.

## Failure cases

- Реалізація етапу жорстко закодована.
- Швидкий шлях обходить правила доменів даних.
- Конфігурація не містить усіх основних етапів.
- `variant` використовується як `level` або навпаки.
- Перетворення форматів або stateful-моделі не відображені у параметрах.
- DP1 instance змінює `input_route` між кадрами без explicit reconfiguration
  і buffer reallocation policy.
- Stage0 приховано виконує conversion/binning, хоча `pipeline.input_normalization`
  налаштований як `passthrough`.
- `prep.variant` вимагає одного memory/coordinate route, а implementation
  використовує інший без explicit stage spec.
- Tile-specific structures використовуються для `full_frame`, `roi` або
  `adaptive_roi` без явного binding у відповідній stage spec.

## Typical misuse

- Трактувати конфігурацію як необов’язкову runtime-декорацію.
- Міняти поведінку через приховані прапорці поза `C`.
- Додавати operational profiling switches у stage parameters замість
  `dp1.config.application.profiling`.
- Вибирати 8-bit або 16-bit algorithm implementation через `cv::Mat::type()`
  без route metadata.

## Open questions

- Точна схема і формат валідації.
- Формальний перелік параметрів для кожного `variant`.
- Політика сумісності `schema_version`.
- Формальний registry supported routes для кожної stage implementation за межами
  stage-interface level.

## Connections

- uses: dp1.pipeline.formal_model
- uses: dp1.config.stage_variant_registry
- constrained_by: dp1.domain.conversion_rules
- constrained_by: dp1.domain.pixel_format
- constrained_by: dp1.domain.common_types
- constrained_by: dp1.domain.profiling
- constrained_by: dp1.domain.threshold
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- references: dp1.domain.runtime.cyclic_frame_buffer
- related_runtime_config: dp1.config.application
- related_runtime_config: dp1.config.application.logging
- related_runtime_config: dp1.config.application.profiling
- configures: dp1.stage_spec.radiometric_correction.inverse_median
