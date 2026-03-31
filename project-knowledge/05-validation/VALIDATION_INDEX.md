# VALIDATION_INDEX

## Призначення

Цей індекс є точкою входу в знання про тестування, валідацію і regression-сценарії.

Цей розділ також фіксує практичні сценарії валідації для змін у DP1/DP2, коли automated tests не створюються в межах задачі.

Для задач AI-agent тестування canonical source:
- `AI_AGENT_TESTING_WORKFLOW.md`

## Документи розділу

- `AI_AGENT_TESTING_WORKFLOW.md` - обов'язковий алгоритм виконання test run для Amanita + Comparator.

## Поточні картки

- `validation.amnt0004.dp1_binning` - валідація фічі сумуючого бінування в DP1 (OFF/ON, sweep, негативні кейси)
  link: `cards/validation.amnt0004.dp1_binning.md`

## Пов'язані документи

- `project-knowledge/00-governance/TESTING_POLICY.md` - governance-правила тестування та межі дій агента.
- `project-knowledge/06-tasks/TASKS_INDEX.md` - task cards з контекстом конкретних validation задач.

## Пов'язані інструменти

- `test.agent/scripts/run_amanita_stage.sh` - універсальний запуск етапу Amanita.
- `test.agent/scripts/run_comparator_stage.sh` - універсальний запуск етапу Comparator.
- `test.agent/scripts/generate_summary.sh` - автоматична генерація підсумкового звіту `<TestId>_Summary.md`.
