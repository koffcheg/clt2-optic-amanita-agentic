# DP1_v2 Phase 0: Runtime Skeleton Parity With Controlled Refactor

## Призначення

Цей документ фіксує нове трактування `Phase 0` для `DP1_v2/datapro1_v2`.

`Phase 0` більше не розглядається як лише `bootstrap + config foundation`.
У новому обсязі це фаза, після якої `datapro1_v2` уже працює як повноцінний модуль DP1:
- запускається через CLI і конфіг;
- приймає кадри з `campro/ipc` і з `uri/file`;
- проводить кадр через повний новий runtime-конвеєр;
- формує валідний порожній `TDataRes`;
- передає порожній `TDataRes` у DP2 через живий legacy-сумісний send boundary;
- записує порожні, але сумісні файлові артефакти.

При цьому перенос із legacy має виконуватися не `1в1`, а через контрольований рефакторинг і усунення відомих структурних проблем коду.

## Ціль фази

Побудувати для `DP1_v2` повний робочий каркас виконання, у якому:
- зовнішній runtime-контракт сумісний з legacy DP1 на рівні запуску, джерел кадрів, `TDataRes`, DP1 -> DP2 handoff і файлових артефактів;
- внутрішня архітектура вже розділена на явні стадії та модулі;
- алгоритмічні стадії поки можуть бути порожніми або мінімальними заглушками;
- подальший перенос CV-логіки виконується локально по стадіях, без повторного перелому всього runtime-конвеєра.

## Критерії завершення Phase 0

`Phase 0` вважається завершеною, якщо `datapro1_v2`:
- запускається як legacy DP1 через CLI/config/log config;
- підтримує обидва основні шляхи приймання кадрів:
  - `campro/ipc`
  - `uri/file`
- проводить кадр через повний новий stage-by-stage конвеєр;
- формує валідний порожній `TDataRes`;
- може передати цей порожній `TDataRes` у DP2 через живий сумісний send boundary;
- пише порожні, але сумісні артефакти на диск у legacy layout;
- не використовує legacy runtime path як свій execution core;
- має явні точки розширення для подальшого переносу алгоритміки.

## Обов'язкові етапи, що переносяться повністю

### 1. Запуск процесу і bootstrap runtime

Має бути перенесено повністю:
- CLI contract;
- logging init;
- config loading;
- signal handling;
- camera calibration loading;
- DP1 -> DP2 connection init.

Очікуваний результат:
- `datapro1_v2` запускається як повноцінний процес DP1;
- життєвий цикл процесу не залежить від тимчасових smoke-only рішень;
- верхньорівневий runtime path уже є production-shaped каркасом.

### 2. Вибір шляху запуску за типом джерела

Має бути перенесено повністю:
- визначення типу source з конфігу;
- маршрутизація execution flow у `ipc` або `uri/file` path;
- top-level orchestration, сумісна за поведінкою з legacy DP1.

Очікуваний результат:
- новий DP1 має той самий клас runtime-сценаріїв запуску, що й legacy DP1.

### 3. Приймання кадрів

Має бути перенесено повністю:
- IPC/campro runner;
- URI/file runner;
- bounded frame queue або еквівалентний контроль буфера;
- lazy creation of processor на першому валідному кадрі;
- єдиний внутрішній контракт входу кадра.

Очікуваний результат:
- новий DP1 реально отримує кадри з обох основних джерел і проводить їх далі в pipeline.

### 4. Заповнення frame metadata

Має бути перенесено повністю:
- складання `TDataCam`;
- складання `TDataFrame`;
- підтримка upstream metadata для IPC path;
- підтримка сурогатних metadata values для URI/file path там, де це потрібно для сумісності.

Очікуваний результат:
- downstream stage-и працюють з валідним frame-level metadata contract;
- порожній `TDataRes` є сумісним не лише за типом, а й за змістом метаданих.

### 5. Збирання `TDataRes`

Має бути перенесено повністю:
- формування `data_cam`;
- формування `data_frame`;
- порожній `meas`;
- валідний `calib_frame`;
- обчислення і запис `dp1_spent_time`.

Очікуваний результат:
- новий DP1 формує коректний per-frame result container навіть без алгоритмічних detections.

### 6. DP1 -> DP2 send boundary

Має бути перенесено повністю.

Обов'язкові умови:
- send boundary живий уже в `Phase 0`;
- він може відправити порожній `TDataRes`;
- не змінюються message id, payload schema, serializer layout і transport semantics ранньої сумісної фази.

Очікуваний результат:
- `datapro1_v2` уже присутній у реальному міжмодульному контурі, а не лише пише локальні smoke outputs.

### 7. Файлові артефакти

Має бути перенесено повністю на рівні зовнішнього контракту:
- layout каталогів;
- naming rules;
- `.blob`;
- за потреби `.json`.

Очікуваний результат:
- новий DP1 залишає артефакти в тій самій структурі, що й legacy DP1;
- downstream/manual tooling не ламається на рівні базового I/O layout.

Уточнення межі:
- йдеться лише про результатні артефакти кадра;
- debug image dumps не входять у `Phase 0`;
- display/video/debug output path не входить у `Phase 0`.

## Етапи, що переносяться як каркас або заглушки

### 1. Гілка калібрування

Переноситься як окрема стадія конвеєра.

У `Phase 0` достатньо:
- читати і тримати calibration-related config/state;
- мати окремий stage boundary;
- формувати валідний `calib_frame`;
- підтримувати безпечний fallback, якщо реальна frame-level pose logic ще не перенесена.

Не обов'язково в `Phase 0`:
- повний перенос template/pose estimation logic;
- parity усіх деталей binocular processing.

### 2. Стадія попередньої обробки

Переноситься як окрема стадія конвеєра.

У `Phase 0` достатньо:
- окремої preprocess boundary;
- pass-through implementation за замовчуванням;
- конфігурованих точок розширення;
- готовності підключити `binning` та інші preprocessing features пізніше без зміни runtime skeleton.

### 3. Тайлова стадія обробки

Переноситься як окрема стадія конвеєра, але без обов'язкового переносу CV-логіки.

У `Phase 0` достатньо:
- stage boundary для tile planning / tile execution;
- порожньої або мінімальної реалізації замість реальної segmentation logic;
- повернення порожнього набору measurements;
- порожнього або нульового draw/debug result.

Тобто переноситься не алгоритм `legacy datapro1(...)`, а місце цього алгоритму в новій архітектурі.

### 4. Стадія вимірювань

Переноситься як окрема стадія конвеєра.

У `Phase 0` достатньо:
- явного input/output contract;
- порожньої реалізації, яка повертає `meas = []`;
- готовності до подальшого локального додавання реальної measurement logic.

## Debug/display код не входить у `Phase 0`

У межах `Phase 0` не переносяться і залишаються в legacy DP1:
- display path;
- video save path;
- `median_bg` debug/visualization branch;
- debug image dumps проміжних кадрів і стадій;
- будь-які UI/debug side effects у runtime hot path.

Під debug image dumps тут маються на увазі допоміжні зображення, які legacy DP1 може писати у директорії на кшталт:
- `frame_input`;
- `frame_diff`;
- `frame_coor`;
- інші debug-папки з `TFolder`, якщо вони використовуються для проміжних PNG/знімків стадій.

Отже, у `Phase 0` переносяться лише:
- сумісний запуск;
- сумісне приймання кадрів;
- сумісний runtime skeleton;
- порожній `TDataRes`;
- DP1 -> DP2 send boundary;
- сумісні результатні `.blob/.json` артефакти.

## Обов'язки по сумісності

Нова `Phase 0` повинна зберігати зовнішню сумісність у таких межах:
- запуск як окремого DP1-процесу;
- підтримка обох основних source paths;
- коректне формування `TDataRes`;
- живий DP1 -> DP2 handoff;
- сумісний layout локальних артефактів.

Сумісність на цьому етапі не означає:
- алгоритмічну parity з legacy DP1;
- повну parity сегментації;
- performance parity;
- side-by-side equivalence результатів детекції;
- перенесення debug/display behavior.

## Обов'язки по рефакторингу

Перенос із legacy в `Phase 0` виконується як `compatible refactoring transfer`, а не як буквальне копіювання runtime core.

Це означає:
- не копіювати legacy `main + runner + proc_next_frame + datapro1(...)` як новий execution core;
- розділити відповідальності по окремих stage boundaries і модулях;
- ізолювати orchestration, processing, transport, file output і debug/display side effects;
- будувати розширювану структуру, у яку потім локально додається алгоритміка.

## Пропонована модульна структура для `Phase 0`

Для `Phase 0` пропонується лінійний runtime-конвеєр із явними межами між модулями.

### 1. `startup`
Відповідає за запуск процесу.

Вхід:
- `argc/argv`

Вихід:
- `StartupContext`

### 2. `config`
Відповідає за читання і нормалізацію конфігу.

Вхід:
- `config_path`
- `cam_index`

Вихід:
- `RuntimeConfig`

### 3. `source_runner`
Відповідає за приймання кадрів із джерела.

Вхід:
- `RuntimeConfig`
- `cam_index`
- `StopToken`

Вихід:
- `RawFrameEnvelope`

### 4. `frame_normalizer`
Приводить вхідний кадр до єдиного внутрішнього контракту.

Вхід:
- `RawFrameEnvelope`

Вихід:
- `FramePacket`

### 5. `metadata_builder`
Збирає `TDataCam` і `TDataFrame`.

Вхід:
- `FramePacket`
- `RuntimeConfig`
- `cam_index`

Вихід:
- `FrameContext`

### 6. `calibration_stage`
Готує `calib_frame` і робочий кадр для наступних стадій.

Вхід:
- `FrameContext`
- `CameraCalibrationData`
- `RuntimeConfig`

Вихід:
- `PreparedFrame`

### 7. `processing_pipeline`
Містить новий каркас обробки кадра.

Вхід:
- `PreparedFrame`

Вихід:
- `ProcessingResult`

У `Phase 0` це stage-by-stage skeleton без обов'язкового переносу реальної CV-логіки:
- `preprocess`
- `tile_plan`
- `tile_process`
- `measure`

### 8. `result_sink`
Завершує формування результату і виконує зовнішні side effects.

Вхід:
- `ProcessingResult`
- `RuntimeConfig`

Вихід:
- `TDataRes`

Побічні дії:
- DP1 -> DP2 send
- запис `.blob/.json`

### Мінімальні внутрішні контракти

Для `Phase 0` достатньо мати такий мінімальний набір внутрішніх типів:
- `StartupContext`
- `RuntimeConfig`
- `RawFrameEnvelope`
- `FramePacket`
- `FrameContext`
- `PreparedFrame`
- `ProcessingResult`

Фінальний зовнішній контракт:
- `TDataRes`

## Проблеми legacy-коду, які не можна переносити `1в1`

### 1. Змішення обов'язків у frame-level path

У legacy `proc_next_frame(...)` в одному місці змішані:
- збирання metadata;
- calibration;
- запуск processing;
- формування result;
- send у DP2;
- запис артефактів;
- display/video behavior.

У `DP1_v2` ці речі мають бути розділені.

### 2. Монолітність `datapro1(...)`

Legacy `datapro1(...)` змішує:
- tile planning;
- threading orchestration;
- preprocessing fragments;
- subtractor path;
- segmentation;
- measurement aggregation.

У `DP1_v2` це має бути розкладено на окремі стадії та інтерфейси.

### 3. Жорстке зчеплення processing path і side effects

У `DP1_v2` потрібно уникнути ситуації, коли processing stage безпосередньо володіє:
- мережевим handoff;
- файловим виводом;
- display/debug sinks.

Ці частини мають бути окремими модулями або окремими службами.

### 4. Перенесення поганої hot-path структури

Навіть якщо алгоритміка ще порожня, новий каркас не повинен наслідувати як основу дизайну:
- busy-wait orchestration;
- неявні глобальні стани;
- важкі static runtime залежності без явного lifecycle;
- труднощі з підміною або локальним тестуванням окремих стадій.

### 5. Неявні контракти між стадіями

У `DP1_v2` кожна стадія має мати явний контракт:
- що приймає на вхід;
- що віддає на вихід;
- хто володіє буферами;
- які дозволені side effects;
- де проходить межа помилки і хто її обробляє.

## Що не входить у завершення Phase 0

Поза межами `Phase 0` залишаються:
- реальна segmentation parity;
- реальна measurement parity;
- повний перенос subtractor logic;
- повний перенос median/binning semantics;
- будь-який debug/display/video output path;
- debug image dumps;
- performance parity з legacy;
- side-by-side validation;
- QoS/autotune/backend policy;
- production-grade tuning алгоритмічних стадій.

## Підсумкове формулювання

У новому трактуванні `Phase 0` для `DP1_v2` - це не "мінімальний старт бінарника", а "повний робочий каркас модуля DP1 із зовнішньою сумісністю та внутрішнім рефакторингом".

Після завершення цієї фази новий DP1 повинен уже:
- жити в реальному runtime-контурі;
- приймати кадри;
- проходити повний шлях обробки;
- формувати порожній, але валідний результат;
- відправляти його в DP2;
- записувати сумісні артефакти;
- залишатися структурно готовим до поетапного переносу алгоритмів без нового архітектурного зламу.
