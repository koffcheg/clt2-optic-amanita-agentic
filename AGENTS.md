# AGENTS.md

## Призначення

Цей файл задає базові правила роботи AI-агента в репозиторії `clt2-optic_amanita` та описує, як орієнтуватися в knowledge base. Він не дублює повний опис проєкту, а маршрутизує агента до потрібних файлів і фіксує правила синхронізації коду та документації.

## Очікуване розміщення knowledge base

Knowledge base має лежати в корені репозиторію в каталозі `project-knowledge/`.

Робоча схема:
- `AGENTS.md` - правила роботи агента і маршрутизація
- `project-knowledge/PROJECT_INDEX.md` - карта проєкту і точка входу в domain knowledge
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md` - екосередовище, стек, збірка, залежності, ОС
- `project-knowledge/02-dp1/` - знання про DP1
- `project-knowledge/03-dp2/` - знання про DP2
- `project-knowledge/04-protocols/` - міжмодульні контракти, wire formats, IPC/TCP/RPC
- `project-knowledge/05-validation/` - тести, mock-и, regression scenarios, validation notes
- `project-knowledge/00-governance/CODE_STYLE.md` - правила стилю коду
- `project-knowledge/00-governance/TESTING_POLICY.md` - правила тестування
- `project-knowledge/06-tasks/TASKS_INDEX.md` - індекс task cards
- `project-knowledge/06-tasks/cards/` - task cards для конкретних задач
- `project-knowledge/00-governance/TASK_CARD_TEMPLATE.md` - шаблон task card

## Ієрархія джерел істини

Якщо між джерелами є конфлікт, агент має орієнтуватися в такому порядку:

1. Явні інструкції користувача в поточній задачі.
2. `AGENTS.md`.
3. `project-knowledge/00-governance/CODE_STYLE.md`.
4. `project-knowledge/00-governance/TESTING_POLICY.md`.
5. Код, `CMakeLists.txt`, `.h/.hpp/.cpp/.cc`, конфігураційні JSON/XML, Dockerfile, shell-скрипти.
6. Build/run-артефакти репозиторію: `README.md`, `builder/`, `docker-compose.yml`, `overseer/`, `config/`.
7. Knowledge base у `project-knowledge/`.
8. Старі нотатки, чернетки, неактуальні описи.


Якщо knowledge base суперечить коду, код вважається authoritative source.
Агент не повинен автоматично виправляти knowledge base без явного узгодження.
Він повинен описати розбіжність, вказати зачеплені файли і запропонувати зміни.

## Обмеження та контроль змін

### Обмеження на зміни knowledge base

Knowledge base є контрольованим артефактом проєкту.

Агенту заборонено без явного узгодження:
- редагувати `project-knowledge/PROJECT_INDEX.md`
- створювати, змінювати, перейменовувати або видаляти knowledge cards
- змінювати структуру knowledge base
- редагувати шаблони карток
- редагувати roadmap-файли
- виконувати масову синхронізацію або реструктуризацію конспекту
- редагувати `CODE_STYLE.md`, `TESTING_POLICY.md` або `PROJECT_ECOSYSTEM.md` без явного узгодження

Якщо агент виявив, що knowledge base застаріла, суперечить коду або неповна, він повинен:
1. описати проблему
2. вказати зачеплені файли
3. запропонувати точний перелік змін
4. дочекатися явного узгодження

Виняток:
агенту дозволено створювати та оновлювати task cards у `project-knowledge/06-tasks/cards/` для поточної погодженої задачі, якщо це потрібно правилами workflow.
Task cards не вважаються доменними knowledge cards і не замінюють оновлення knowledge base.

### Обмеження на зміни коду

Агент повинен надавати перевагу мінімальним, локальним і легко перевірюваним змінам.

Агенту заборонено без явного узгодження:
- виконувати широкі рефакторинги
- змінювати архітектуру
- змінювати публічні контракти або формати обміну
- додавати нові бібліотеки, фреймворки або зовнішні інструменти
- змінювати CI/CD, Docker, build pipeline або deployment logic
- масово перейменовувати файли, символи або директорії

### Обмеження на тестування

Агенту заборонено без явного узгодження:
- створювати нові automated tests
- створювати fixtures, mocks, stubs, snapshots або golden files
- додавати тестову інфраструктуру
- масово переписувати наявні тести
- змінювати тестову стратегію проєкту

Під час execution test runs (Amanita/Comparator) агенту заборонено змінювати production код Amanita або Comparator.
Дозволені лише:
- підготовка per-run конфігів і скриптів у run-директорії
- запуск бінарників/CLI
- збір логів, артефактів і звітів

Якщо агент бачить, що тести потрібні, він повинен не створювати їх автоматично, а описати test plan і дочекатися узгодження.

## Обов'язковий старт для агента

Перед будь-якою нетривіальною задачею агент повинен:

1. Перевірити, чи існує task card для поточної задачі.
2. Якщо task card відсутня і задача є нетривіальною, створити нову task card у `project-knowledge/06-tasks/cards/` за шаблоном `project-knowledge/00-governance/TASK_CARD_TEMPLATE.md`.
3. Заповнити в task card: опис задачі, межі задачі, обмеження, релевантні файли, очікувані зміни та ризики.
4. Прочитати `project-knowledge/PROJECT_INDEX.md`.
5. Прочитати релевантний індекс підсистеми або тематичний файл.
6. Перевірити твердження по коду, якщо задача стосується runtime behavior, контрактів або формату даних.
7. Після змін перевірити, чи зачеплено knowledge base, контракти, пайплайн або екосередовище.
8. Якщо knowledge base потребує оновлення, не змінювати її автоматично без явного узгодження, а підготувати перелік потрібних змін.

## Маршрути читання по типу задачі

### 1. Загальна архітектура, склад модулів, high-level pipeline
Читати:
- `project-knowledge/PROJECT_INDEX.md`
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`
- кореневий `README.md`
- кореневий `CMakeLists.txt`

### 2. Збірка, залежності, Docker, ОС, toolchain
Читати:
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`
- `README.md`
- `builder/Dockerfile`
- `docker-compose.yml`
- `builder/*.sh`
- відповідні `CMakeLists.txt`

### 3. DP1: ingest, frame processing, tile pipeline, output
Читати:
- `project-knowledge/02-dp1/DP1_INDEX.md`
- `project-knowledge/02-dp1/DP1_CARDS_INDEX.md`
- `project-knowledge/02-dp1/cards/*.md`
- `datapro1/src/*`
- `datapro1/config/*`

### 4. DP2: receiving, aggregation, post-processing, tracking
Читати:
- `project-knowledge/03-dp2/DP2_INDEX.md`
- `project-knowledge/03-dp2/DP2_CARDS_INDEX.md`
- `project-knowledge/03-dp2/cards/*.md`
- `datapro2/src/*`
- `datapro2/config/*`
- спільні протокольні файли з `project-knowledge/04-protocols/`

### 5. Міжмодульні контракти, TCP/RPC, серіалізація, файли обміну
Читати:
- `project-knowledge/04-protocols/PROTOCOLS_INDEX.md`
- релевантні картки в DP1/DP2
- `datapro1/src/dp1_tr_res2dp2.cpp`
- `datapro1/src/dp1_rpc_data_mrsh.cpp`
- код прийому в DP2

### 6. Тести, mock-и, перевірка сценаріїв
Читати:
- `project-knowledge/05-validation/`
- `gtests/`
- `test.mock/`
- `overseer/`, якщо задача стосується orchestration або remote runs

### 7. Код-стайл, правила змін коду, тестові обмеження
Читати:
- `AGENTS.md`
- `project-knowledge/00-governance/CODE_STYLE.md`
- `project-knowledge/00-governance/TESTING_POLICY.md`

### 8. Поточна задача та її межі
Читати:
- `project-knowledge/06-tasks/TASKS_INDEX.md`
- поточну task card `project-knowledge/06-tasks/cards/AMNT-XXXX.md`
- `AGENTS.md`
- за потреби релевантні domain files

## Коли knowledge base потребує оновлення

Knowledge base вважається такою, що потребує оновлення, якщо агент:
- додає нову сутність, яка впливає на архітектуру або контракти
- змінює формат даних, serialization, network payload або file output
- змінює build/runtime environment, залежності або toolchain
- змінює high-level pipeline або responsibilities модуля
- знаходить, що існуюча картка суперечить коду

За замовчуванням агент не повинен виконувати таке оновлення автоматично.

Натомість він повинен:
- вказати, які саме файли knowledge base потребують змін
- коротко описати причину
- запропонувати мінімальний обсяг синхронізації
- дочекатися явного узгодження

Мінімальний обсяг потенційної синхронізації може включати:
- релевантну картку або нову картку
- `PROJECT_INDEX.md`, якщо змінився ландшафт знань або структура розділів
- `PROJECT_ECOSYSTEM.md`, якщо змінилася екосереда

## Правила створення нових карток

Створення нових карток дозволене лише за явним узгодженням або якщо поточна задача прямо вимагає оновлення knowledge base.

У проєкті існують два типи карток:
- domain cards - картки знань про систему, контракти, структури, пайплайни та середовище
- task cards - картки конкретних задач виконання

Правила цього розділу для ID-просторів `project.*`, `dp1.*`, `dp2.*`, `protocols.*`, `validation.*` стосуються domain cards.
Task cards використовують окремий формат ID: `AMNT-0001`, `AMNT-0002`, ...

### Принципи
- 1 картка = 1 сутність або 1 вузький контракт.
- Не змішувати в одній картці структуру, алгоритм і мережевий протокол, якщо це різні поняття.
- Якщо інформація не підтверджена кодом, позначати її в `Assumptions` або `Open questions`.
- Якщо сутність є shared між DP1 і DP2, картка має жити в `04-protocols/` або мати звідти canonical index.

### Правила для task cards
- 1 task card = 1 задача
- filename = task id
- формат ID: `AMNT-XXXX`
- task card створюється перед виконанням нетривіальної задачі
- task card містить опис задачі, межі, обмеження, плановані зміни, ризики та validation approach
- task card не є authoritative source для domain knowledge

### Іменування
Рекомендовані простори імен для ID:
- `project.*` - загальна архітектура та проєктні сутності
- `ecosystem.*` - стек, build, runtime, dependencies
- `dp1.types.*`, `dp1.frame.*`, `dp1.runtime.*`, `dp1.rpc.*`, `dp1.net.*`, `dp1.io.*`, `dp1.tiles.*`, `dp1.config.*`
- `dp2.types.*`, `dp2.runtime.*`, `dp2.net.*`, `dp2.track.*`, `dp2.io.*`, `dp2.config.*`
- `protocols.*` - shared wire/file/message contracts
- `validation.*` - test assets, scenarios, regressions

### Мінімальний шаблон картки
Кожна нова картка має містити:
- YAML front matter з `id`, `title`, `tags`, `source`, `status`
- `Definition`
- `Assumptions`
- `Theorem / Contract`
- `Interpretation`
- `Failure cases`
- `Typical misuse`
- `Open questions` - якщо потрібно
- `Connections`

Шаблон лежить у `project-knowledge/00-governance/CARD_TEMPLATE.md`.

## Де фіксувати інформацію про екосередовище

Усе, що стосується мов, стандартів, бібліотек, ОС, Docker, build tools, конфігураційних форматів і runtime dependencies, треба фіксувати в:
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`

Туди ж входить:
- список мов і стандартів
- основні бібліотеки та їх роль
- цільова ОС і build container
- build system і мінімальні команди
- authoritative source files для environment knowledge

Якщо інформація детальна і велика, її можна виносити в окремі дочірні документи, наприклад:
- `project-knowledge/01-project/BUILD_AND_RUN.md`
- `project-knowledge/01-project/DEPENDENCIES.md`
- `project-knowledge/01-project/REPO_LAYOUT.md`

## Поведінка агента при розбіжностях або нестачі знань

Якщо knowledge base неповна, агент не повинен вигадувати факти.

Він має:
- перевірити код і конфігураційні файли
- описати, чого саме бракує
- вказати, які картки або індекси слід оновити
- явно позначити `Open questions`
- запропонувати створення або оновлення картки як `draft`, але не робити цього без узгодження

## Definition of done для документаційної частини задачі

Цей блок застосовується лише до задач, у яких оновлення документації було явно погоджене.

Документаційна частина вважається завершеною, якщо:
- нові або змінені технічні факти підтверджені кодом
- оновлено релевантні картки
- оновлено індекси, якщо змінилася навігація
- оновлено `PROJECT_ECOSYSTEM.md`, якщо торкнулися середовища
- у knowledge base немає тихих конфліктів між кодом і текстом
