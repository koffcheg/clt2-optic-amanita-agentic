---
id: "<canonical.id>"
title:
  uk: "<Українська назва>"
  en: "<English title>"
tags: [tag1, tag2, tag3]
source:
  file: "path/to/file"
  lines: "10-42"
status: "draft"
---

## Definition
Коротке визначення сутності.

## Assumptions
Які припущення робляться щодо lifetime, ownership, thread-safety, типів даних, формату, викликів тощо.

## Theorem / Contract
Що гарантується. Які вхідні/вихідні умови. Які обмеження не можна порушувати.

## Interpretation
Як цю сутність слід читати в архітектурі проєкту. Яку роль вона виконує.

## Failure cases
Типові збої, некоректні стани, проблеми portability, undefined behavior, format mismatch, lifetime issues.

## Typical misuse
Типові помилки використання.

## Open questions
Питання, які ще не підтверджені кодом або потребують перевірки.

## Connections
- used_by: <other.card.id>
- produces: <other.card.id>
- overlaps_with: <other.card.id>
