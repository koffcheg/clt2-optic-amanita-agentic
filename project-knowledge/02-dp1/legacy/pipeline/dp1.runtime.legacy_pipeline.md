---
id: dp1.runtime.legacy_pipeline
title:
  uk: "Legacy DP1 pipeline - повний runtime-конвеєр обробки кадра"
  en: "Legacy DP1 pipeline - full per-frame runtime pipeline"
tags: [dp1, runtime, pipeline, legacy, orchestration]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_main.cpp; datapro1/src/dp1_ipc_runner.cpp; datapro1/src/dp1_uri_runner.cpp; datapro1/src/dp1_frame_proc.cpp; datapro1/src/datapro1.cpp"
  lines: "dp1_main.cpp:35-123; dp1_ipc_runner.cpp:31-84; dp1_uri_runner.cpp:25-74; dp1_frame_proc.cpp:114-411; datapro1.cpp:31-340, 371-505"
status: "draft"
---

## Definition
`dp1.runtime.legacy_pipeline` - це повний шлях виконання legacy `datapro1` від старту процесу до формування результату кадра, передачі його в DP2 та опційного запису артефактів на диск.

Конвеєр складається з таких етапів:
1. Запуск процесу та читання конфігу.
2. Вибір циклу запуску за типом джерела.
3. Приймання кадра з IPC або URI/VideoCapture.
4. Вхід у `frame_processor::proc_next_frame(...)`.
5. Заповнення метаданих кадра.
6. Опційна гілка калібрування.
7. Попередня обробка перед основним DP1.
8. Тайлова обробка в `datapro1(...)`.
9. Збирання `TDataRes`.
10. Передача в DP2.
11. Опційний файловий вивід та вивід на екран/у відео.

## Assumptions
- Legacy-конвеєр виконується навколо одного верхньорівневого процесу `datapro1`, який запускається як `datapro1 <cam_index> [config_datapro1.json] [dp1_log.xml]`.
- Вибір шляху приймання кадра робиться тільки через `cfg.get_frame_src_type()` і значення `config.source.source`.
- Для IPC-шляху у `proc_next_frame(...)` передається `cam_pro::FrameHeader`; для URI-шляху header відсутній і частина метаданих кадра синтезується локально.
- Основний обчислювальний конвеєр живе всередині `fr_proc_impl::proc_next_frame(...)` та `datapro1(...)`, а не в циклах запуску джерел.
- Тайлова обробка може бути багатопотоковою, але кадри подаються в `frame_processor` послідовно з циклу запуску джерела.

## Theorem / Contract
- Загальний контракт конвеєра: `main(...)` спочатку ініціалізує logging, `prg_config`, калібрування камери, обробку сигналів та з'єднання DP1 -> DP2, і лише потім запускає `run_ipc_src(...)` або `run_uri_src(...)`.
- Загальний контракт шляху приймання кадра: IPC-цикл передає в `frame_processor` кадр разом із `FrameHeader` та `ipc_start_time`, а URI-цикл передає кадр без `FrameHeader` після нормалізації `BGR -> GRAY -> CV_16UC1`.
- Загальний контракт покадрової обробки: `fr_proc_impl::proc_next_frame(...)` збирає метадані кадра, виконує optional calibration/preprocessing, запускає `datapro1(...)`, формує `TDataRes`, викликає `send_res_to_dp2(...)` і за конфігом пише артефакти.
- Загальний контракт тайлової обробки: `datapro1(...)` планує тайли, обробляє їх, збирає `out_meas` та `data_draw` і повертає покадрові результати назад у `proc_next_frame(...)`.
- Загальний контракт виводу артефактів: покадровий запис на диск пишеться в `test.out_folder + "/" + folder_name_dp1.data_bin` з іменем `C{cam:03}_F{frame:06}` та розширеннями `.blob` / `.json`.

## Interpretation
Ця сутність є канонічним високорівневим описом того, як саме legacy DP1 проходить шлях:
`process bootstrap -> source ingest -> frame processing -> tile segmentation -> result packaging -> network/file export`.

Важливо, що конвеєр у legacy DP1 не є "чисто алгоритмічним":
- частина поведінки під час виконання знаходиться в `main(...)` і циклах запуску джерел;
- частина покадрової координації та побічних дій - у `fr_proc_impl::proc_next_frame(...)`;
- ядро конвеєра сегментації та вимірювань - у `datapro1(...)`;
- файловий і мережевий вивід виконується вже після завершення основного шляху комп'ютерного зору.

## Етап 1. Запуск процесу та читання конфігу
Legacy DP1 стартує в `main(...)` у `datapro1/src/dp1_main.cpp`.

На цьому етапі процес:
- парсить CLI-аргументи;
- визначає `cam_index`, шлях до program config і шлях до logging config;
- ініціалізує logging через `log4cxx`;
- читає `prg_config`;
- читає дані калібрування камери;
- ініціалізує обробку сигналів;
- ініціалізує канал DP1 -> DP2.

Це верхньорівневий запуск модуля. Якщо тут виникає помилка конфігу, logging або калібрування, конвеєр не переходить до шляху приймання кадрів.

## Етап 2. Вибір циклу запуску за типом джерела
Після запуску `main(...)` викликає `cfg.get_frame_src_type()` і вибирає один із двох шляхів запуску:
- `run_ipc_src(...)` для `campro` / IPC source;
- `run_uri_src(...)` для URI-based source (`webcam`, `ipcam`, `videofile`, `imagefile`).

На цьому етапі конвеєр ще не обробляє кадри, а лише маршрутизує потік виконання у відповідний шлях приймання.

## Етап 3. Приймання кадра з IPC або URI/VideoCapture
На цьому етапі кадр потрапляє у DP1 під час виконання.

Для IPC-шляху:
- `run_ipc_src(...)` запускає receiver;
- callback `on_rc_next_frame(...)` приймає shared-memory/IPC frame;
- кадр, його `FrameHeader` та `ipc_start_time` кладуться у bounded deque.

Для URI-шляху:
- `run_uri_src(...)` читає кадр через `cv::VideoCapture`;
- кадр приводиться до формату `BGR -> GRAY -> CV_16UC1`;
- нормалізований кадр кладеться у bounded deque.

Цей етап завершується тим, що цикл запуску джерела передає кадр у `frame_processor`.

## Етап 4. Вхід у `frame_processor::proc_next_frame(...)`
Після першого валідного кадра цикл запуску джерела ліниво створює `frame_processor` через `get_fr_processor(...)`.

У конструкторі `fr_proc_impl` викликається `init_params()`, де:
- ініціалізується вивід на екран і у відео;
- готується runtime-стан для binocular-гілки;
- перевіряється runtime-конфіг для binning;
- обчислюються параметри тайлів;
- створюється обмежувач тайлової обробки;
- готуються допоміжні runtime-буфери.

Після цього кожен кадр передається у `proc_next_frame(...)` як межа покадрової координації.

## Етап 5. Заповнення метаданих кадра
На початку `proc_next_frame(...)` legacy DP1 заповнює `TDataCam` і `TDataFrame`.

Для IPC-шляху метадані беруться з `cam_pro::FrameHeader`:
- `index_frame`;
- `width`, `height`;
- `Az`, `El`, `V_az`, `V_el`;
- `turretInfoValid`;
- `exposureStart`, `exposureLength`;
- `pixelWidth`, `pixelHeight`, `focalLength`.

Для URI-шляху частина цих полів синтезується локально:
- `index_frame` береться з внутрішнього лічильника;
- геометрія кадра береться з `cv::Mat`;
- поля турелі, експозиції та оптики підставляються як runtime-сурогати.

Саме тут формується контракт метаданих кадра для подальшого `TDataRes`.

## Етап 6. Опційна гілка калібрування
Якщо `cfg.binocular.switched == true`, конвеєр виконує покадрову гілку калібрування.

Можливі два режими:
- використати вже задані `R_matrix` і `t_vector`;
- якщо `using_template == true`, спробувати оцінити позу на поточному кадрі, а при невдачі повернутися до заданих `R/t`.

Результат цього етапу - заповнення `frame_param_`, який далі потрапляє у `TDataRes.calib_frame` і, за потреби, у файлові артефакти.

## Етап 7. Попередня обробка перед основним DP1
Перед викликом `datapro1(...)` legacy-шлях виконує попередню обробку на рівні цілого кадра.

Поточна підтверджена попередня обробка:
- optional sum-binning;
- переведення кадра до одноканального подання для binning path (за потреби);
- подальше масштабування measurements і draw data назад у систему координат оригінального кадра після завершення обробки.

Цей етап ще не виконує сегментацію, але готує `frame_to_process` для тайлового конвеєра.

## Етап 8. Тайлова обробка в `datapro1(...)`
Основний алгоритмічний конвеєр живе у `datapro1(...)`.

Всередині цього етапу відбувається:
1. Планування тайлів через `calc_tile_limit`.
2. Підготовка багатопотокового розподілу роботи для обробників тайлів.
3. Розбиття кадра на тайли через `splitImage(...)`.
4. Жорстка конвертація tile data у `CV_8UC1`.
5. Optional median filter на тайлі.
6. Optional covariance filter на тайлі.
7. Застосування background subtractor.
8. Уточнення сегментації, порогова обробка і виділення контурів.
9. Обчислення геометрії та даних для відмалювання контурів.
10. Обчислення `TOptionsMeasurement` для кожного об'єкта.
11. Збирання всіх вимірювань з тайлів у покадровий `out_meas`.
12. Збирання всіх результатів для відмалювання з тайлів у `var.data_draw`.

Саме цей етап є ядром legacy-конвеєра сегментації та вимірювань.

## Етап 9. Збирання `TDataRes`
Після повернення з `datapro1(...)` покадрова координація завершує складання результату кадра.

На цьому етапі:
- `data_res.meas = out_meas_`;
- `data_res.calib_frame = frame_param_`;
- у `data_res.data_frame.dp1_spent_time` записується витрачений час;
- обмежувач тайлової обробки отримує час обробки останнього кадра.

Результатом є готовий покадровий контейнер `TDataRes`.

## Етап 10. Передача в DP2
Після формування `TDataRes` legacy DP1 викликає `send_res_to_dp2(data_res)`.

Цей етап:
- не змінює структуру `TDataRes`;
- використовує вже ініціалізовану межу мережевої передачі;
- виконується синхронно з покадровим шляхом.

Тобто передача у DP2 є частиною штатного покадрового конвеєра, а не окремою післяобробкою.

## Етап 11. Опційний файловий вивід та вивід на екран/у відео
Після передавання в мережевий канал конвеєр може створювати локальні артефакти.

Файловий вивід:
- якщо `cfg.test.res_file.switched == true`, пишеться `.blob`;
- якщо `txt_file == true`, додатково пишеться `.json`;
- у покадровому режимі файли пишуться в `test.out_folder/data_bin` з базовим ім'ям `C{cam:03}_F{frame:06}`.

Вивід на екран / у відео:
- якщо `cfg.test.display.display == true`, формується накладення на кадр з `data_draw`;
- якщо `cfg.test.display.video_save == true`, кадр пишеться у записувач відео;
- якщо `median_bg.switched == true`, додатково працює окрема debug/visualization гілка median background.

Це завершальний етап legacy-конвеєра, після якого кадр вважається повністю обробленим.

## Failure cases
- Невалідний config або logging cfg дає ранній stop ще до запуску циклу приймання кадрів.
- Невідомий `source.source` завершує процес без старту шляху приймання.
- URI-шлях ламається при неможливості відкрити source або при порожньому кадрі.
- IPC-шлях залежить від коректного часу життя спільного буфера кадру; помилки передавання можуть дати некоректний `cv::Mat`.
- У `datapro1(...)` є жорстка рання конвертація tile data з 16-bit у 8-bit, тому обчислювальний шлях не зберігає повний mono16 dynamic range.
- `datapro1(...)` чекає завершення робочих потоків через polling loop з `sleep_for(1ms)`, що створює додаткову затримку і busy-wait поведінку.
- JSON output очікує валідні `R_matrix` і `t_vec`; порожні або неініціалізовані calibration matrices можуть зламати запис.
- Передавання в DP2 виконується синхронно з покадровим шляхом, тому проблеми з транспортом можуть впливати на затримку кадра.

## Typical misuse
- Спрощувати legacy DP1 до одного виклику `datapro1(...)` і ігнорувати етапи запуску, цикли приймання кадрів та вивід.
- Вважати URI-шлях та IPC-шлях однаковими за контрактом метаданих: у URI-шляху частина `TDataFrame` синтетична, а не прийшла з upstream header.
- Вважати `median_bg` частиною основного шляху сегментації: у поточному коді це окрема покадрова debug/visualization гілка після `datapro1(...)`.
- Вважати файловий `.blob` тим самим форматом, що й payload DP1 -> DP2: це різні wire/file contracts.
- Припускати, що `dp1_spent_time` вимірює тільки алгоритм `datapro1(...)`; у legacy-шляху це покадровий час від `ipc_start_time` до кінця `proc_next_frame(...)`.

## Open questions
- Чи існує downstream-споживач, який покладається саме на rolling `.blob` режим, а не тільки на покадрові файли.
- Чи потрібна окрема канонічна картка для debug/display artifact path як самостійного підконвеєра під час виконання.

## Connections
- uses: dp1.runtime.dp1_main_orchestration
- uses: dp1.runtime.run_ipc_src
- uses: dp1.runtime.run_uri_src
- uses: dp1.frame.frame_processor
- uses: dp1.preproc.binning_sum
- uses: dp1.net.dp1_tr_res2dp2_connection
- uses: dp1.rpc.serialize_dp1_res
- uses: dp1.io.save_res_blob
- uses: dp1.io.save_res_json
- contains: dp1.types.TDataRes
- overlaps_with: dp1.runtime.dp1_main_orchestration
