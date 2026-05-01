---
id: dp1.canonical.source_of_truth
title:
  uk: "Джерела істини canonical DP1"
  en: "Canonical DP1 sources of truth"
tags: [dp1, canonical, source-of-truth]
kind: governance-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/product/dp1.canonical.source_of_truth.md"
  lines: "1-120"
status: "draft"
---

## Definition

Ця картка визначає, які джерела знань можуть керувати проєктуванням і генерацією коду для canonical DP1.

## Assumptions

Task cards контролюють доступ до legacy-матеріалів.

## Theorem / Contract

`source_role` керує придатністю картки як джерела для canonical-проєктування і
майбутньої генерації коду.

Дозволені canonical-джерела:
- картки canonical-доменів даних;
- картки інтерфейсів етапів;
- майбутні специфікації етапів;
- модель конфігурації `C`;
- canonical-протокол передачі DP1 -> DP2;
- картки перевірки відповідності.

Заборонені джерела цільової архітектури:
- legacy-код;
- legacy-reference cards;
- неформальний текст, який не формалізований як картка або специфікація етапу.

Картки з `source_role: canonical` використовуються за замовчуванням як
canonical-джерела, якщо вони містять формальний контракт і якщо наявні потрібні
`Stage_Spec` та конфігурація `C`.

Картки з `source_role: legacy-reference` не використовуються для генерації коду,
якщо активна task card явно не дозволяє legacy access і не перелічує точні
legacy-джерела.

## Interpretation

Якщо потрібного джерела немає, agent має зупинитися і запропонувати відсутню картку або специфікацію етапу.

## Failure cases

- Код генерується з нотатки roadmap.
- Legacy-поведінка копіюється як canonical-дизайн без дозволу task card.

## Typical misuse

- Трактувати `DP1_CARDS_INDEX.md` як canonical-джерело після розділення legacy і canonical.

## Open questions

- Які майбутні Stage_Spec-картки треба створити першими для canonical DP1.

## Connections

- uses: dp1.canonical.product_definition
- uses: dp1.pipeline.stage_contract
