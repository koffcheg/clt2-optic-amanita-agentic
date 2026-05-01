---
id: dp1.types.TDataCalibrationCamera
title:
  uk: "TDataCalibrationCamera - калібрування камери"
  en: "TDataCalibrationCamera - camera calibration"
tags: [dp1, struct, calibration, opencv]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "24-29"
status: "draft"
---

## Definition
Калібрувальні параметри камери для перетворень “пікселі ↔ промені/простір” та для задач, де потрібні внутрішні параметри (intrinsics).

**Поля:**
- `cameraMatrix` (3×3, double): матриця внутрішніх параметрів K.
- `distCoeffs` (1×5, double): коефіцієнти дисторсії (типово k1,k2,p1,p2,k3).
- `avg_reprojection_error` (double): середня похибка репроєкції при калібруванні.
- `square_size` (float): розмір квадрата калібрувальної мішені (одиниці задаються доменом).
- `board_width`, `board_height` (int): розмір шахматної дошки (кількість внутрішніх кутів).

## Assumptions
- `cameraMatrix` та `distCoeffs` відповідають реальній оптичній схемі і **цьому** `cam_index`.
- Порядок коефіцієнтів дисторсії відповідає OpenCV-конвенції для використаних функцій.
- Одиниці `square_size` узгоджені з іншими розрахунками (м/мм).

## Theorem / Contract
- Якщо `avg_reprojection_error` “прийнятний” (доменно визначений поріг), то перетворення з цією калібровкою дає стабільніші оцінки положення/геометрії об’єктів.
- `cameraMatrix` має бути невиродженою; `fx, fy > 0`.

## Interpretation
Це те, що дозволяє DP1/DP2 робити метричні оцінки та позніші геометричні перетворення (напр. у бінокулярному режимі).

## Failure cases
- Невірні intrinsics → систематичні помилки у координатах.
- `distCoeffs` не підходить під модель (потрібні 8/12 параметрів) → артефакти після undistort.

## Typical misuse
- Застосовувати калібровку іншої камери (переплутаний `cam_index`).
- Ігнорувати одиниці `square_size`.

## Connections
- used_by: dp1.methods.read_camera_settings (див. dataproCameraCalibration.h)
- part_of: dp1.frame_processing (як вхід до get_fr_processor)
- paired_with: dp1.types.TDataCalibrationFrame (позові оцінки)
