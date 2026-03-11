---
id: dp1.ipc.ipc_data_rc
title:
  uk: "ipc_data_rc - інтерфейс отримання кадрів через IPC"
  en: "ipc_data_rc - IPC frame receiver interface"
tags: [dp1, class, interface, ipc]
source:
  file: "datapro1/src/dp1_ipc.h"
  lines: "7-20"
status: "draft"
---

## Definition
Абстрактний receiver, який запускає окремий потік приймання даних і викликає callback при надходженні кадра.

Метод:
`receive_data(callback(ipc_rc_data_store frame, size_t frame_size, size_t frame_ipc_index, ipc_time_point ipc_start_time))`

## Assumptions
- Callback викликається **в receiving thread** (не в main thread).
- `ipc_rc_data_store` володіє буфером кадра (RAII), а його `get()` можна інтерпретувати як `cam_pro::Frame*` (див. rc_ipc_raw_frame).
- Типи `ipc_rc_data_store` та `ipc_time_point` визначені в `m_ipc_def.h` (зовнішня залежність, не в цьому архіві).

## Theorem / Contract
- Receiver повинен гарантувати: один callback = один кадр.
- Caller (runner) має зробити thread-safe handoff (mutex/cv/queue), бо callback в іншому потоці.

## Interpretation
Це “вхідний адаптер” для live-камери/IPC-джерела.

## Failure cases
- Виклик callback після stop → гонки при завершенні.
- Handoff без move/без копії → use-after-free.

## Typical misuse
- Викликати важкі обчислення прямо в callback receiving thread.
- Зберігати `ipc_rc_data_store` десь глобально без контрольованого ownership.

## Connections
- implemented_by: dp1.ipc.ipc_data_rc_impl (dp1_ipc.cpp)
- used_by: dp1.runner.run_ipc_src
- produces: dp1.runtime.rc_ipc_raw_frame
