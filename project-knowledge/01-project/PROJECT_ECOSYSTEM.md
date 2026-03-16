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

### Універсальний build-playbook для AI-агента

Мета цього блоку: дати відтворюваний алгоритм збірки без прив'язки до конкретного користувача або машини.

#### Правило підтвердження команд

**Перед кожною командою, що змінює стан системи або файлової системи, агент зобов'язаний явно запитати підтвердження у користувача.**

Це стосується без винятків:
- встановлення або оновлення системних пакетів (`apt-get install`, `apt-get upgrade`, ...)
- збірки та встановлення бібліотек зі сирців
- `cmake` configure (перший запуск або перезапуск з іншими параметрами)
- `cmake --build` (або будь-яка команда build)
- будь-яких операцій запису у файлову систему поза директорією репозиторію

Команди **тільки для читання** (пошук, перевірка версій, `dpkg -l`, `find`, `cat`, `cmake --debug-find-pkg`) можна виконувати без запиту — вони безпечні та оборотні.

Заборонено виконувати кілька кроків підряд між двома підтвердженнями. Одне підтвердження = один крок.

---

#### Крок 1. Перевірка наявності залежностей (read-only, без підтвердження)

Системні пакети (наявність):
```
dpkg -l | grep -E 'libeigen3|libopenexr|libjansson'
```

Бібліотеки з custom prefix — пошук cmake config-файлів:
```
find / -name "OpenCVConfig.cmake" 2>/dev/null
find / -name "log4cxxConfig.cmake" 2>/dev/null
find / -name "Clipper2Config.cmake" 2>/dev/null
find / -name "fmtConfig.cmake" 2>/dev/null
find / -name "boost_filesystem-config.cmake" -o -name "BoostConfig.cmake" 2>/dev/null
```

За результатами зафіксувати знайдені prefix-и або відзначити відсутність.

---

#### Крок 2. Перевірка версій знайдених бібліотек

Мінімальні вимоги проєкту:
| Бібліотека | Мінімальна версія |
|---|---|
| Boost | 1.85.0 |
| OpenCV | 4.9.0 (рекомендовано static `opencv_world`) |
| log4cxx | 1.3.1 |
| Clipper2 | не задана, перевірити `ClipperVersion.h` |
| fmt | 11.0.2 (приблизно) |
| jansson | ≥ 2.x (системний пакет) |

Визначити версію знайденого cmake-пакету:
```
grep -r "version" <FOUND_PREFIX>/lib/cmake/<Pkg>/<Pkg>ConfigVersion.cmake 2>/dev/null
```

---

#### Крок 3. Встановлення відсутніх системних пакетів (з підтвердженням)

Якщо системні залежності відсутні, виконувати **тільки після підтвердження від користувача**:
```
sudo apt-get update
sudo apt-get install -y libeigen3-dev libopenexr-dev libjansson-dev
```

---

#### Крок 4. Збірка відсутніх custom-prefix бібліотек (з підтвердженням)

Якщо бібліотеки (Boost, OpenCV, log4cxx, Clipper2, fmt) не знайдені:
1. Ознайомитися з інструкцією збірки в `README.md` проєкту — там є canonical build steps.
2. Підготувати і показати команди для підтвердження.
3. Виконувати **по одному кроку** з підтвердженням перед кожним.

**Не виконувати** кілька кроків без проміжних підтверджень.

---

#### Крок 5. Configure (з підтвердженням)

Перед кожним запуском cmake configure показати команду користувачу і дочекатися підтвердження.

Якщо OpenCV (static `opencv_world`) вимагає транзитивні CMake-targets `Eigen3::Eigen` або `OpenEXR::OpenEXR`, а вони не визначені, застосувати non-invasive workaround — до cmake configure створити тимчасовий cmake-файл і передати через `-DCMAKE_PROJECT_INCLUDE=`:
```cmake
# /tmp/opencv_deps.cmake
find_package(Eigen3 CONFIG REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)
```

Шаблон configure (узагальнений):
```
cmake -S <SRC_DIR> -B <BUILD_DIR> \
  -DBoost_NO_SYSTEM_PATHS=ON \
  -DBOOST_ROOT=<BOOST_PREFIX> \
  -Dlog4cxx_DIR=<LOG4CXX_PREFIX>/lib/cmake/log4cxx \
  -DOpenCV_DIR=<OPENCV_PREFIX>/lib/cmake/opencv4 \
  -DClipper2_DIR=<CLIPPER2_PREFIX>/lib/cmake/clipper2 \
  [-DCMAKE_PROJECT_INCLUDE=<opencv_deps.cmake>]
```

---

#### Крок 6. Build (з підтвердженням)

Показати команду, дочекатися підтвердження:
```
cmake --build <BUILD_DIR> -j<N> --target camerapro datapro1 datapro2 turretpro manager
```

`<N>` — кількість паралельних потоків, рекомендовано 4–8.

---

### Типові точки відмови для агентів

- Boost версія: у системі може бути старіша версія, ніж вимагає проєкт. Агент має явно обмежувати пошук Boost до потрібного prefix.
- OpenCV (static `opencv_world`): можливі транзитивні CMake-target залежності (`Eigen3::Eigen`, `OpenEXR::OpenEXR`) у середовищі, де вони не створені автоматично.
- Для таких випадків без зміни коду проєкту дозволений non-invasive workaround на рівні configure: `-DCMAKE_PROJECT_INCLUDE=<temporary.cmake>` з pre-load `find_package(Eigen3 CONFIG REQUIRED)` та `find_package(OpenEXR CONFIG REQUIRED)`.
- Якщо збірка відтворюється тільки з workaround, агент має зафіксувати це в task card і не змінювати кодову базу без окремого узгодження.

### Правило універсальності

Будь-які приклади з конкретними шляхами (`/home/<user>/...`) розглядати лише як локальні приклади.
Authoritative практика для knowledge base: формулювати команди через placeholders та крок пошуку конфігів через `find`/`cmake --debug-find-pkg=<Pkg>`.

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
