---
id: dp1.validation.stage_contract_checks
title:
  uk: "Перевірки контрактів етапів DP1"
  en: "DP1 stage contract checks"
tags: [dp1, canonical, validation, stage-contract, code-generation]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/02-dp1/canonical/validation/dp1.validation.stage_contract_checks.md"
status: "draft"
---

## Definition

Ця картка визначає knowledge-level checklist перевірок, які future code
generation task має врахувати для кожної stage implementation. Вона не створює
автоматизовані тести і не визначає runtime test workflow.

## Assumptions

- Автоматизовані тести не створюються в межах цієї knowledge-only підготовки.
- Практичне виконання checks має бути описане окремо у validation execution
  route або implementation task.
- Stage specs можуть додавати variant-specific checks.

## Theorem / Contract

Для кожної stage implementation мають бути перевірки:

```yaml
stage_contract_checks:
  - check: "StageConfig contains enabled, variant, level, parameters"
    source: "dp1.config.pipeline_configuration_c"
  - check: "stage.variant exists in dp1.config.stage_variant_registry"
    source: "dp1.config.stage_variant_registry"
  - check: "stage.level is compatible with dp1.config.complexity_levels"
    source: "dp1.config.complexity_levels"
  - check: "input carrier matches dp1.pipeline.stage_io_matrix"
    source: "dp1.pipeline.stage_io_matrix"
  - check: "output carrier matches stage-interface card"
    source: "canonical/stages/*.md"
  - check: "cv::Mat invariants pass before OpenCV primitive call"
    source: "dp1.domain.opencv_invariants"
  - check: "no implicit conversion changes domain without metadata"
    source: "dp1.domain.conversion_rules"
  - check: "FrameContext is not used as hidden output owner"
    source: "dp1.domain.runtime.frame_context"
  - check: "Visualization domain is not used as computation input"
    source: "dp1.domain.visualization"
  - check: "StageTiming/ProfileEvent is recorded or explicitly skipped by policy"
    source: "dp1.domain.common_types"
```

Tile-route checks:

```yaml
tile_route_checks:
  - check: "TileDesc.tile_id is unique inside frame"
  - check: "TileDesc.roi_with_border is inside FramePacket geometry"
  - check: "TileDesc.valid_area is inside TileRawView.image extent"
  - check: "TileRawView.image is non-owning ROI view"
  - check: "TileRawView.image may be non-continuous"
  - check: "Tile-local outputs are cropped by valid_area before acceptance"
  - check: "Tile-local coordinates are converted to FrameGlobal before final output"
  - check: "TileResult does not carry image buffers"
  - check: "Duplicate suppression policy is applied or marked blocking"
```

Measurement-boundary checks:

```yaml
measurement_checks:
  - check: "MeasurementRecord has frame_id, camera_id/source relation, time_ref"
  - check: "MeasurementRecord coordinate_space is FrameGlobal for DP1 -> DP2 handoff"
  - check: "Photometry source is Raw/Input or explicitly allowed Processing domain"
  - check: "DP1 -> DP2 handoff does not require debug images, masks, or temporary buffers"
  - check: "Payload schema/version policy is present or marked blocking"
```

## Fields / Interface

Checklist entries мають такі поля:

```yaml
fields:
  - name: "check"
    meaning: "Що має бути перевірено."
  - name: "source"
    meaning: "Canonical source card для правила."
  - name: "scope"
    meaning: "Stage-level, tile-route, measurement-boundary або pipeline-level."
```

## Input / Output

Input:

- canonical stage-interface cards;
- data-domain cards;
- configuration cards;
- pipeline Stage I/O Matrix.

Output:

- checklist для future implementation and review;
- перелік blocking open questions для code-generation readiness.

## Constraints

- Ця картка не створює test files.
- Ця картка не замінює `05-validation` execution workflow.
- Якщо check потребує stage-specific algorithm details, він має бути перенесений
  у future stage spec.

## Failure cases

- Code generation починається без перевірки registered variant.
- Stage приймає `CV_8UC1` без розрізнення Raw U8 і MaskU8.
- Tile-local measurement потрапляє у DP2 handoff без globalization.
- Missing duplicate suppression policy приховується замість explicit blocker.

## Typical misuse

- Трактувати checklist як automated test suite.
- Закривати open questions неявними assumptions у code.
- Використовувати цю картку як дозвіл змінювати source code.

## Open questions

- Exact runtime mechanism для reporting failed checks.
- Які checks мають бути compile-time/static, а які runtime.
- Як `05-validation` має materialize ці checks у майбутніх validation routes.

## Connections

- uses: dp1.pipeline.stage_contract
- uses: dp1.pipeline.stage_io_matrix
- uses: dp1.config.stage_variant_registry
- uses: dp1.domain.opencv_invariants
- uses: dp1.domain.common_types
- uses: dp1.domain.conversion_rules
- uses: dp1.domain.coordinates
- uses: dp1.domain.measurement.record
- extends: dp1.validation.canonical_conformance
