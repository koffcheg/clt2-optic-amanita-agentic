# clt2-optic

Система оптичного виявлення цілей на базі комп'ютерного зору.

## 1. Коротко про проєкт

Репозиторій містить кілька модулів, але поточне ядро розробки та запуску:
- `datapro1` (DP1) — первинна обробка кадру.
- `datapro2` (DP2) — прийом і подальша обробка результатів DP1.

Інші модулі збережені в репозиторії, але для щоденного workflow варто орієнтуватися насамперед на DP1/DP2.

## 2. Структура репозиторію

Основні каталоги:
- `datapro1/` — код і конфіги DP1.
- `datapro2/` — код і конфіги DP2.
- `common/` — спільні моделі, утиліти й типи.
- `builder/` — скрипти збірки, включно з цільовим скриптом DP1/DP2.
- `project-knowledge/` — knowledge base, політики та task cards.
- `test.mock/` — мок-програми для ізольованих сценаріїв.
- `gtests/` — unit-тести (GoogleTest).

Системно-корисні шляхи Linux (IPC):
- `/proc/sys/fs/mqueue/msg_max`
- `/dev/mqueue/`
- `/dev/shm/`

## 3. Resource Root (datasets + run outputs)

Проєкт використовує єдиний корінь ресурсів для вхідних даних і результатів запуску.

### 3.1. Базовий конфіг у репозиторії

Файл:
- `amanita_resources.conf`

Приклад:
```ini
resources_root=AmanitaResources
```

Значення може бути:
- відносним до кореня репозиторію;
- або абсолютним шляхом.

### 3.2. Локальний override

Для конкретної машини використовується:
- `amanita_resources.local.conf`

Цей файл не має потрапляти в git.

### 3.3. Optional env override

За потреби можна перевизначити корінь через змінну:
- `AMANITA_RESOURCES_DIR`

### 3.4. Порядок резолву шляху

1. `AMANITA_RESOURCES_DIR` (якщо задана).
2. `amanita_resources.local.conf`.
3. `amanita_resources.conf`.
4. fallback: `<repo_root>/AmanitaResources`.

## 4. Рекомендована структура ресурсів

```text
AmanitaResources/
  datasets/
    raw/
      video/
      frames/
    calibration/
  runs/
    dp1/
    dp2/
  archives/
```

Що куди класти:
- Вхідні відео/кадри: `datasets/raw/...`
- Калібрувальні файли: `datasets/calibration/...`
- Виходи DP1: `runs/dp1/...`
- Виходи DP2: `runs/dp2/...`

## 5. Швидкий старт: збірка DP1/DP2

Цільовий скрипт:
- `builder/build_dp1_dp2.sh`

Команди:
```bash
./builder/build_dp1_dp2.sh Debug
./builder/build_dp1_dp2.sh Release
```

Результати збірки:
- `build-dp1dp2-debug/`
- `build-dp1dp2-release/`

Якщо залежності встановлені у custom-prefix, можна передати підказки:
```bash
BOOST_ROOT=/path/to/boost-1.85.0 \
LOG4CXX_DIR=/path/to/log4cxx/lib/cmake/log4cxx \
OPENCV_DIR=/path/to/opencv/lib/cmake/opencv4 \
./builder/build_dp1_dp2.sh Debug
```

## 6. Запуск DP1/DP2

Рекомендований порядок: спочатку DP2, потім DP1.

### 6.1. Запуск DP2

```bash
./build-dp1dp2-debug/datapro2/datapro2 \
  ./build-dp1dp2-debug/datapro2/config/config_datapro2.json \
  ./build-dp1dp2-debug/datapro2/config/dp2_log.xml
```

### 6.2. Запуск DP1 (camera index = 0)

```bash
./build-dp1dp2-debug/datapro1/datapro1 0 \
  ./build-dp1dp2-debug/datapro1/config/config_datapro1.json \
  ./build-dp1dp2-debug/datapro1/config/dp1_log.xml
```

## 7. Налаштування датасету кадрів у DP1

Для запуску на послідовності кадрів у `config_datapro1.json`:
- `source.source = "imagefile"`
- `source.link = "${AMANITA_RESOURCES_DIR}/datasets/raw/frames/<dataset>/C001_F%06d.png"`

Для відео:
- `source.source = "videofile"`
- `source.link = "${AMANITA_RESOURCES_DIR}/datasets/raw/video/<file>.mp4"`

Приклад output-папки DP1:
- `test.out_folder = "${AMANITA_RESOURCES_DIR}/runs/dp1/<run_name>/"`

## 8. Docker-збірка

Підготувати контейнер:
```bash
./builder/prepare_docker.sh
```

Зібрати весь проєкт:
```bash
docker compose run build
```

Зібрати окремі компоненти:
```bash
docker compose run build_manager
docker compose run build_camerapro
docker compose run build_datapro1
docker compose run build_datapro2
docker compose run build_calibration
```

Відкрити shell у build-контейнері:
```bash
docker compose run console
```

## 9. Ручна збірка залежностей (legacy довідка)

Нижче збережено скорочений legacy-процес для випадків, коли потрібна ручна підготовка середовища без Docker.

### 9.1. log4cxx

Системні пакети:
```bash
sudo apt-get install build-essential libapr1-dev libaprutil1-dev gzip zip
```

Збірка (приклад):
```bash
tar zxvf apache-log4cxx-1.3.1.tar.gz
cd apache-log4cxx-1.3.1
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/home/u/lib/log4cxx-1.3.1 \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF ..
cmake --build . -j
make install
```

### 9.2. OpenCV

Системні пакети:
```bash
sudo apt install libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
```

Збірка (приклад):
```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_opencv_world=ON \
      -DBUILD_opencv_xfeatures2d=ON \
      -DOPENCV_ENABLE_NONFREE=ON \
      -DWITH_GTK=ON \
      -DWITH_GTK_2_X=ON \
      -DBUILD_SHARED_LIBS=OFF \
      -DCMAKE_INSTALL_PREFIX=/home/u/lib/opencv-4.9.0 ../opencv-4.9.0
cmake --build . -j12
make install
```

### 9.3. Clipper2

```bash
cmake -DCMAKE_INSTALL_PREFIX=/home/u/lib/clipper2 ..
cmake --build .
make install
```

Підключення до головного проєкту:
- `-DClipper2_DIR=/home/u/lib/clipper2/lib/cmake/clipper2`

### 9.4. fmt

```bash
wget -nc -O fmt.zip https://github.com/fmtlib/fmt/releases/download/11.0.2/fmt-11.0.2.zip
unzip -n fmt.zip
mkdir -p build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE ..
make -j"$(nproc)" install
```

### 9.5. Jansson

```bash
sudo apt-get install libjansson-dev
```

## 10. Доступ до ресурсів з Windows

Базовий варіант без додаткового клієнтського ПЗ:
- Samba share на Linux;
- відкриття через Провідник Windows: `\\host\share`.

Наприклад:
- `\\192.168.X.X\AmanitaResources`

## 11. Робочий цикл після внесення змін

Рекомендований цикл:
1. Внести кодові зміни.
2. Зібрати `Debug` і/або `Release` через `builder/build_dp1_dp2.sh`.
3. Запустити DP2, потім DP1 на цільовому датасеті.
4. Перевірити артефакти в `runs/dp1/<run_name>` і `runs/dp2/<run_name>`.
5. Передати короткий handoff: що змінено, яким профілем зібрано, які команди запуску, де outputs.

---

## Currency Dashboard test assignment

This repository also contains a standalone fullstack MVP for the banking currency dashboard test assignment in `currency-dashboard/`. The backend is now implemented as a PHP Laravel application and serves the Vue frontend plus JSON APIs.

### What is implemented

- PHP Laravel backend in `currency-dashboard/backend/`.
- Vue 3 frontend with Vue Router, Pinia and Axios loaded from CDN.
- Five seeded banks with metadata, supported currencies (`usd`, `eur`, `gbp`, `chf`, `pln`) and seeded branches.
- Best-effort integrations with:
  - NBU exchange rates: `https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?json`
  - MinFin bank rates: `https://minfin.com.ua/api/currency/rates/banks/{currency_code}`
  - Finance.ua branches: `https://finance.ua/api/organization/v1/branches?slug={bank_slug}&locale=uk`
- Laravel scheduler command for regular rates and branches refreshes.
- JSON-file fallback storage in `storage/app/currency-dashboard/state.json` when external APIs are unavailable.
- APIs for banks, bank details, currencies, rates, NBU/average summary, nearest branches, statistics, significant changes and MVP profile/subscriptions.

### Run locally

From the repository root:

```bash
cd currency-dashboard/backend
cp .env.example .env
composer install
php artisan key:generate
php artisan serve --host=127.0.0.1 --port=8000
```

Open:

```text
http://127.0.0.1:8000
```

The Laravel app serves the Vue files from `currency-dashboard/frontend/`, so a separate frontend build step is not required for this MVP.

### Refresh external data

Run one refresh manually:

```bash
cd currency-dashboard/backend
php artisan currency-dashboard:refresh --all
```

Run Laravel's scheduler worker during local development:

```bash
cd currency-dashboard/backend
php artisan schedule:work
```

### API quick checks

```bash
curl http://127.0.0.1:8000/api/health
curl http://127.0.0.1:8000/api/banks
curl 'http://127.0.0.1:8000/api/rates?currencies=usd,eur'
curl 'http://127.0.0.1:8000/api/rates/summary?currencies=usd,eur'
curl 'http://127.0.0.1:8000/api/branches/nearest?lat=50.4501&lng=30.5234&limit=5'
curl 'http://127.0.0.1:8000/api/statistics?currency=usd'
```

### Notes and limitations

- The MVP stores dashboard state, users, subscriptions, rates history and significant-change events in a JSON file rather than a production database.
- Real email delivery is not wired in; the MVP exposes notification preferences and significant-change events that an SMTP/queue worker could consume.
- External banking APIs are not required for startup. If they fail, `/api/health` reports the most recent integration errors and the UI continues to work with fallback data.
- AI-agent usage is documented in `LLM_INSTRUCTIONS.md`.
