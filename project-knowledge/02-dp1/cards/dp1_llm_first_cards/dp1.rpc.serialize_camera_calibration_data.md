---
id: dp1.rpc.serialize_camera_calibration_data
title:
  uk: "serialize_camera_calibration_data - payload калібрування камери (K, distCoeffs)"
  en: "serialize_camera_calibration_data - camera calibration payload (K, distCoeffs)"
tags: [dp1, datapro1, rpc, calibration, serialization]
source:
  file: "datapro1/src/dp1_rpc_data_mrsh.cpp"
  lines: "39-57"
status: "draft"
---

## Definition
Серiалiзацiя/десерiалiзацiя `TDataCalibrationCamera` для передачі DP1 -> DP2: матриця камери, дисторсія та параметри chessboard.

## Assumptions
- `cameraMatrix` і `distCoeffs` записуються функцією `serialize_Mat`, тобто формат `cv::Mat` є канонічним для RPC.
- Параметри `avg_reprojection_error`, `square_size`, `board_width`, `board_height` мають однакові типи й порядок в DP1/DP2.

## Theorem / Contract
Порядок полів у payload:
1) `cameraMatrix` (cv::Mat),
2) `distCoeffs` (cv::Mat),
3) `avg_reprojection_error`,
4) `square_size`,
5) `board_width`,
6) `board_height`.

Десеріалізація повинна читати рівно в такому ж порядку.

## Interpretation
Це “глобальні налаштування камери”, які DP1 відправляє DP2 окремим повідомленням одразу після конекту.

## Failure cases
- Розбіжність типів/порядку полів між DP1 і DP2 -> зсув курсора в `CMemStore` і “каскадне” псування всього потоку.
- Якщо `cameraMatrix`/`distCoeffs` порожні або мають неочікуваний тип -> DP2 може отримати некоректну модель камери.

## Typical misuse
- Вважати, що `distCoeffs` завжди фіксованої довжини (в OpenCV залежить від моделі).
- Змінювати `TDataCalibrationCamera` без оновлення wire-format (і без версіонування).

## Connections
- used_by: dp1.net.tr_camera_settings (DP1 -> DP2 повідомлення)
- relies_on: dp1.rpc.serialize_Mat
- container: dp1.types.TDataCalibrationCamera
