# PROJECT_INDEX

## Призначення

Це кореневий індекс knowledge base для проєкту `clt2-optic_amanita`. Файл описує сам проєкт, його модулі, високорівневий pipeline, структуру knowledge base та рекомендовані точки входу для людини або AI-агента.

На відміну від `AGENTS.md`, цей файл не задає процесні правила роботи агента, а пояснює, що це за система, з яких частин вона складається і де шукати знання по конкретній підсистемі.

---

## 1. Коротко про проєкт

`clt2-optic_amanita` - це багатокомпонентний проєкт комп'ютерного зору для обробки оптичних даних. За структурою репозиторію він включає модулі захоплення кадрів, первинної обробки, наступної обробки результатів, сервісні й візуалізаційні компоненти, інструменти збірки та тестові mock-модулі.

Ключова виробнича зв'язка для knowledge base зараз:
- `camerapro` - захоплення або передача кадрів
- `datapro1` - DP1, первинна обробка кадру
- `datapro2` - DP2, наступний етап обробки результатів від DP1
- `common/*` - спільні моделі, frame utilities, turret/common logic
- `gtests` і `test.mock/*` - перевірка та ізольоване тестування каналів і модулів
- `builder/*`, `docker-compose.yml`, `overseer/*` - збірка, orchestration, remote/automation tooling

---

## 2. High-level pipeline

На поточному рівні знань high-level pipeline можна читати так:

1. Джерело кадрів або upstream-компонент формує вхідні дані.
2. `camerapro` та/або runner-и подають кадри в DP1.
3. `datapro1` виконує первинну обробку кадру, segmentation/measurement pipeline та формує результат кадру.
4. DP1 за потреби:
   - надсилає результат у DP2 через TCP/RPC-подібний канал
   - зберігає `.blob` та/або `.json`
   - відображає або записує відео/діагностичні артефакти
5. `datapro2` приймає результати DP1 і виконує власний етап обробки, агрегації або трекінгу.
6. Допоміжні модулі, GUI та mock-компоненти використовуються для налагодження, валідації та інтеграційних сценаріїв.

---

## 3. Поточний стан knowledge base

На цей момент knowledge base найбільш повно покриває DP1:
- типи даних і метадані кадру
- frame processor boundary
- runtime buffers для IPC/URI ingest
- конфігурацію DP1
- tile limiting і snail path
- serialization primitives і DP1 -> DP2 handoff
- файловий вивід `.blob` / `.json`

DP2 покриває не лише базові типи, а й runtime boundaries та receive path DP1 -> DP2.

Validation layer має окремий розділ `05-validation/` з canonical workflow для AI-agent test runs.

Спільні протоколи винесені в `04-protocols/PROTOCOLS_INDEX.md` як окремий вхідний індекс.

---

## 4. Структура knowledge base

```text
project-knowledge/
  00-governance/
    CARD_TEMPLATE.md
    KNOWLEDGE_BASE_ROADMAP.md
	CODE_STYLE.md
	TESTING_POLICY.md
	TASK_CARD_TEMPLATE.md
  01-project/
    PROJECT_ECOSYSTEM.md
  02-dp1/
    DP1_INDEX.md
    DP1_CARDS_INDEX.md
    cards/*.md
  03-dp2/
    DP2_INDEX.md
    DP2_CARDS_INDEX.md
    cards/*.md
  04-protocols/
    PROTOCOLS_INDEX.md
  05-validation/
    VALIDATION_INDEX.md
    AI_AGENT_TESTING_WORKFLOW.md
  06-tasks/
	cards/
	TASKS_INDEX.md
    ...
```

Ролі розділів:
- `00-governance/` - правила формату, шаблони, план розвитку knowledge base
- `01-project/` - загальна інформація про проєкт і екосередовище
- `02-dp1/` - знання про DP1
- `03-dp2/` - знання про DP2
- `04-protocols/` - shared contracts між модулями
- `05-validation/` - перевірка, mock-и, regression cases, test assets
- `06-tasks/` - task cards для конкретних задач, їх меж, ризиків і коротких результатів

---

## 5. Точки входу по темах

### 5.1. Якщо потрібно зрозуміти проєкт загалом
Читати:
- цей файл
- `01-project/PROJECT_ECOSYSTEM.md`
- кореневий `README.md`
- кореневий `CMakeLists.txt`

### 5.2. Якщо задача про DP1
Читати:
- `02-dp1/DP1_INDEX.md`
- `02-dp1/DP1_CARDS_INDEX.md`
- картки з `02-dp1/cards/`
- код `datapro1/src/*`

### 5.3. Якщо задача про DP2
Читати:
- `03-dp2/DP2_INDEX.md`
- `03-dp2/DP2_CARDS_INDEX.md`
- картки з `03-dp2/cards/`
- код `datapro2/src/*`
- конфіг `datapro2/config/*`

### 5.4. Якщо задача про shared contracts, serialization, мережу або file exchange
Читати:
- `04-protocols/PROTOCOLS_INDEX.md`
- відповідні картки DP1/DP2
- код handoff/serialization у DP1 і код прийому в DP2

### 5.5. Якщо задача про збірку, Docker, залежності або цільову ОС
Читати:
- `01-project/PROJECT_ECOSYSTEM.md`
- `builder/Dockerfile`
- `docker-compose.yml`
- `builder/*.sh`
- `README.md`

### 5.6. Якщо задача про тестування, еталонні прогони або аналіз результатів
Читати:
- `05-validation/VALIDATION_INDEX.md`
- `05-validation/AI_AGENT_TESTING_WORKFLOW.md`
- `00-governance/TESTING_POLICY.md`
- `06-tasks/TASKS_INDEX.md`

---

## 6. DP1: поточний обсяг знань

DP1 наразі є найкраще описаною підсистемою. Відомі такі основні групи знань:
- вхід кадру і boundary між runner-ом та processor-ом
- формування `TDataRes` і суміжних структур
- tile-based processing
- конфігурація DP1
- серіалізація результатів
- DP1 -> DP2 transport
- локальний файловий вивід

Для DP1 knowledge base вже придатна як стартова опора для аналізу коду, внесення змін і подальшого вирощування карток.

---

## 7. DP2: поточний статус

Для DP2 сформовано робочий набір карток і окремий індекс:
- `03-dp2/DP2_CARDS_INDEX.md`
- `03-dp2/cards/*.md`

Поточне покриття фокусується на:
- core структурах трекінгу (`Measurement`, `PTPoint`, `TStrobe`, `Trajectory`)
- конфігураційних структурах (`binocular_cfg`, `dp2strobe_mth_cfg`, `dp2_cfg`, `turret_exch_cfg`)
- runtime boundaries (`server`, `session`, `dp2_rpc_cl`)
- receive path DP1 -> DP2
- фіксації перетинів із DP1-типами через `Connections`

Наступний крок розвитку розділу - деталізація turret exchange lifecycle, reconnect/failure сценаріїв і подальша синхронізація з `04-protocols/`.

---

## 8. Де зберігати інформацію про екосередовище

Уся інформація про мовний стек, build system, бібліотеки, версії, Docker, ОС, конфігураційні формати та важливі зовнішні залежності повинна жити в:
- `01-project/PROJECT_ECOSYSTEM.md`

Це потрібно, щоб не розмазувати environment knowledge по DP1/DP2-картках і не змішувати доменні знання з build/runtime knowledge.

---

## 9. Найближчі пріоритети розвитку knowledge base

1. Уточнити й нормалізувати поточні DP1-картки.
2. Поглибити DP2-картки по turret exchange lifecycle і failure handling.
3. Розширити `04-protocols/` конкретними shared wire/file contract cards.
4. Розвивати `05-validation/`: test cards, regression scenarios, mapping на datasets.
