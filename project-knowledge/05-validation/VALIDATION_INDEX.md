# VALIDATION_INDEX

## Призначення

Цей індекс є точкою входу в знання про тестування, валідацію і regression-сценарії.

Для задач AI-agent тестування canonical source:
- `AI_AGENT_TESTING_WORKFLOW.md`

## Документи розділу

- `AI_AGENT_TESTING_WORKFLOW.md` - обов'язковий алгоритм виконання test run для Amanita + Comparator.

## Пов'язані документи

- `project-knowledge/00-governance/TESTING_POLICY.md` - governance-правила тестування та межі дій агента.
- `project-knowledge/06-tasks/TASKS_INDEX.md` - task cards з контекстом конкретних validation задач.

## Пов'язані інструменти

- `test.agent/scripts/run_amanita_stage.sh` - універсальний запуск етапу Amanita.
- `test.agent/scripts/run_comparator_stage.sh` - універсальний запуск етапу Comparator.
- `test.agent/scripts/generate_summary.sh` - автоматична генерація підсумкового звіту `<TestId>_Summary.md`.
