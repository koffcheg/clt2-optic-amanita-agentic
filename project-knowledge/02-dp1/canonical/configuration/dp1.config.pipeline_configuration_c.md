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
- вибрану реалізацію для кожного етапу;
- рівень складності;
- параметри;
- дозволені домени даних;
- обмеження під час виконання;
- вимоги до профілювання;
- режим валідації;
- дозволені швидкі шляхи;
- заборону неявних перетворень.

Базова форма DSL:

```json
{
  "schema_version": "1.0",
  "profile": "RT-5|RT-20|custom",
  "pipeline": {
    "prep": {
      "enabled": true,
      "variant": "full_frame|roi|tiles|adaptive_roi",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "radiometric": {
      "enabled": true,
      "variant": "mean_subtraction|gaussian_subtraction|inverse_median|adaptive_background|band_pass|per_tile_background",
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
      "variant": "gaussian_quasi|fixed_kernel|template|adaptive_kernel|psf_fit",
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
      "variant": "area|geometry|shape_photometry",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    },
    "measurement": {
      "enabled": true,
      "variant": "centroid_bbox|photometry_basic|moments_subpixel",
      "level": "L0|L1|L2|L3|Lx",
      "parameters": {}
    }
  }
}
```

Кожен етап має містити `enabled`, `variant`, `level`, `parameters`.
Профіль `RT-5` допускає лише `L0` і частково доведені `L1`-варіанти. Профіль
`RT-20` допускає `L1` і `L2`-варіанти за умови проходження часової валідації.

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
        "output_dynamic_range_mode": "RawSigned|ClipToInputRange|ShiftToPositive|ScaleToInputRange"
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
  циклічний буфер кадрів не виділяється;
- runtime disable/re-enable потребує окремого рішення у
  `dp1.stage_spec.radiometric_correction.inverse_median`.

## Interpretation

Вибір реалізації належить конфігурації, а не прихованим гілкам коду.

## Failure cases

- Реалізація етапу жорстко закодована.
- Швидкий шлях обходить правила доменів даних.
- Конфігурація не містить усіх основних етапів.
- Перетворення форматів або stateful-моделі не відображені у параметрах.

## Typical misuse

- Трактувати конфігурацію як необов’язкову runtime-декорацію.
- Міняти поведінку через приховані прапорці поза `C`.

## Open questions

- Точна схема і формат валідації.
- Формальний перелік параметрів для кожного `variant`.
- Політика сумісності `schema_version`.

## Connections

- uses: dp1.pipeline.formal_model
- constrained_by: dp1.domain.conversion_rules
- configures: dp1.stage_spec.radiometric_correction.inverse_median
