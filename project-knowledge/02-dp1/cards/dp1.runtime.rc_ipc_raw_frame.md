---
id: dp1.runtime.rc_ipc_raw_frame
title:
  uk: "rc_ipc_raw_frame - елемент буфера кадрів IPC"
  en: "rc_ipc_raw_frame - IPC frame buffer entry"
tags: [dp1, struct, runtime, buffer, ipc]
source:
  file: "datapro1/src/dp1_ipc_runner.cpp"
  lines: "12-19 (локальна структура в .cpp)"
status: "draft"
---

## Definition
Локальна (translation-unit) структура, яку `dp1_ipc_runner.cpp` використовує як елемент черги `deque<rc_ipc_raw_frame>`.

**Поля (ключові):**
- `ipc_rc_data_store frame`: RAII-буфер отриманого кадра (володіння даними).
- `frame_size`, `frame_index`: метадані кадра з IPC.
- `ipc_start_time`: таймстамп отримання.
- `cv::Mat mat`: view на дані кадра через `cam_pro_frame->asMat()`.
- `cam_pro::Frame* cam_pro_frame`: pointer на дані всередині `frame`.

## Assumptions
- `cam_pro_frame` валідний, поки живе `frame` (ipc_rc_data_store).
- `mat` не володіє даними, а лише посилається на буфер `cam_pro_frame`.

## Theorem / Contract
- Переміщення (`std::move`) rc_ipc_raw_frame між змінними не повинно ламати мат (але треба бути уважним з OpenCV shallow copy).
- Handoff у deque зберігає lifetime даних до моменту pop_back.

## Interpretation
Це “спакований кадр”: байти + зручний Mat + header.

## Failure cases
- Pop з deque до завершення processing → dangling `cv::Mat` в processor.
- Некоректний cast `reinterpret_cast<cam_pro::Frame*>` якщо IPC payload не відповідає очікуваному формату.

## Typical misuse
- Клонувати `mat` без необхідності (дорого) або, навпаки, зберігати view після звільнення.
- Ігнорувати `cfg.num_frame_to_keep` і роздувати чергу.

## Connections
- produced_by: dp1.ipc.ipc_data_rc callback
- consumed_by: dp1.frame.frame_processor via dp1.frame.frame_n_header
