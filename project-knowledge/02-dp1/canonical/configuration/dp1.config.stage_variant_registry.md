---
id: dp1.config.stage_variant_registry
title:
  uk: "Registry етапів, варіантів і рівнів DP1"
  en: "DP1 stage, variant, and level registry"
tags: [dp1, canonical, config, stage, variant, code-generation]
kind: config-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/configuration/dp1.config.stage_variant_registry.md"
status: "draft"
---

## Definition

Ця картка визначає canonical registry для восьми фіксованих етапів DP1,
дозволених `variant`, рівнів складності `level` і меж відповідальності
конфігурації `C`.

## Assumptions

- Registry є knowledge-level контрактом для майбутньої генерації коду.
- Registry не є малим ТЗ етапу і не визначає алгоритмічні деталі реалізації.
- Повні параметри кожного `variant` мають бути визначені у майбутніх stage specs
  або окремих configuration cards.

## Theorem / Contract

Canonical DP1 має вісім фіксованих stage interfaces:

```yaml
stages:
  - prep
  - radiometric_correction
  - enhancement
  - matched_filtering
  - candidate_extraction
  - segmentation_refinement
  - object_filtering
  - measurement
```

Терміни:

```yaml
terms:
  - name: "Stage"
    meaning: "Фіксований interface pipeline."
  - name: "Variant"
    meaning: "Сімейство реалізації етапу, яке вибирається через configuration C."
  - name: "Level"
    meaning: "Клас складності, вартості і очікуваної якості для вибраного variant."
  - name: "Parameters"
    meaning: "Variant-specific параметри у `stage.parameters`."
```

`variant` і `level` не є одним поняттям. AI-кодер не має трактувати `L0`, `L1`,
`L2`, `L3` або `Lx` як назви algorithm variant.

Кожен stage configuration fragment має містити:

```json
{
  "enabled": true,
  "variant": "<registered-variant>",
  "level": "L0|L1|L2|L3|Lx",
  "parameters": {}
}
```

Дозволені stage variants:

```yaml
stage_variant_registry:
  - stage: "prep"
    variants: ["full_frame", "roi", "tiles", "adaptive_roi"]
    typical_levels: ["L0", "L1", "L2"]
    notes: "Визначає просторовий route кадру, ROI або tile execution."

  - stage: "radiometric_correction"
    variants:
      - "mean_subtraction"
      - "gaussian_subtraction"
      - "median"
      - "inverse_median"
      - "adaptive_background"
      - "band_pass"
      - "per_tile_background"
    typical_levels: ["L0", "L1", "L2"]
    notes: "Формує residual/corrected representation і може мати stateful background owner."

  - stage: "enhancement"
    variants: ["gaussian", "dog", "bilateral", "guided", "multi_scale"]
    typical_levels: ["L0", "L1", "L2"]
    notes: "Підвищує SNR перед detection без формування final detections."

  - stage: "matched_filtering"
    variants: ["gaussian", "kernel", "template", "adaptive_kernel", "psf_fit"]
    typical_levels: ["L0", "L1", "L2"]
    notes: "Формує detector response map у Processing domain."

  - stage: "candidate_extraction"
    variants: ["global_threshold", "adaptive_threshold", "absdiff", "mog2", "knn"]
    typical_levels: ["L0", "L1", "L2"]
    notes: "Перетворює response/residual/enhanced representation на mask і Candidate[]."

  - stage: "segmentation_refinement"
    variants: ["single_morphology", "open_close_contours", "connected_components", "multi_step_morphology"]
    typical_levels: ["L0", "L1", "L2"]
    notes: "Перетворює mask/candidates на Segment[] або уточнює object regions."

  - stage: "object_filtering"
    variants: ["area", "geom_basic", "shape_photometry"]
    typical_levels: ["L0", "L1", "L2"]
    notes: "Формує ValidatedObject[] і не формує MeasurementRecord напряму."

  - stage: "measurement"
    variants: ["centroid_bbox", "photometry_basic", "rotated_bbox_moments_subpixel"]
    typical_levels: ["L0", "L1", "L2"]
    notes: "Формує MeasurementRecord[] як final DP1 Measurement-domain output."
```

`prep.variant = "tiles"` змінює execution route, але не додає нові semantic
stages. У tile route етапи `radiometric_correction` ... `measurement`
виконуються для tile-local carriers, а frame-level output формується через
`TileResult[]` і merge.

AI-кодер не має:

- створювати новий `variant`, якщо його немає у registry;
- вибирати `variant` поза `PipelineConfig`;
- підміняти `variant` рівнем складності;
- використовувати OpenCV primitive як назву canonical stage;
- hard-code algorithm choice у stage implementation без `C`.

## Fields / Interface

Registry використовується такими configuration fields:

```yaml
fields:
  - name: "stage.enabled"
    type: "bool"
    required: true
    meaning: "Чи бере stage участь у pipeline run."
  - name: "stage.variant"
    type: "string enum"
    required: true
    meaning: "Назва implementation family із цього registry."
  - name: "stage.level"
    type: "string enum"
    required: true
    meaning: "Complexity/performance class із `dp1.config.complexity_levels`."
  - name: "stage.parameters"
    type: "ParameterMap"
    required: true
    meaning: "Параметри, дозволені stage-interface card або future stage spec."
```

## Input / Output

Input:

- `PipelineConfig`;
- stage-interface cards у `canonical/stages/*.md`;
- `dp1.config.complexity_levels`.

Output:

- допустима пара `stage.variant + stage.level`;
- constraint для future stage spec і code-generation task.

## Constraints

- Registry не замінює stage-interface cards.
- Registry не замінює future stage specs.
- `variant` може бути зареєстрований тут, але залишатися неготовим для code
  generation, доки немає stage spec.
- `Lx` не є escape hatch: він потребує explicit validation і task-card scope.

## Failure cases

- Stage implementation обирає algorithm за `cv::Mat::type()` замість
  `PipelineConfig`.
- `tiles` трактується як окремий ninth stage.
- `L1` трактується як `variant`.
- `candidate_extraction` напряму видає `MeasurementRecord`.
- `measurement` читає visualization/debug image як photometry source.

## Typical misuse

- Додавати новий variant у stage card, але не оновлювати registry.
- Використовувати різні назви одного stage між config, stage card і pipeline
  binding.
- Змішувати spelling `matched_filter` і `matched_filtering` без alias policy.

## Open questions

- Формальна таблиця сумісності `profile -> stage -> variant -> level`.
- Повні schemas `parameters` для кожного variant.
- Alias policy для коротких config keys, якщо вони відрізняються від canonical
  stage ids.

## Connections

- constrains: dp1.config.pipeline_configuration_c
- uses: dp1.config.complexity_levels
- constrains: dp1.pipeline.stage_contract
- constrains: dp1.pipeline.stage_domain_bindings
- constrains: dp1.pipeline.stage_io_matrix
- links: dp1.stage.prep
- links: dp1.stage.radiometric_correction
- links: dp1.stage.enhancement
- links: dp1.stage.matched_filtering
- links: dp1.stage.candidate_extraction
- links: dp1.stage.segmentation_refinement
- links: dp1.stage.object_filtering
- links: dp1.stage.measurement
