---
id: dp1.rpc.serialize_frame_calibration_data
title:
  uk: "serialize_frame_calibration_data - payload калібрування кадра (R,t,rvec, features)"
  en: "serialize_frame_calibration_data - per-frame calibration payload (R,t,rvec, features)"
tags: [dp1, datapro1, rpc, calibration, serialization]
source:
  file: "datapro1/src/dp1_rpc_data_mrsh.cpp"
  lines: "59-87"
status: "draft"
---

## Definition
Серiалiзацiя/десерiалiзацiя `TDataCalibrationFrame` (кадрова калібровка/поза) для DP1 -> DP2.

## Assumptions
- Матриці `R_matrix`, `t_vec`, `r_vec`, `descriptors_scene` - це `cv::Mat` у форматі, сумісному з `serialize_Mat`.
- Вектори `chessboard_corners` і `keypoints_scene` є непорожні або код не звернеться до `[0]` (див. failure cases).

## Theorem / Contract
Payload-порядок:
1) `R_matrix`, `t_vec`, `r_vec`, `descriptors_scene` як `cv::Mat`,
2) `uint32 size(chessboard_corners)` + raw bytes масиву `cv::Point2f[size]`,
3) `uint32 size(keypoints_scene)` + raw bytes масиву `cv::KeyPoint[size]`.

## Interpretation
Це “додаткова геометрія кадра” (поза/ознаки), яку DP2 може використовувати для стерео/узгодження та наступних етапів.

## Failure cases
- Якщо `chessboard_corners.size() == 0`, виклик `&vec[0]` в Serialize є UB -> можливий crash.
- Аналогічно для `keypoints_scene` при size==0.
- `cv::KeyPoint` - складна структура; raw-copy припускає ABI сумісність (та сама версія OpenCV/компілятора/packing).

## Typical misuse
- Серіалізувати “порожні” вектори без спец-обробки (треба умовно не писати `&vec[0]` коли size==0).
- Змінювати тип feature keypoints (або версію OpenCV) без контролю сумісності.

## Connections
- used_by: dp1.rpc.serialize_dp1_res
- relies_on: dp1.rpc.serialize_Mat
- container: dp1.types.TDataCalibrationFrame
