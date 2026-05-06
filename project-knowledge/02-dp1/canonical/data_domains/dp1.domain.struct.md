---
id: dp1.domain.struct
title:
  uk: "Struct домен DP1"
  en: "DP1 Struct domain"
tags: [dp1, canonical, data-domain, struct]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.struct.md"
status: "draft"
---

## Definition

Struct domain описує семантичну роль структурованих проміжних об'єктів DP1 після mask-level extraction і перед final measurement.

Цей домен містить explicit structures для гіпотез, уточнених областей, object-like records і результатів filtering.

## Assumptions

- Об'єкти Struct domain не є фінальним DP1 -> DP2 measurement payload.
- Конкретні C++ representations мають визначатися в окремих implementation tasks.
- `Candidate`, `Segment` і `ValidatedObject` є canonical structures у цьому
  домені.

## Theorem / Contract

Для Struct domain діють такі правила:

- об'єкти мають мати explicit identity і source relation;
- geometry має вказувати coordinate system або relation до source frame;
- candidates є provisional hypotheses, а не validated objects;
- segments є refined regions, а не final measurements;
- validated objects є результатом object filtering, але не final measurement
  payload;
- filtering decisions не мають втрачати source traceability, якщо інше явно не визначено stage spec.

Canonical structures:

- `dp1.domain.struct.candidate`.
- `dp1.domain.struct.segment`.
- `dp1.domain.struct.validated_object`.

## Interpretation

Struct domain є мостом між pixel/mask semantics і measurement semantics. Він не дозволяє pipeline кодувати весь object state тільки в masks або unstructured lists.

## Failure cases

- `Candidate`, `Segment` або `ValidatedObject` трактується як final measurement.
- Object geometry втрачає relation до frame/source mask.
- Stage передає unstructured rectangles без identity/source metadata.

## Typical misuse

- Ховати candidates або segments у `FrameContext`.
- Кодувати object state тільки через mask pixels.

## Open questions

- Standard quality flags для struct-domain objects.

## Connections

- has_structure: dp1.domain.struct.candidate
- has_structure: dp1.domain.struct.segment
- has_structure: dp1.domain.struct.validated_object
- derived_from: dp1.domain.mask
- feeds: dp1.domain.measurement
