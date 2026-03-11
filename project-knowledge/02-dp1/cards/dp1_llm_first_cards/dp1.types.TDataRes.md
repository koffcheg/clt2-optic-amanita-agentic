---
id: dp1.types.TDataRes
title:
  uk: "TDataRes - результат DP1 на кадрі"
  en: "TDataRes - DP1 per-frame result"
tags: [dp1, struct, result, serialization]
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "66-72"
status: "draft"
---

## Definition
Контейнер, який пакує всі виходи DP1 для одного кадра: метадані камери, метадані кадра, виміри об’єктів та (опційно) дані калібрування на кадрі.

**Поля:**
- `data_cam` (TDataCam)
- `data_frame` (TDataFrame)
- `meas` (vector<TOptionsMeasurement>)
- `calib_frame` (TDataCalibrationFrame)

## Assumptions
- Всі вкладені структури відповідають **одному** кадру (`data_frame.index_frame`) і **одній** камері (`data_cam.cam_index`).
- Порядок `meas` не є семантично важливим, якщо явно не зафіксовано інше.

## Theorem / Contract
- `meas.size() == 0` допустимо (кадр без об’єктів).
- Якщо `calib_frame` не валідний, downstream повинен мати спосіб це визначити (домовленість/прапорець/розмір Mat).

## Interpretation
Це “повідомлення” DP1, яке можна:
- зберігати на диск,
- передавати в DP2,
- використовувати для тестів/трасування.

## Failure cases
- Мікс різних кадрів/камер в одному контейнері через помилки потоків.
- Серіалізація без версії схеми (schema_version) → проблеми сумісності.

## Typical misuse
- Використовувати `calib_frame` як завжди валідний.
- Покладатися на стабільність `id_obj` між кадрами (це не трек).

## Connections
- serialized_by: dp1.methods.serialize_dp1_res (dp1_rpc_data_mrsh.h)
- sent_to: dp1.dp2.send_res_to_dp2 (dp1_tr_res2dp2.h)
- contains: dp1.types.TDataFrame, dp1.types.TOptionsMeasurement, dp1.types.TDataCalibrationFrame
