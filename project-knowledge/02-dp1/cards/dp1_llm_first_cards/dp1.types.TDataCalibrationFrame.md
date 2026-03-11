---
id: dp1.types.TDataCalibrationFrame
title:
  uk: "TDataCalibrationFrame - калібрування/поза на кадрі"
  en: "TDataCalibrationFrame - per-frame calibration/pose"
tags: [dp1, struct, calibration, pose, opencv]
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "31-36"
status: "draft"
---

## Definition
Калібрувальні/позові дані, обчислені **для конкретного кадра** (на відміну від параметрів камери, які сталі).

**Поля:**
- `R_matrix` (double): матриця обертання (типово 3×3).
- `t_vec` (double): вектор переносу (типово 3×1).
- `r_vec` (double): вектор Родріґеса.
- `descriptors_scene`: дескриптори ознак сцени (для matching).
- `chessboard_corners`: знайдені кути шахматної дошки (2D).
- `keypoints_scene`: ключові точки сцени.

## Assumptions
- Розміри `R_matrix/t_vec/r_vec` можуть стартувати як (1×1) і мають бути заповнені валідними розмірами перед використанням.
- Якщо поза не знайдена, структура може містити “дефолти” → потрібно мати зовнішній прапорець валідності (або домовленість у коді).

## Theorem / Contract
- Якщо `R_matrix` і `t_vec` валідні, то вони задають перетворення “мішень/сцена → камера” у прийнятій у проєкті конвенції.
- `r_vec` і `R_matrix` мають бути взаємоузгодженими (Rodrigues(r_vec) == R_matrix).

## Interpretation
Це “поза кадра” або “допоміжні дані для оцінки пози/калібрування”, які можна передавати в DP2 разом із вимірами.

## Failure cases
- Невірна конвенція осей/напрямків у R,t → дзеркальні/інвертовані пози.
- Частково заповнені Mat (1×1) сприймаються як валідні → аварії або тихі помилки.

## Typical misuse
- Використати `R_matrix` без перевірки, що поза була обчислена на цьому кадрі.
- Змішати `r_vec` (Rodrigues) з Euler-кути без конвертації.

## Connections
- part_of: dp1.types.TDataRes
- computed_by: dp1.methods.poseEstimationFromCoplanarPoints (dataproCameraCalibration.h)
- depends_on: dp1.types.TDataCalibrationCamera
