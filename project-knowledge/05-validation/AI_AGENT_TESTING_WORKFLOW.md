# AI_AGENT_TESTING_WORKFLOW

## Призначення

Цей документ фіксує стандартний алгоритм test run для AI-агента.
Для командної роботи цей документ є canonical source правил і послідовності виконання тестування Amanita + Comparator.

Scope цього документа:
- застосовується лише до test execution і test run orchestration;
- не є універсальною policy для звичайних задач розробки.

Політика незмінності production коду під час test run:
- під час execution test runs заборонено змінювати production код Amanita або Comparator;
- дозволені лише per-run конфіги/скрипти в run-директорії, запуск бінарників/CLI та збір артефактів/звітів.

Базове правило Amanita run:
- за замовчуванням тестуються DP1 і DP2 разом в одному stage;
- окреме тестування лише одного модуля (тільки DP1 або тільки DP2) можливе лише за явним узгодженням.

## 1. Розташування тестів

Коренева директорія тестів:
- `${AMANITA_RESOURCES_DIR}/tests`

Для кожного run має бути окрема директорія:
- `${AMANITA_RESOURCES_DIR}/tests/<TestId>`

Кожен новий `TestId` має виконуватись незалежно і стартувати з нульового стану.
Заборонено перевикористовувати staged конфіги або результати попередніх test runs.
Для нового run конфіги готуються заново per-run скриптом у `Temp/`.

Формат `TestId`:
- `код_задачі_YYYYMMDD_HHMMSS_номер_рана`
- приклад: `AMNT-0005_20260325_213500_01`

## 2. Обов'язкова структура test run

У кожному run обов'язково мають бути:
- `Configs/` - конфіги Amanita і Comparator для цього run
- `RunResults/` - артефакти виконання
- `Logs/` - stdout/stderr, timing та checks по кожному кроку
- `Temp/` - тимчасові файли та per-run скрипти зміни конфігів

У `Configs/` обов'язкова деталізація по підсистемах і запусках:
- `Configs/Amanita/DP1/<StageId>/...`
- `Configs/Amanita/DP2/<StageId>/...`
- `Configs/Comparator/<StageId>/...`

Для multi-run сценаріїв кожен запуск має власний `<StageId>` і власні піддиректорії в `Configs/*` та `Logs/*`.

У `RunResults/` очікується як мінімум:
- `RunResults/Amanita/DP1`
- `RunResults/Amanita/DP2`
- `RunResults/Comparator`

У корені run мають лежати звіти.
Мінімально обов'язковий файл:
- `<TestId>_Summary.md` - генерується автоматично на основі артефактів Amanita і Comparator

Мінімальний набір даних у `<TestId>_Summary.md` (генерується автоматично, default-профіль):
- id тесту (`TestId`);
- дата тесту;
- датасет;
- короткий опис тесту;
- мінімальні дані по DP1 і DP2:
	- загальна кількість знайдених об'єктів на всіх кадрах (парсується з DP1 JSON);
	- кількість відстежених траєкторій DP2;
	- час виконання;
- дані звіту Comparator (кількість пар, mean overlap);
- коротке summary за результатами тесту.

За потреби перед стартом test run може бути погоджений альтернативний (custom) набір полів Summary.
У такому випадку:
- default-структура лишається базовою точкою відліку;
- генерація виконується per-run скриптами для поточного запуску;
- такі скрипти мають зберігатися лише в `Temp/` відповідного run (не як універсальні репозиторні скрипти).

## 3. Порядок виконання (strict sequence)

Кроки виконуються строго послідовно, якщо користувач явно не вказав інше:
1. Перевірка готового Python-оточення для Comparator.
2. Пересборка Amanita через `builder/build_dp1_dp2.sh` (за замовчуванням `Debug`, потім `Release`, якщо не узгоджено інакше).
3. Підготовка/модифікація конфігів Amanita і Comparator.
4. Запуск Amanita (DP1 + DP2 як єдиний stage).
5. Валідація адекватності артефактів Amanita.
6. Запуск Comparator.
7. Валідація артефактів Comparator.
8. Автоматична генерація `<TestId>_Summary.md` за допомогою `test.agent/scripts/generate_summary.sh`.
9. Загальний аналіз результатів на основі згенерованого підсумкового звіту.

Правило завершення Amanita stage:
- для file-based джерел DP1 (`imagefile`, `videofile`) нормальне завершення по EOF має завершуватись із success-кодом `0`;
- non-zero exit code у Amanita stage вважається помилкою і обробляється через stop-on-failure.

Правило precheck для кроку 1:
- перед будь-яким Comparator stage обов'язково перевірити, чи є готове робоче Python-оточення Comparator;
- якщо оточення відсутнє або невалідне, test run зупиняється;
- далі агент не виконує автоналаштування без підтвердження користувача, а пропонує налаштувати оточення.

Для execution використовувати строго stage-скрипти з репозиторію:
- `test.agent/scripts/run_amanita_stage.sh`
- `test.agent/scripts/run_comparator_stage.sh`

Наступний крок дозволено запускати лише після повного завершення попереднього кроку (blocking wait-until-exit).

Структура `<TestId>_Summary.md` є стандартизованою та формується автоматично скриптом `generate_summary.sh`.
Перед запуском дозволено погодити custom-набір полів Summary; тоді використовуються per-run скрипти з `Temp/` поточного test run.

Результат кроку 9 обов'язково:
- відображається в чаті;
- дублюється в самому кінці `<TestId>_Summary.md` в секції `Result summary`.

Формат інтерпретації для кроку 9 (обов'язковий):
- загальний статус (`PASS`/`FAIL`) і короткий висновок про придатність run для baseline/candidate-порівняння;
- стабільність між stage (для multi-run: ключові відмінності або підтвердження відсутності відмінностей);
- якість детекції/порівняння за метриками Comparator:
  - `comparator_mean_overlap_pct`;
  - `comparator_mean_rms_deviation_area`;
  - `comparator_mean_false_positives_pct`;
  - `comparator_mean_false_negatives_pct`;
- інтерпретація продуктивності (часи DP1/DP2/Comparator, ознаки деградації або стабільності);
- явні ризики та наступний крок (що потрібно зробити для повної валідації, якщо це smoke-run або synthetic-run).

Для smoke/synthetic multi-run обов'язково додавати дисклеймер, що candidate може бути технічною копією baseline і метрики не відображають реальну алгоритмічну дельту.

Мовна узгодженість звітів:
- один конкретний Summary має бути написаний однією мовою без змішування мов;
- пріоритетна мова для Summary і підсумкових повідомлень: українська (якщо користувач явно не попросив іншу).

Для multi-run сценаріїв (наприклад baseline -> candidate-A -> candidate-B) запуски також виконуються послідовно, а результат зводиться в один підсумковий звіт, якщо не задано інше.

Правило агрегації multi-run у `generate_summary.sh`:
- якщо передано `--stage-id`, генерується Summary для вказаного stage;
- якщо `--stage-id` не передано і в run знайдено кілька stage, формується один aggregated Summary по всіх stage (на основі `Logs/Amanita/*` та `Logs/Comparator/*`);
- у multi-run Summary додається загальний блок агрегованих метрик і окремі блоки по кожному stage.

Примітка: цей workflow є універсальним і не прив'язаний до конкретної feature-задачі 

## 4. Політика зупинки на помилках

Якщо будь-який крок завершується помилкою:
- негайно зупинити сценарій;
- зафіксувати причину в run-директорії (`FAILED.txt` або еквівалент);
- не продовжувати наступні кроки;
- чекати явних вказівок користувача.

Для невідомих non-zero exit кодів:
- використовувати stop-and-wait за замовчуванням.

Після fail без явного дозволу користувача заборонено:
- виконувати autonomous recovery;
- повторно запускати pipeline;
- змінювати конфіги для обходу помилки.

## 5. Політика скриптів

Універсальні скрипти для запуску зберігаються в репозиторії:
- `test.agent/scripts/run_amanita_stage.sh`
- `test.agent/scripts/run_comparator_stage.sh`
- `test.agent/scripts/generate_summary.sh` - автоматична генерація `<TestId>_Summary.md`

Скрипти зміни конфігів мають бути per-run, не універсальні, і зберігатися у `Temp/` відповідного run.

### Використання generate_summary.sh

Скрипт `generate_summary.sh` автоматично генерує підсумковий звіт на основі артефактів від Amanita і Comparator stage-ів:

```bash
test.agent/scripts/generate_summary.sh \
  --run-root <path> \
  --test-id <id> \
  [--stage-id <id>] \
  [--dataset <name>] \
  [--description <text>] \
  [--test-date "YYYY-MM-DD HH:MM:SS"]
```

Параметри:
- `--run-root`: кореневий каталог тестового запуску
- `--test-id`: ідентифікатор тесту (у форматі `AMNT-XXXX_YYYYMMDD_HHMMSS_NN`)
- `--stage-id`: ідентифікатор stage-ю (опціонально, авто-визначається зі структури `Logs/*`)
- `--dataset`: назва датасету (опціонально, авто-визначається з Comparator report)
- `--description`: короткий опис тесту (опціонально, буде дефолтний текст)
- `--test-date`: дата/час тесту (опціонально, за замовчуванням поточні дата/час)

Для custom-набору полів Summary:
- використовувати окремі per-run скрипти генерації/пост-обробки;
- розміщувати їх у `Temp/` поточного test run;
- не зберігати custom-скрипти як універсальні скрипти репозиторію.

Поведіка для multi-run без явного `--stage-id`:
- `generate_summary.sh` автоматично збирає всі stage з `Logs/Amanita/<StageId>` та `Logs/Comparator/<StageId>`;
- формує один `<TestId>_Summary.md` з:
  - загальним підсумком multi-run;
  - секціями `Stage <StageId>` для кожного запуску.

Результат:
- Файл `<TestId>_Summary.md` у корені `<run-root>` з обов'язковими полями:
  - id тесту, дата, датасет, опис
  - метрики DP1: `dp1_total_objects_all_frames`, `dp1_execution_time_sec`
  - метрики DP2: `dp2_total_objects_all_frames`, `dp2_execution_time_sec`
  - агрегати Comparator: `comparator_execution_time_sec`, `comparator_report_files`, `comparator_frames_compared_total`, `comparator_mean_overlap_pct`, `comparator_mean_rms_deviation_area`, `comparator_mean_false_positives_pct`, `comparator_mean_false_negatives_pct`
  - деталізація `comparator_reports` по кожному report-файлу
  - висновок
