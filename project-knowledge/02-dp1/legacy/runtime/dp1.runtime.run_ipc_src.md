---
id: dp1.runtime.run_ipc_src
title:
  uk: "run_ipc_src - orchestration циклу обробки IPC-кадрів"
  en: "run_ipc_src - IPC frame processing orchestration loop"
tags: [dp1, runtime, ipc, runner]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_ipc_runner.cpp"
  lines: "48-89"
status: "draft"
---

## Definition
`run_ipc_src` запускає runtime-цикл DP1 для IPC-джерела: ініціалізує receiver, чекає кадри через condition variable, керує bounded буфером кадрів і передає їх у `frame_processor`.

## Assumptions
- `need_stop()` є потокобезпечним і може викликатися часто з main loop.
- Callback `on_rc_next_frame` пише в shared state під mutex.
- `cfg.num_frame_to_keep` задає верхню межу deque-буфера.

## Theorem / Contract
- Receiver запускається один раз: `receiver->receive_data(on_rc_next_frame)`.
- `frame_processor` створюється ліниво при першому валідному кадрі (розмір кадра береться з header IPC).
- Кожен оброблений кадр передається в `proc_next_frame` з `cam_pro::FrameHeader` та `ipc_start_time`.

## Interpretation
Це головний orchestration boundary DP1 для live IPC ingest між receiving thread і processing thread.

## Failure cases
- Тривала відсутність кадрів дає тайм-аути очікування і може приховувати проблеми upstream.
- Некоректний lifetime буфера кадрів при помилках handoff може призвести до dangling `cv::Mat`.

## Typical misuse
- Збільшувати `num_frame_to_keep` без контролю latency/memory.
- Припускати, що callback виконується в main thread.

## Open questions
- Чи потрібна явна метрика backlog/lag для queue health у production.

## Connections
- uses: dp1.ipc.ipc_data_rc
- uses: dp1.runtime.rc_ipc_raw_frame
- uses: dp1.frame.frame_processor
- related_to: dp1.runtime.dp1_main_orchestration
