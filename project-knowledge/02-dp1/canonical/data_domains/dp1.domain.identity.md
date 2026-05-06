---
id: dp1.domain.identity
title: "Canonical-політика ідентичності DP1"
tags: [dp1, canonical, data-domain, identity, code-generation]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.identity.md"
status: "draft"
---

## Definition

Ця картка визначає canonical policy ідентичності для DP1 domain structures.

Canonical identity не має залежати тільки від локального numeric id, якщо об'єкт
може потрапити у multi-camera, multi-run або parallel tile route.

## Assumptions

- Один DP1 pipeline run має stable `pipeline_run_id`.
- Один DP1 instance може мати один або кілька source/camera identifiers.
- Tile workers можуть створювати локальні ids паралельно, але merge має
  зберегти deterministic relation до source frame і source tile.

## Theorem / Contract

Canonical identity model:

```text
pipeline_run_id + camera_id + frame_id + local_id
```

де:

- `pipeline_run_id` ідентифікує запуск pipeline/config.
- `camera_id` ідентифікує source або DP1 instance.
- `frame_id` ідентифікує кадр у межах source/run.
- `local_id` ідентифікує domain object у межах кадру або stage output.

Для економії пам'яті `pipeline_run_id` не обов'язково дублювати у кожному
`Candidate`, `Segment`, `ValidatedObject` або `MeasurementRecord`, якщо він
доступний через `FrameContext`. Проте semantics id мають трактуватися саме як
складений identity tuple.

Рекомендовані local id fields:

- `candidate_id`;
- `segment_id`;
- `object_id`;
- `measurement_id`;
- `tile_id`;
- `component_id`.

Local ids мають бути bounded numeric values, а не free-form strings.

## Interpretation

Ця policy дозволяє паралельним workers генерувати компактні local ids і водночас
не втрачати global traceability. Для debug/reporting повний identity tuple може
бути reconstructed із object fields і `FrameContext`.

## Failure cases

- `candidate_id` вважається globally unique без `frame_id` і `camera_id`.
- Tile worker створює ids, які конфліктують після merge.
- `pipeline_run_id` не збережений у context/logs, тому output неможливо
  відтворити.

## Typical misuse

- Використовувати string ids у hot path.
- Дублювати довгий `pipeline_run_id` у кожному small object без потреби.

## Open questions

- Exact deterministic allocation policy для tile-local ids.
- Чи має `frame_id` бути monotonic per source або externally provided.

## Connections

- constrains: dp1.domain.raw.frame_packet
- constrains: dp1.domain.runtime.frame_context
- constrains: dp1.domain.struct.candidate
- constrains: dp1.domain.struct.segment
- constrains: dp1.domain.struct.validated_object
- constrains: dp1.domain.measurement.record
