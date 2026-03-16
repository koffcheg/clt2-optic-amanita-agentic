---
id: dp1.runtime.rc_uri_raw_frame
title:
  uk: "rc_uri_raw_frame - елемент буфера кадрів URI"
  en: "rc_uri_raw_frame - URI frame buffer entry"
tags: [dp1, struct, runtime, buffer, uri]
source:
  file: "datapro1/src/dp1_uri_runner.cpp"
  lines: "10-15 (локальна структура в .cpp)"
status: "draft"
---

## Definition
Локальна структура буфера для кадрів, які читаються через `cv::VideoCapture` (файл/камера/стрім).

**Поля:**
- `frame_size` (може не використовуватись),
- `frame_index`,
- `cv::Mat mat`,
- `cam_pro::FrameHeader cam_pro_header` (локальний; може бути default/не заповнений).

## Assumptions
- Runner може конвертувати BGR → GRAY → CV_16UC1 (див. dp1_uri_runner.cpp) для уніфікації з пайплайном DP1.
- `cam_pro_header` часто не заповнюється (викликається processor з nullptr).

## Theorem / Contract
- `mat` містить копію кадра (через `frame.copyTo(just_rc_frames.mat)`), тому lifetime менш ризикований, ніж в IPC-випадку.

## Interpretation
Це “адаптер” для тестування/відтворення пайплайна DP1 на файлах.

## Failure cases
- Зміна типу кадра (не CV_16UC1) без корекції downstream → некоректні фільтри/маски.
- Втрата метаданих кадра (header==nullptr) → downstream не може відтворити frame metadata.

## Typical misuse
- Забути, що тут `ipc_start_time` сурогатний (`steady_clock::now()`).
- Вважати, що `frame_index` відповідає original frame number у відео (може бути пропуски).

## Connections
- used_by: dp1.runner.run_uri_src
- produces: dp1.frame.frame_n_header (cam_pro_header = nullptr)
