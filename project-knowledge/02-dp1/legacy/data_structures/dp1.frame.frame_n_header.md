---
id: dp1.frame.frame_n_header
title:
  uk: "frame_n_header - вхід кадра у frame_processor"
  en: "frame_n_header - frame input to frame_processor"
tags: [dp1, struct, frame, api]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_frame_proc.h"
  lines: "14-19"
status: "draft"
---

## Definition
Легковаговий контейнер-посилання, який передається у `frame_processor::proc_next_frame` і містить:
- вказівник на `cv::Mat` кадра,
- (опційно) вказівник на `cam_pro::FrameHeader`,
- часову мітку IPC (`ipc_start_time`).

## Assumptions
- `mat` вказує на валідний `cv::Mat`, який живе щонайменше до завершення `proc_next_frame`.
- `cam_pro_header` може бути `nullptr` (наприклад, URI source).
- `ipc_start_time` має сенс лише для IPC-джерела (або завжди заповнюється “now” як surrogate).

## Theorem / Contract
- `proc_next_frame` не має зберігати `mat*` чи `cam_pro_header*` “на потім” без копії/ownership-протоколу.
- Якщо `cam_pro_header != nullptr`, то `width/height/…` беруться саме з нього як authoritative.

## Interpretation
Це “API boundary” між джерелом кадрів (IPC/URI) і основним обробником кадра.

## Failure cases
- dangling pointer: `mat` посилається на тимчасовий буфер з deque, який pop_back/pop_front знищив.
- `cam_pro_header` використано без nullptr-check.

## Typical misuse
- Кешувати `cv::Mat*` у класі processor без `clone()` і без гарантій lifetime.
- Припускати, що `ipc_start_time` - це system_clock time (це може бути steady_clock).

## Connections
- consumed_by: dp1.frame.frame_processor
- produced_by: dp1.runtime.rc_ipc_raw_frame, dp1.runtime.rc_uri_raw_frame
- complements: dp1.types.TDataFrame (більш повні метадані)
