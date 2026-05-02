DP1 cards index

How to use: open card Markdown files in the 'cards' folder.

01 Data structures
------------------

- TDataCalibrationCamera - калібрування камери
  id: dp1.types.TDataCalibrationCamera
  link: cards/dp1.types.TDataCalibrationCamera.md
  src: datapro1/src/datarpoTypes.h:24-29
  note: Калібрувальні параметри камери для перетворень “пікселі ↔ промені/простір” та для задач, де потрібні внутрішні параметри (intrinsics).

- TDataCalibrationFrame - калібрування/поза на кадрі
  id: dp1.types.TDataCalibrationFrame
  link: cards/dp1.types.TDataCalibrationFrame.md
  src: datapro1/src/datarpoTypes.h:31-36
  note: Калібрувальні/позові дані, обчислені **для конкретного кадра** (на відміну від параметрів камери, які сталі).

- TDataCam - метадані камери
  id: dp1.types.TDataCam
  link: cards/dp1.types.TDataCam.md
  src: datapro1/src/datarpoTypes.h:38-41
  note: Ідентифікація камери та (опційно) її положення у просторі в координатах системи/установки.

- TDataFrame - метадані кадра
  id: dp1.types.TDataFrame
  link: cards/dp1.types.TDataFrame.md
  src: datapro1/src/datarpoTypes.h:7-22
  note: C-структура метаданих одного кадра, яку DP1/DP2 використовує як “паспорт кадра”: індекси, час експозиції, геометрія кадра та (за наявності) дані турелі.

- TDataRes - результат DP1 на кадрі
  id: dp1.types.TDataRes
  link: cards/dp1.types.TDataRes.md
  src: datapro1/src/datarpoTypes.h:66-72
  note: Контейнер, який пакує всі виходи DP1 для одного кадра: метадані камери, метадані кадра, виміри об’єктів та (опційно) дані калібрування на кадрі.

- TDataproConfig - параметри тайлінгу та обробки
  id: dp1.types.TDataproConfig
  link: cards/dp1.types.TDataproConfig.md
  src: datapro1/src/datarpoTypes.h:79-89
  note: Параметри, які визначають як кадр ділиться на тайли та як обробка працює на межах тайлів.

- TDataproVar - робочі буфери DP1
  id: dp1.types.TDataproVar
  link: cards/dp1.types.TDataproVar.md
  src: datapro1/src/datarpoTypes.h:91-95
  note: Набір робочих буферів, що зберігають проміжні дані по тайлах і стан фонових віднімачів.

- TDrawMeasurement - геометрія для відрисовки
  id: dp1.types.TDrawMeasurement
  link: cards/dp1.types.TDrawMeasurement.md
  src: datapro1/src/datarpoTypes.h:61-64
  note: Структура для візуалізації одного об’єкта: axis-aligned bbox + rotated bbox.

- TFolder - імена підпапок для дебагу/експорту
  id: dp1.types.TFolder
  link: cards/dp1.types.TFolder.md
  src: datapro1/src/datarpoTypes.h:97-105
  note: Набір “канонічних” назв підпапок для збереження проміжних результатів/логів.

- TOptionsMeasurement - виміри одного об’єкта
  id: dp1.types.TOptionsMeasurement
  link: cards/dp1.types.TOptionsMeasurement.md
  src: datapro1/src/datarpoTypes.h:43-59
  note: Набір числових характеристик сегментованого об’єкта (контур/маска), який є виходом DP1 для подальшої обробки (DP2, трекінг, аналітика).



02 Frame processing interface
-----------------------------

- frame_n_header - вхід кадра у frame_processor
  id: dp1.frame.frame_n_header
  link: cards/dp1.frame.frame_n_header.md
  src: datapro1/src/dp1_frame_proc.h:14-19
  note: Легковаговий контейнер-посилання, який передається у `frame_processor::proc_next_frame` і містить: - вказівник на `cv::Mat` кадра, - (опційно) вказівник на `cam_pro::FrameHeader`, - часову мітку IPC (`ipc_start_time`).

- frame_processor - інтерфейс обробника кадрів
  id: dp1.frame.frame_processor
  link: cards/dp1.frame.frame_processor.md
  src: datapro1/src/dp1_frame_proc.h:21-29
  note: Абстрактний інтерфейс (polymorphic), який реалізує один метод: `proc_next_frame(frame_n_header rc_frame)`.



03 Runtime buffers and threading
--------------------------------

- dp1_th_proc_par - параметри багатопотокової обробки тайлів
  id: dp1.runtime.dp1_th_proc_par
  link: cards/dp1.runtime.dp1_th_proc_par.md
  src: datapro1/src/datapro1.cpp:169-222 (локальна структура в .cpp)
  note: Локальна структура, яка агрегує всі посилання/ресурси, потрібні робочим потокам DP1 для паралельної обробки тайлів одного кадра.

- rc_ipc_raw_frame - елемент буфера кадрів IPC
  id: dp1.runtime.rc_ipc_raw_frame
  link: cards/dp1.runtime.rc_ipc_raw_frame.md
  src: datapro1/src/dp1_ipc_runner.cpp:12-19 (локальна структура в .cpp)
  note: Локальна (translation-unit) структура, яку `dp1_ipc_runner.cpp` використовує як елемент черги `deque<rc_ipc_raw_frame>`.

- rc_uri_raw_frame - елемент буфера кадрів URI
  id: dp1.runtime.rc_uri_raw_frame
  link: cards/dp1.runtime.rc_uri_raw_frame.md
  src: datapro1/src/dp1_uri_runner.cpp:10-15 (локальна структура в .cpp)
  note: Локальна структура буфера для кадрів, які читаються через `cv::VideoCapture` (файл/камера/стрім).



04 Input sources and IPC
------------------------

- ipc_data_rc - інтерфейс отримання кадрів через IPC
  id: dp1.ipc.ipc_data_rc
  link: cards/dp1.ipc.ipc_data_rc.md
  src: datapro1/src/dp1_ipc.h:7-20
  note: Абстрактний receiver, який запускає окремий потік приймання даних і викликає callback при надходженні кадра.



05 Tiling and scheduling
------------------------

- i_calc_tile_limit - інтерфейс ліміту обробки тайлів
  id: dp1.tiles.i_calc_tile_limit
  link: cards/dp1.tiles.i_calc_tile_limit.md
  src: datapro1/src/dp1_calc_limit.h:12-31
  note: Інтерфейс, який вирішує: чи потрібно обробляти конкретний тайл у наступному кадрі, і дозволяє оновлювати статистику часу обробки.

- snail_path - порядок обходу тайлів (спіраль/равлик)
  id: dp1.tiles.snail_path
  link: cards/dp1.tiles.snail_path.md
  src: datapro1/src/dp1_snail_path.h:12-55
  note: Клас, який генерує послідовність (row,col) для обходу прямокутної сітки тайлів за “спіральною/равликовою” траєкторією.



06 Configuration
----------------

- calc_tile_lim_cfg_t - конфіг обмеження тайлів за часом
  id: dp1.config.calc_tile_lim_cfg_t
  link: cards/dp1.config.calc_tile_lim_cfg_t.md
  src: datapro1/src/dp1_config.h:12-16
  note: Конфіг для адаптивного пропуску/обмеження обробки тайлів (performance governor), щоб утримувати заданий time budget.

- prg_config - конфіг DP1 (JSON → структури)
  id: dp1.config.prg_config
  link: cards/dp1.config.prg_config.md
  src: datapro1/src/dp1_config.h:17-170
  note: Клас конфігурації програми DP1, який читає JSON (`config_datapro1.json`) і тримає типізовані секції конфігу.

- sum-binning preprocessing у DP1
  id: dp1.preproc.binning_sum
  link: cards/dp1.preproc.binning_sum.md
  src: datapro1/src/dp1_frame_proc.cpp:17-420
  note: Опційний preprocessing етап до тайлінгу, керований через `config.binning`, із масштабуванням вимірів назад у СК початкового кадра.



07 RPC serialization and primitives
-----------------------------------

- CMemStore - буфер байтів з курсором (black-box API)
  id: dp1.rpc.CMemStore
  link: cards/dp1.rpc.CMemStore.md
  src: datapro1/src/dp1_tr_res2dp2.cpp; datapro1/src/dp1_rpc_data_mrsh.cpp:dp1_tr_res2dp2.cpp:58-99; dp1_rpc_data_mrsh.cpp:8-156
  note: `CMemStore` - зовнішня залежність (header `mem_store.h` не входить у архів DP1), яка використовується як: - буфер накопичення payload, - курсор для послідовного читання/запису, - джерело `data()`/`size()` для фреймінгу повідомлення.

- rpc_data_former - фреймінг TCP повідомлень (black-box API)
  id: dp1.rpc.rpc_data_former
  link: cards/dp1.rpc.rpc_data_former.md
  src: datapro1/src/dp1_tr_res2dp2.cpp:17-70
  note: `rpc_data_former` - зовнішня залежність (header `m_rpc_d_former.h` не входить у архів DP1), яка формує “raw TCP message bytes” з payload.

- serialize_Mat / deserialize_Mat - двійковий формат cv::Mat для RPC
  id: dp1.rpc.serialize_Mat
  link: cards/dp1.rpc.serialize_Mat.md
  src: datapro1/src/dp1_rpc_data_mrsh.cpp:8-37
  note: Пара функцій, що визначає **wire-format** для `cv::Mat` при передачі DP1 -> DP2 через `CMemStore`: 1) заголовок (cols, rows, elemSize, type), 2) байти пікселів (рядками або одним блоком).

- serialize_camera_calibration_data - payload калібрування камери (K, distCoeffs)
  id: dp1.rpc.serialize_camera_calibration_data
  link: cards/dp1.rpc.serialize_camera_calibration_data.md
  src: datapro1/src/dp1_rpc_data_mrsh.cpp:39-57
  note: Серiалiзацiя/десерiалiзацiя `TDataCalibrationCamera` для передачі DP1 -> DP2: матриця камери, дисторсія та параметри chessboard.

- serialize_dp1_res / deserialize_dp1_res - payload результатів кадра DP1
  id: dp1.rpc.serialize_dp1_res
  link: cards/dp1.rpc.serialize_dp1_res.md
  src: datapro1/src/dp1_rpc_data_mrsh.cpp:89-156
  note: Wire-format для `TDataRes` (результат одного кадра), який DP1 відправляє в DP2.

- serialize_frame_calibration_data - payload калібрування кадра (R,t,rvec, features)
  id: dp1.rpc.serialize_frame_calibration_data
  link: cards/dp1.rpc.serialize_frame_calibration_data.md
  src: datapro1/src/dp1_rpc_data_mrsh.cpp:59-87
  note: Серiалiзацiя/десерiалiзацiя `TDataCalibrationFrame` (кадрова калібровка/поза) для DP1 -> DP2.



08 DP1 -> DP2 network protocol
------------------------------

- dp1_to_dp2_* - типи повідомлень DP1 -> DP2 (msg_type)
  id: dp1.net.dp1_to_dp2_message_types
  link: cards/dp1.net.dp1_to_dp2_message_types.md
  src: datapro1/src/dp1_tr_res2dp2.cpp:72-101
  note: `msg_type` - перше поле payload у `CMemStore` перед даними. DP1 використовує щонайменше два типи:

- send_res_to_dp2 / init_connect_to_dp2 - канал передачі DP1 -> DP2 (TCP)
  id: dp1.net.dp1_tr_res2dp2_connection
  link: cards/dp1.net.dp1_tr_res2dp2_connection.md
  src: datapro1/src/dp1_tr_res2dp2.cpp:31-112
  note: Модуль, що відповідає за: - з’єднання з DP2 по TCP (`boost::asio`), - реконект з інтервалом, - відправку двох типів повідомлень: calibration та measurements.



09 File outputs (.blob/.json)
-----------------------------

- Іменування вихідних файлів DP1 (.blob/.json)
  id: dp1.io.dp1_output_filenames
  link: cards/dp1.io.dp1_output_filenames.md
  src: datapro1/src/dataproSaveToFile.cpp:8-29, 98-115
  note: Правила побудови імен файлів результатів DP1, які кодують `cam_index`, час (UTC) і/або `index_frame`.

- save_res (.blob) - файловий формат збереження результатів DP1
  id: dp1.io.save_res_blob
  link: cards/dp1.io.save_res_blob.md
  src: datapro1/src/datapro1.cpp; datapro1/src/dataproSaveToFile.cpp:datapro1.cpp:371-505; dataproSaveToFile.cpp:31-96
  note: `.blob` - бінарний файл результатів DP1. Існують два режими: - **rolling** (append у файл з інтервалом по часу), - **single-frame** (один файл на кадр з заданим `file_name`).

- seva_res_json (.json) - схема JSON-виводу результатів DP1
  id: dp1.io.save_res_json
  link: cards/dp1.io.save_res_json.md
  src: datapro1/src/datapro1.cpp:400-505
  note: JSON “вітрина” результатів DP1 (читабельний артефакт), який записується поруч із `.blob`, якщо `txt_file == true`.



10 Runtime lifecycle and orchestration
-------------------------------------

- ipc_data_rc_impl - IPC receiver implementation
  id: dp1.ipc.ipc_data_rc_impl
  link: cards/dp1.ipc.ipc_data_rc_impl.md
  src: datapro1/src/dp1_ipc.cpp:20-252
  note: Реалізація IPC receive path з POSIX queue/shared memory і фоновим потоком читання.

- run_ipc_src - orchestration IPC runner
  id: dp1.runtime.run_ipc_src
  link: cards/dp1.runtime.run_ipc_src.md
  src: datapro1/src/dp1_ipc_runner.cpp:48-89
  note: Main loop обробки IPC-кадрів: bounded queue, lazy init processor, handoff у proc_next_frame.

- run_uri_src - orchestration URI runner
  id: dp1.runtime.run_uri_src
  link: cards/dp1.runtime.run_uri_src.md
  src: datapro1/src/dp1_uri_runner.cpp:25-71
  note: Main loop URI ingest: читання кадрів, конвертація формату і передача в frame processor.

- dp1_main_orchestration - bootstrap і source routing
  id: dp1.runtime.dp1_main_orchestration
  link: cards/dp1.runtime.dp1_main_orchestration.md
  src: datapro1/src/dp1_main.cpp:59-120
  note: Top-level orchestration запуску DP1: config/log/signal/network init і вибір runner path.

11 Stage interfaces
-------------------

- ICandidateExtractionStage - canonical stage interface для candidate extraction boundary
  id: dp1.stage.ICandidateExtractionStage
  link: cards/dp1.stage.icandidateextractionstage.md
  src: project-knowledge/06-tasks/cards/AMNT-0009.md
  note: Формалізує логічний input/output кандидатів після frame preprocessing і compatibility очікування для downstream segmentation/filter stages.

- ISegmentationStage - canonical stage interface для segmentation boundary
  id: dp1.stage.ISegmentationStage
  link: cards/dp1.stage.isegmentationstage.md
  src: project-knowledge/06-tasks/cards/AMNT-0009.md
  note: Формалізує перетворення candidate artifacts у segmentation-resolved objects для downstream filtering/measurement.

- IObjectFilterStage - canonical stage interface для object filtering boundary
  id: dp1.stage.IObjectFilterStage
  link: cards/dp1.stage.iobjectfilterstage.md
  src: project-knowledge/06-tasks/cards/AMNT-0009.md
  note: Формалізує policy-driven acceptance/rejection перед `IMeasurementStage` із збереженням сумісності з `TDataRes` boundary.
- IMeasurementStage - canonical stage interface для measurement boundary
  id: dp1.stage.IMeasurementStage
  link: cards/dp1.stage.imeasurementstage.md
  src: project-knowledge/06-tasks/cards/AMNT-0007.md; project-knowledge/06-tasks/cards/AMNT-0008.md
  note: Формалізує логічний input/output, compatibility constraints з `TDataRes`/`serialize_dp1_res` і boundary очікування для DP1 -> DP2.

