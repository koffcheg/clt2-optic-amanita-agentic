# ICandidateExtractionStage_SMALL_TZ

## 1) Meta

- Stage name: `ICandidateExtractionStage`
- Related task card: `AMNT-0010`
- Author: agent
- Date: 2026-05-02
- Status: draft

## 2) Context

`ICandidateExtractionStage` розглядається як downstream stage у DP1 stage-driven потоці, що перетворює попередньо підготовлені проміжні результати у набір кандидатів для наступних етапів segmentation, object filtering та measurement.

## 3) Objective

Сформувати мале ТЗ, яке:
- фіксує мінімальний робочий контур stage-level задачі;
- задає чіткі межі in/out scope;
- визначає acceptance і validation рамку без змін production-коду.

## 4) In scope

- формалізація expected input/output рамки на рівні task-артефакту;
- опис preconditions/postconditions у нейтральній, non-contract-breaking формі;
- підготовка бази для подальшої кодової верифікації тверджень.

## 5) Out of scope

- зміна інтерфейсів stage у коді;
- зміна wire/RPC/file контрактів;
- зміни build, runtime orchestration, Docker/CI;
- canonical update існуючих DP1 stage cards у межах цього pilot.

## 6) Inputs

- Вхідні типи/структури: upstream проміжні результати DP1 (точні структури мають бути підтверджені по коду перед canonicalization).
- Джерела даних: попередній stage pipeline DP1.
- Preconditions:
  - upstream stage завершив обробку кадру/тайлів у валідному стані;
  - конфігураційні параметри stage доступні та узгоджені з runtime режимом.

## 7) Outputs

- Вихідні типи/структури: набір candidate entities для downstream stage(s) (точні типи уточнюються після code-level verification).
- Postconditions:
  - результати придатні для наступного stage без додаткової нормалізації поза визначеним контрактом;
  - відсутні побічні зміни глобального стану поза межами очікуваного життєвого циклу stage.
- Побічні артефакти: не передбачено цим ТЗ.

## 8) Constraints

- Contract constraints: не змінювати публічні stage/protocol контракти без окремого узгодження.
- Performance/latency constraints: не вводити додаткових heavy-операцій у hot path без обґрунтування.
- Memory/lifecycle constraints: уникати необґрунтованого розширення hot-path структур.
- Tooling/process constraints: knowledge-only execution, без змін code/build/test infra.

## 9) Acceptance criteria

- [ ] Stage-level мале ТЗ для `ICandidateExtractionStage` створено у визначеному шляху.
- [ ] Документ містить in/out scope, inputs/outputs, constraints, risks, open questions.
- [ ] Документ явно позначено як draft/pilot і non-canonical для domain knowledge.

## 10) Validation approach

- Команди/кроки перевірки:
  - review markdown structure;
  - перевірка відповідності обмеженням AMNT-0010;
  - diff-check тільки по погоджених knowledge файлах.
- Очікуваний результат: knowledge-only пакет сформовано без змін за межами scope.
- Blocker: будь-яка потреба змінити code/build/contracts без явного узгодження.

## 11) Risks

- ризик неточності формулювань inputs/outputs до code-level верифікації;
- ризик інтерпретації pilot як фінального контрактного опису.

## 12) Open questions

- Які exact C++ типи та ownership/lifecycle semantics використовуються на межі цього stage?
- Які точні інваріанти якості candidate-набору очікує downstream stage?

## 13) References

- `AGENTS.md`
- `project-knowledge/00-governance/CANONICALIZATION_POLICY.md`
- `project-knowledge/00-governance/STAGE_SMALL_TZ_TEMPLATE.md`
- `project-knowledge/06-tasks/cards/AMNT-0010.md`
