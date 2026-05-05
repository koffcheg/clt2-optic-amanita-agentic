---
id: dp1.pipeline.stage_domain_bindings
title:
  uk: "Прив'язка етапів DP1 до доменів і структур"
  en: "DP1 stage to domain/structure bindings"
tags: [dp1, canonical, pipeline, stage-contract, data-domain]
kind: pipeline-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/pipeline/dp1.pipeline.stage_domain_bindings.md"
  lines: "1-N"
status: "draft"
---

## Definition

Ця картка задає canonical matrix прив'язки DP1 stages до дозволених input/context/output domains і structures.

Мета картки — показати AI-кодеру, які структури дозволені на вході та виході кожного етапу.

## Assumptions

- Це knowledge-only binding matrix, а не claim про поточну реалізацію в `datapro1_v2`.
- Конкретні algorithm variants мають уточнюватися у stage specs.
- Tile-local execution structures є target execution-support model, а не claim про поточний runtime.

## Theorem / Contract

Кожен stage має працювати за загальною формою:

```text
process(input, context, config) -> output
```

де:
- `input` має належати дозволеному input domain/structure для цього stage;
- `context` має бути runtime context, а не контейнером stage output;
- `output` має бути explicit domain/structure output;
- internal buffers не мають ставати canonical output без явного domain contract.

## Canonical stage-domain binding matrix

| Stage | Дозволений input | Runtime context | Дозволений output | Примітки |
|---|---|---|---|---|
| `prep` | `dp1.domain.raw.frame_packet` | `dp1.domain.runtime.frame_context` | `dp1.domain.raw.frame_packet` або `dp1.domain.processing.frame` | Підготовка source data та ROI/tile route. Без candidates/masks/measurements. |
| `radiometric_correction` | `dp1.domain.raw.frame_packet` або `dp1.domain.processing.frame` | `dp1.domain.runtime.frame_context` | `dp1.domain.processing.frame` | Формує corrected/residual processing representation. |
| `enhancement` | `dp1.domain.processing.frame` | `dp1.domain.runtime.frame_context` | `dp1.domain.processing.frame` | Покращує processing representation. |
| `matched_filtering` | `dp1.domain.processing.frame` | `dp1.domain.runtime.frame_context` | `dp1.domain.processing.frame` | Detector/response representation лишається в Processing domain. |
| `candidate_extraction` | `dp1.domain.processing.frame` | `dp1.domain.runtime.frame_context` | `dp1.domain.mask.binary_mask` + `dp1.domain.struct.candidate` | Candidate є provisional, а не validated object. |
| `segmentation_refinement` | `dp1.domain.mask.binary_mask` + optional `dp1.domain.struct.candidate` | `dp1.domain.runtime.frame_context` | `dp1.domain.struct.segment` | Segment уточнює candidate/region. |
| `object_filtering` | `dp1.domain.struct.candidate` або `dp1.domain.struct.segment` | `dp1.domain.runtime.frame_context` | `dp1.domain.struct.candidate` або `dp1.domain.struct.segment` | Filtering змінює acceptance/quality, але не формує measurement payload. |
| `measurement` | `dp1.domain.struct.segment` + optional `dp1.domain.raw.frame_packet` або `dp1.domain.processing.frame` для photometry | `dp1.domain.runtime.frame_context` | `dp1.domain.measurement.record` | Фінальний продуктовий output DP1 для DP1 -> DP2 handoff. |
| `visualization` | Будь-який explicit domain object, потрібний для display/debug | `dp1.domain.runtime.frame_context` | visualization artifact у `dp1.domain.visualization` | Visualization output не має подаватися назад у computation без explicit stage spec. |

## Tile-local execution binding

Tile-local execution support structures можуть використовуватися runtime/execution model без зміни semantic stage contracts:

| Runtime structure | Роль | Не можна використовувати як |
|---|---|---|
| `dp1.domain.runtime.tile_desc` | опис tile/ROI, border, valid area | image buffer або stage output |
| `dp1.domain.runtime.tile_context` | per-tile/per-worker buffers і diagnostics | global mutable state або final output |
| `dp1.domain.runtime.tile_result` | explicit tile-local results перед merge | final DP1 output без merge |

Tile-local outputs мають бути обрізані за valid area, перетворені в global coordinates і merged перед тим, як стати frame-level `MeasurementRecord` output.

## Interpretation

Stage cards визначають межі відповідальності. Data-domain cards визначають дозволені semantic objects. Ця binding matrix з'єднує обидва шари, щоб implementation task могла обмежити, що кожен stage має право читати і видавати.

Stage specs можуть звужувати дозволені domains, але не мають розширювати їх без оновлення цієї binding matrix або окремо погодженої task card.

## Failure cases

- `candidate_extraction` напряму видає `MeasurementRecord`.
- `FrameContext` зберігає candidates, segments, masks або measurements як прихований output.
- Tile execution пише напряму в global measurement output без `TileResult` і merge semantics.
- Visualization image використовується як computation input.

## Typical misuse

- Вважати `cv::Mat` type достатнім для вибору stage input domain.
- Передавати `Candidate` або `Segment` у stages, які очікують Processing або Mask domain.
- Використовувати runtime context як заміну explicit input/output contracts.

## Open questions

- Exact stage naming synchronization із code-level names.
- Чи має `object_filtering` вводити окрему accepted-object structure після MVP.
- Чи має `segmentation_refinement` приймати тільки `Candidate` + mask або також mask-only routes.

## Connections

- uses: dp1.pipeline.stage_contract
- uses: dp1.domain.raw.frame_packet
- uses: dp1.domain.runtime.frame_context
- uses: dp1.domain.processing.frame
- uses: dp1.domain.mask.binary_mask
- uses: dp1.domain.struct.candidate
- uses: dp1.domain.struct.segment
- uses: dp1.domain.measurement.record
- informs: dp1.stage.candidate_extraction
- informs: dp1.stage.segmentation_refinement
- informs: dp1.stage.object_filtering
- informs: dp1.stage.measurement
