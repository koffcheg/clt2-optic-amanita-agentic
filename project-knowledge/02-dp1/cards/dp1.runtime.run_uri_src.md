---
id: dp1.runtime.run_uri_src
title:
  uk: "run_uri_src - orchestration циклу обробки URI-джерела"
  en: "run_uri_src - URI source processing orchestration loop"
tags: [dp1, runtime, uri, runner]
source:
  file: "datapro1/src/dp1_uri_runner.cpp"
  lines: "25-71"
status: "draft"
---

## Definition
`run_uri_src` реалізує runtime-цикл DP1 для URI/VideoCapture-джерела: читає кадри, нормалізує формат (BGR -> GRAY -> CV_16UC1), обмежує буфер і передає кадри в `frame_processor`.

## Assumptions
- `choice_source` коректно відкриває джерело кадрів у `cv::VideoCapture`.
- Формат CV_16UC1 очікується downstream логікою DP1.
- Для URI-джерела `cam_pro_header` відсутній (`nullptr` у `frame_n_header`).

## Theorem / Contract
- Якщо джерело не відкривається або повертає порожній кадр - функція завершується з `-1`.
- `frame_processor` створюється один раз на першому кадрі і далі реюзається.
- У `proc_next_frame` передається сурогатний timestamp `steady_clock::now()`.

## Interpretation
Це альтернативний runner path для offline/debug/stream ingest без IPC metadata header.

## Failure cases
- Невірний format conversion або тип матриці порушує контракт із DP1 pipeline.
- Розрив джерела кадрів завершує обробку без автоматичного reconnect.

## Typical misuse
- Вважати `frame_index` еквівалентом абсолютного індексу оригінального відео.
- Порівнювати URI timestamps із IPC timestamps як однорідні.

## Open questions
- Чи потрібен reconnect/reopen policy для нестабільних URI stream-джерел.

## Connections
- uses: dp1.runtime.rc_uri_raw_frame
- uses: dp1.frame.frame_processor
- related_to: dp1.runtime.dp1_main_orchestration
