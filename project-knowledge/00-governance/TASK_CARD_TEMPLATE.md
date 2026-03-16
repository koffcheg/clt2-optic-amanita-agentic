# TASK_CARD_TEMPLATE.md

<!-- AGENT INSTRUCTIONS: Before creating a new task card:
  1. Open project-knowledge/06-tasks/TASKS_INDEX.md
  2. Find the highest existing AMNT-XXXX number
  3. Increment it by 1 to get the new id
  4. Use that id in the filename (e.g. AMNT-0002.md) and in the `id` field below
  Never use placeholder ids like AMNT-0000.
-->

---
id: AMNT-XXXX
title: Коротка назва задачі
status: draft
type: task-card
priority: normal
created_by: agent
created_at: YYYY-MM-DD
updated_at: YYYY-MM-DD
related_user_request: Короткий опис запиту користувача
scope: local | medium | broad
domain_area:
  - dp1
  - dp2
  - protocols
  - ecosystem
related_files: []
related_cards: []
approval_status: pending
---

## Summary

Короткий опис задачі людською мовою.
Що саме потрібно зробити, без зайвих деталей.

## Goal

Який очікуваний результат задачі.

## In scope

Що входить у межі задачі.

## Out of scope

Що не входить у межі задачі і не повинно змінюватися без окремого узгодження.

## User constraints

Явні обмеження від користувача.
Наприклад:
- не змінювати knowledge base без узгодження
- не створювати тести без узгодження
- не робити широкі рефакторинги
- не додавати нові залежності

## Assumptions

Припущення, з якими агент починає роботу.
Непідтверджені факти потрібно явно позначати тут.

## Relevant context to read

Список файлів, які треба прочитати перед виконанням задачі.

Наприклад:
- `project-knowledge/PROJECT_INDEX.md`
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`
- `project-knowledge/04-protocols/PROTOCOLS_INDEX.md`
- `datapro1/src/...`

## Planned changes

Які саме зміни очікуються.
Краще коротким списком по суті.

## Files expected to be touched

Які файли, ймовірно, будуть змінюватися.

## Risks

Які є ризики:
- порушення контрактів
- зміна формату даних
- вплив на build/run
- ризик побічних змін
- неповний knowledge context

## Validation approach

Як буде перевірятися результат.

Наприклад:
- review by code reading
- local build
- manual scenario
- test plan only
- no validation yet

## Knowledge-base impact

Чи зачіпає задача knowledge base.

Можливі значення:
- none
- maybe
- yes

Якщо `maybe` або `yes`, треба вказати:
- які саме картки або індекси потенційно застарівають
- чи потрібне окреме узгодження на їх оновлення

## Execution notes

Короткі нотатки по ходу виконання.
Без перетворення картки на великий лог.

## Result

Що фактично зроблено.

## Follow-up

Що залишилося на майбутнє або потребує окремого узгодження.