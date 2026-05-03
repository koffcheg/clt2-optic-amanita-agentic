---
id: dp1.stage_spec.candidate_extraction.pilot
title:
  uk: "Пілотна мала специфікація candidate extraction"
  en: "Pilot small specification for candidate extraction"
tags: [dp1, canonical, stage-spec, small-tz, pilot]
kind: stage-spec-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stage_specs/dp1.stage_spec.candidate_extraction.pilot.md"
  lines: "1-N"
status: "draft"
---

## Definition

Пілотне мале ТЗ для `ICandidateExtractionStage`.

Ця картка визначає пілотний цільовий маршрут для майбутньої валідації реалізації, без твердження про поточну runtime-підтримку.

## Scope

- Інтерфейс stage: `ICandidateExtractionStage`.
- Слот pipeline: `candidate_extraction`.
- Рівень складності: `L0`.
- Варіант: `global_threshold`.

## Inputs

- Мапа detector response з upstream stage.
- Дозволені типи: `CV_32FC1` (пріоритетний) або `CV_8UC1`.
- Вхід має представляти одноканальний response у геометрії processing domain.

## Outputs

- Бінарна candidate mask типу `CV_8UC1`.
- Колекція candidate hypotheses, отримана з connected components маски.
- Hypotheses є попередніми та не мають трактуватися як валідовані об'єкти.

## Preconditions

- Існує config fragment `candidate_extraction`.
- Для execution path встановлено `enabled=true`.
- Для цієї pilot spec встановлено `variant=global_threshold` і `level=L0`.
- Параметр threshold існує та є числовим.

## Deterministic flow

1. Валідувати config keys і пару variant-level.
2. Нормалізувати подання входу:
   - якщо `CV_8UC1`, конвертувати за internal scalar range policy;
   - якщо `CV_32FC1`, використовувати безпосередньо.
3. Застосувати global threshold для формування binary mask.
4. Запустити connected-component extraction для генерації hypotheses.
5. Емітити mask і набір hypotheses з явною семантикою "not validated".

## Configuration contract

Канонічний фрагмент `C`:

```json
{
  "candidate_extraction": {
    "enabled": true,
    "variant": "global_threshold",
    "level": "L0",
    "parameters": {
      "threshold": "<number>",
      "min_area": "<int, optional>"
    }
  }
}
```

Правила:
- Відсутній `enabled|variant|level|parameters.threshold` => configuration error.
- Невідомі keys у `candidate_extraction.parameters` => warning + ignore.
- Політика діапазону для `threshold` має бути явно зафіксована в implementation config notes.

## Invariants

- Вихідна mask є одноканальною `CV_8UC1` із бінарними значеннями.
- Candidate hypotheses формуються лише з produced mask.
- Stage не має виконувати downstream validation/classification.
- Stateful background models заборонені в цьому pilot variant.

## Failure cases

- Threshold занадто високий -> майже порожня mask і втрата candidates.
- Threshold занадто низький -> шумна mask і переповнення candidates.
- Невідповідність типу -> порушення контракту і stage failure path.

## Non-goals

- Визначення поведінки adaptive thresholding (`L1`).
- Визначення поведінки stateful background extraction (`L2`).
- Фінальна object validation і measurement.

## Validation hooks

- Кількість candidates на frame.
- Коефіцієнт заповнення mask.
- Трасування значення threshold.
- Час stage для threshold + connected components.

## Source-of-truth and canonicalization safety

Джерело цільової канонічної архітектури:
- ця картка визначає intended behavior для canonical implementation.

Спостережувані runtime/build/config/protocol факти:
- мають бути верифіковані за code/config/protocol artifacts перед твердженням
  про поточну runtime-поведінку.
- Ця картка не стверджує, що маршрут наразі реалізований або
  валідований у `datapro1_v2`.

Правило безпеки:
- якщо code/runtime behavior розходиться з цією карткою, потрібно повідомити
  про невідповідність, класифікувати її як "canonical gap" або "runtime deviation"
  і запропонувати мінімальні reconciliation-оновлення; не зводити відмінність мовчки.

## Connections

- Картка stage interface: `../stages/dp1.stage.candidate_extraction.md`
- Канонічний індекс: `../DP1_CANONICAL_INDEX.md`
- Маршрут pipeline/config: `../pipeline/dp1.pipeline.stage_contract.md`,
  `../configuration/dp1.config.pipeline_configuration_c.md`
