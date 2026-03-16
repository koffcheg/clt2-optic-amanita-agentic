---
id: dp1.io.save_res_json
title:
  uk: "seva_res_json (.json) - схема JSON-виводу результатів DP1"
  en: "seva_res_json (.json) - DP1 JSON output schema"
tags: [dp1, datapro1, io, json, schema]
source:
  file: "datapro1/src/datapro1.cpp"
  lines: "400-505"
status: "draft"
---

## Definition
JSON “вітрина” результатів DP1 (читабельний артефакт), який записується поруч із `.blob`, якщо `txt_file == true`.

## Assumptions
- JSON створюється через Jansson (`json_object_set_new`, `json_dump_file`).
- Час `exposureStart` серіалізується у строку через `timePointToString(...)` з форматом `"%Z %Y-%m-%d %H:%M:%S."`.

## Theorem / Contract (ключі та структура)
Кореневий JSON містить секції:
- `"Camera data"`: `cam_index`, `Xcam`, `Ycam`, `Zcam`, `pixel_width`, `pixel_height`, `focal_length`
- `"Turret data"`: `Az`, `El`, `V_az`, `V_el`
- `"Data frame"`: `index_frame`, `exposureStart`, `exposureLength`, `width`, `height`
- `"Measurements frame"`:
  - `"Size"`: кількість вимірів
  - `"Measurements"`: масив об’єктів з полями `id_obj`, `num_pix_obj`, `x_weight`, `y_weight`, `x_rec`, `y_rec`, `rec_height`, `rec_width`, `obj_angel`, `obj_angel_moment`, `obj_eccentricity`, `mean_brightness_obj`, `std_brightness_obj`
- `"Frame calibration"`: `"R matrix"` (9 значень), `"t vector"` (3 значення)

## Interpretation
JSON тут не “канонічний” формат для DP2, а швидше:
- debug/readability,
- легкий імпорт у аналітику,
- перевірка регресій без парсингу blob.

## Failure cases
- Відсутні/порожні матриці `R_matrix` або `t_vec` -> звернення `.at<double>` може кинути exception/UB (залежно від OpenCV).
- Ключі містять пробіли (`"Camera data"`) - це ок для JSON, але незручно для деяких інструментів.

## Typical misuse
- Спиратися на JSON як на “точний формат обміну” замість RPC.
- Парсити `"exposureStart"` як UTC без перевірки timezone у рядку.

## Connections
- paired_with: dp1.io.save_res_blob
- source_structs: dp1.types.TDataCam, dp1.types.TDataFrame, dp1.types.TOptionsMeasurement, dp1.types.TDataCalibrationFrame
