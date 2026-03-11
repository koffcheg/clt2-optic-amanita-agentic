# PROJECT_ECOSYSTEM

## Призначення

Цей файл фіксує екосередовище проєкту: мови, стандарти, бібліотеки, toolchain, цільову ОС, build/runtime infrastructure та authoritative source files, з яких це підтверджується.

Його задача - бути єдиним місцем для environment knowledge, щоб AI-агент і розробник не збирали цю інформацію кожного разу з усього репозиторію заново.

---

## 1. Цільова платформа

Поточний репозиторій і build-інфраструктура орієнтовані насамперед на Linux-середовище.

Підтвердження:
- `builder/Dockerfile` використовує `ubuntu:22.04`
- `docker-compose.yml` орієнтує збірку на контейнеризований Linux workflow
- `README.md` містить Linux-команди, пакети `apt`, POSIX queue paths (`/dev/mqueue`, `/dev/shm`)

Робоча базова ОС для відтворюваної збірки:
- Ubuntu 22.04 у Docker build container

---

## 2. Мови та стандарти

### Основний код
- C++ - основна мова проєкту
- C++20 - використовується в кореневій CMake-конфігурації для більшості модулів
- C++17 - окремо зафіксовано для модуля `manager`
- C - присутній як secondary language в `manager`

### Допоміжні мови
- Python - використовується в `overseer/`
- Bash / shell scripting - build і orchestration scripts у `builder/`, `overseer/`, `install/`
- CMake - основна build-конфігурація
- JSON/XML - формати конфігурації та частини runtime input

### Важливе зауваження
Проєкт містить змішане середовище стандартів C++: загальний корінь - C++20, але `manager` збирається з C++17. Це потрібно враховувати при зміні shared code, інтерфейсів та compile assumptions.

---

## 3. Основні бібліотеки та залежності

### Computer vision / image processing
- OpenCV 4.9.0
- використовується `opencv_world`
- у Dockerfile вмикаються `xfeatures2d` і `OPENCV_ENABLE_NONFREE`

### Logging
- Apache log4cxx 1.3.1

### Utility / formatting / geometry
- Boost 1.85.0
- fmt 11.0.2
- Clipper2 1.4.0

### Data/config / parsing
- Jansson

### Runtime / system
- `pthread`
- POSIX IPC primitives згадуються в `README.md` через `/dev/mqueue` і `/dev/shm`

### Testing
- GoogleTest присутній у `gtests/`

---

## 4. Build system

### Основна збірка
- CMake є головною системою збірки
- у корені встановлено `cmake_minimum_required(VERSION 3.16)`
- для `manager` локально вказано `3.18`

### Containerized build
Підтримується через:
- `builder/Dockerfile`
- `docker-compose.yml`
- `builder/build_all.sh`
- `builder/build_datapro1.sh`
- `builder/build_datapro2.sh`
- інші `builder/*.sh`

### Локальна manual build path
Описана в кореневому `README.md`.

---

## 5. Підсистеми репозиторію

### Основні production-like модулі
- `camerapro`
- `datapro1`
- `datapro2`
- `manager`
- `calibration`
- `viewer`
- `turretpro`
- `turretgui`
- `plotter`
- `targetsim`

### Shared code
- `common/frame`
- `common/model`
- `common/turret`
- `common/utils`

### Validation / test / mocks
- `gtests`
- `test.mock/cam-pro-mock`
- `test.mock/datapro1-mock`
- `test.mock/datapro2-mock`
- `test.mock/dp2-data-gen`
- `test.mock/turretstate`

### Automation / orchestration
- `builder/`
- `overseer/`

---

## 6. Важливі runtime/formats knowledge

### Конфігурація
- JSON і XML є ключовими форматами конфігу
- приклади лежать у `*/config/`

### Мережа / обмін даними
- є TCP/RPC-подібний handoff між DP1 і DP2
- деталі форматів треба описувати не тут, а в `04-protocols/`

### File outputs
- щонайменше для DP1 використовуються `.blob` і `.json`
- детальний опис форматів має жити в DP1/Protocols knowledge, а не в цьому файлі

---

## 7. Authoritative source files для ecosystem knowledge

Коли потрібно перевірити або оновити environment knowledge, дивитися в такому порядку:

1. `CMakeLists.txt` у корені та в підмодулях
2. `builder/Dockerfile`
3. `docker-compose.yml`
4. `builder/*.sh`
5. кореневий `README.md`
6. `*/config/*`

---

## 8. Правила підтримки цього файлу

Оновлювати `PROJECT_ECOSYSTEM.md` потрібно, якщо змінюється хоча б одна з таких речей:
- версія або набір ключових бібліотек
- цільова ОС або build container
- стандарт C++ або compile assumptions
- структура модулів репозиторію
- build/run workflow
- важливі конфігураційні формати

Не слід дублювати сюди алгоритмічний опис DP1/DP2. Для цього існують `PROJECT_INDEX.md` і тематичні картки.
