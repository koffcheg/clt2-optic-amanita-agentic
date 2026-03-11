---
id: dp1.rpc.serialize_dp1_res
title:
  uk: "serialize_dp1_res / deserialize_dp1_res - payload результатів кадра DP1"
  en: "serialize_dp1_res / deserialize_dp1_res - DP1 per-frame result payload"
tags: [dp1, datapro1, rpc, serialization, dp2]
source:
  file: "datapro1/src/dp1_rpc_data_mrsh.cpp"
  lines: "89-156"
status: "draft"
---

## Definition
Wire-format для `TDataRes` (результат одного кадра), який DP1 відправляє в DP2.

## Assumptions
- `TDataCam` і `TDataFrame` є POD-структурами і можуть передаватися як raw-bytes (`write_native(struct)`).
- `TOptionsMeasurement` має фіксований розмір і ABI стабільність між DP1 і DP2.
- `CMemStore` забезпечує послідовний доступ і `getCurrFreeLimit()` коректно відображає доступний залишок.

## Theorem / Contract
Формат повідомлення (у термінах bytes у `CMemStore`):

1) `uint32 sizeof(TDataCam)` + raw bytes `TDataCam`  
2) `uint32 sizeof(TDataFrame)` + raw bytes `TDataFrame`  
3) `uint32 sizeof(TOptionsMeasurement)` + `uint32 count` + `count * raw bytes(TOptionsMeasurement)`  
4) `serialize_frame_calibration_data(calib_frame)` (див. відповідну картку)

`deserialize_dp1_res` робить size-check на кожному кроці і повертає `false` при mismatch.

## Interpretation
Формат спеціально містить `sizeof(...)` для базових структур, щоб ловити ABI mismatch на ранньому етапі (лог + fail-fast) замість “тихого псування” даних.

## Failure cases
- `sizeof(TDataCam)` або `sizeof(TDataFrame)` відрізняються між DP1 і DP2 (packing, компілятор, різні поля) -> десеріалізація відмовляє.
- `sizeof(TOptionsMeasurement)` mismatch -> десеріалізація відмовляє.
- `count` занадто великий або даних менше -> перевірка `need_size > getCurrFreeLimit()` -> fail.
- Далі (calib_frame) може зламатися через UB з порожніми векторами (див. `serialize_frame_calibration_data`).

## Typical misuse
- Додавати поле в `TOptionsMeasurement` і забути, що це змінить `sizeof`.
- Серіалізувати непроініціалізовані поля POD-структур (вийде “сміття” в payload).

## Connections
- container: dp1.types.TDataRes
- depends_on: dp1.rpc.serialize_frame_calibration_data
- produced_by: dp1.net.send_res_to_dp2
- alternate_storage: dp1.io.save_res_blob / dp1.io.save_res_json (файлові формати, не RPC)
