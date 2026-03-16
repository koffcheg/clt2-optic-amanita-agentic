# DP2 cards index

How to use: open card Markdown files in the `cards` folder.

01 Core data structures
-----------------------

- Measurement - базовий вимір об'єкта в DP2
  id: dp2.types.Measurement
  link: cards/dp2.types.Measurement.md
  src: datapro2/src/datapro2Types.h:8-16
  note: Елемент вхідних/проміжних даних трекінгу з 2D/3D координатами, часовою міткою та кутами.

- PTPoint - кандидат точки для побудови/продовження траєкторій
  id: dp2.types.PTPoint
  link: cards/dp2.types.PTPoint.md
  src: datapro2/src/datapro2Types.h:18-21
  note: Обгортка над Measurement із лічильником кадрів для етапу асоціації.

- TStrobe - геометричне вікно (strobe) траєкторії
  id: dp2.types.TStrobe
  link: cards/dp2.types.TStrobe.md
  src: datapro2/src/datapro2Types.h:23-28
  note: Параметри області пошуку/відсікання для конкретної траєкторії.

- Trajectory - агрегований стан треку
  id: dp2.types.Trajectory
  link: cards/dp2.types.Trajectory.md
  src: datapro2/src/datapro2Types.h:30-52
  note: Контейнер історії вимірів, оцінених параметрів руху, статистики й strobe-даних.

02 Configuration structures
---------------------------

- binocular_cfg - конфіг бінокулярного режиму
  id: dp2.config.binocular_cfg
  link: cards/dp2.config.binocular_cfg.md
  src: datapro2/src/dp2_srobe_mth_chg.h:6-15
  note: Параметри узгодження двох камер і тестового генератора точки.

- dp2strobe_mth_cfg - конфіг параметрів трекінгу/strobe-методу
  id: dp2.config.dp2strobe_mth_cfg
  link: cards/dp2.config.dp2strobe_mth_cfg.md
  src: datapro2/src/dp2_srobe_mth_chg.h:17-52
  note: Границі швидкості/прискорення/STD, правила drop/show треків і політика збереження результатів.

- dp2::dp2_cfg::turret_exch_cfg - конфіг обміну з turret
  id: dp2.config.dp2_cfg.turret_exch_cfg
  link: cards/dp2.config.dp2_cfg.turret_exch_cfg.md
  src: datapro2/src/dp2_cfg.h:9-16
  note: Мережеві параметри та інтервали reconnect/dispatch для каналу DP2 -> turret.

- dp2::dp2_cfg - кореневий конфіг DP2
  id: dp2.config.dp2_cfg
  link: cards/dp2.config.dp2_cfg.md
  src: datapro2/src/dp2_cfg.h:8-26
  note: Агрегація server/turret/strobe/binocular секцій із JSON конфігу.

03 Cross-module overlaps
------------------------

DP2 напряму використовує DP1-типи через `datapro1/src/datarpoTypes.h` та RPC десеріалізацію:
- `TDataRes`, `TDataCam`, `TDataFrame`, `TDataCalibrationCamera`, `TOptionsMeasurement`
- Це зафіксовано в Connections відповідних DP2 карток і не дублює канонічні DP1 описи.

04 Runtime boundaries
--------------------

- dp2::server - асинхронний TCP acceptor
  id: dp2.runtime.server
  link: cards/dp2.runtime.server.md
  src: datapro2/src/dp2_svr.h:15-31
  note: Runtime boundary для listen/accept lifecycle і створення сесій.

- dp2::session - per-connection read lifecycle
  id: dp2.runtime.session
  link: cards/dp2.runtime.session.md
  src: datapro2/src/dp2_ses.h:8-33
  note: Read-loop TCP stream і передача raw bytes в rpc sink.

- dp2_rpc_cl - payload dispatcher
  id: dp2.rpc.dp2_rpc_cl
  link: cards/dp2.rpc.dp2_rpc_cl.md
  src: datapro2/src/dp2_rpc_cl.h:15-21
  note: Dispatch повідомлень DP1 за msg_type на доменні handler-и.

05 Receive path DP1 -> DP2
--------------------------

- receive path contract (stream -> framed msg -> payload)
  id: dp2.net.dp1_to_dp2_receive_path
  link: cards/dp2.net.dp1_to_dp2_receive_path.md
  src: datapro2/src/dp2_ses.cpp:26-63
  note: Контракт шляху прийому від TCP stream до message-level dispatch у DP2.
