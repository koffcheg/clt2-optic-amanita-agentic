---
id: dp1.io.save_res_blob
title:
  uk: "save_res (.blob) - файловий формат збереження результатів DP1"
  en: "save_res (.blob) - DP1 result file format"
tags: [dp1, datapro1, io, blob, file-format]
source:
  file: "datapro1/src/datapro1.cpp; datapro1/src/dataproSaveToFile.cpp"
  lines: "datapro1.cpp:371-505; dataproSaveToFile.cpp:31-96"
status: "draft"
---

## Definition
`.blob` - бінарний файл результатів DP1. Існують два режими:
- **rolling** (append у файл з інтервалом по часу),
- **single-frame** (один файл на кадр з заданим `file_name`).

Формат визначається тим, у якому порядку викликаються `save_data_*` та що додається для binocular.

## Assumptions
- `TDataCam`, `TDataFrame`, `TOptionsMeasurement` записуються як raw bytes (POD).
- Для `cv::Mat` у файлі використовується `save_Mat_to_bin`: заголовок (4 int) + raw bytes `tmp.data` (clone).

## Theorem / Contract (single-frame варіант)
Порядок запису у файл `<file_name>.blob`:
1) `TDataCam` (raw, sizeof)
2) `TDataFrame` (raw, sizeof)
3) `size_t meas_count` + `meas_count * TOptionsMeasurement` (або 0)
4) `int check_binocular` (1 або 0)
5) Якщо `check_binocular == 1`:
   - camera calibration (`cameraMatrix`, `distCoeffs`, scalars)
   - frame calibration (`R_matrix`, `t_vec`, `r_vec`, `descriptors_scene`, vectors)

Rolling-варіант: при першому записі додає `TDataCam`, далі у файл можуть додаватися лише `TDataFrame + meas`.

## Interpretation
`.blob` - “офлайн артефакт” для відлагодження/архівації та відтворення результатів без мережі.

## Failure cases
- `size_t` залежить від платформи (32/64) -> blob може бути непереносимий між різними архітектурами.
- Якщо `TData*` не POD або мають padding/packing різний між збірками -> несумісність.
- Rolling режим: файл змішує багато кадрів без явного фреймінгу “початок запису кадра” (окрім фіксованого sizeof структур) -> складніший парсинг.

## Typical misuse
- Вважати `.blob` portable між Windows/Linux без контролю packing/endianness.
- Писати `cv::Mat` без clone/continuous гарантії (у реалізації це вже враховано через `clone()`).

## Connections
- naming: dp1.io.dp1_output_filenames
- json_mirror: dp1.io.save_res_json
- overlaps_with: dp1.rpc.serialize_dp1_res (але це інший формат!)
