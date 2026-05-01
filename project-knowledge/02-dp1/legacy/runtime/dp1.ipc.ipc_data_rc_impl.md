---
id: dp1.ipc.ipc_data_rc_impl
title:
  uk: "ipc_data_rc_impl - реалізація IPC receiver з фоновим thread"
  en: "ipc_data_rc_impl - IPC receiver implementation with background thread"
tags: [dp1, ipc, runtime, queue, shared-memory]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_ipc.cpp"
  lines: "20-252"
status: "draft"
---

## Definition
`ipc_data_rc_impl` реалізує `ipc_data_rc`: відкриває POSIX shared memory і message queue, запускає receiving thread (`rc_data_thread_func`), читає нотифікації кадрів і копіює frame bytes у локальну пам'ять для callback.

## Assumptions
- Upstream камера/producer публікує валідні `ipc_next_frame_notify` повідомлення.
- `need_prg_stop_()` може бути викликаний під час open/retry/read loop.
- Queue повідомлення відповідають очікуваному розміру notify struct.

## Theorem / Contract
- `receive_data(...)` дозволений лише один раз на інстанс.
- На кожну валідну queue-нотифікацію формується `ipc_rc_data_store` з копією кадру і викликається notifier.
- При зміні `sh_mem_size` виконується remap shared memory.

## Interpretation
Це низькорівневий ingest boundary між IPC transport (queue + shmem) і DP1 runtime.

## Failure cases
- Невалідні/старі повідомлення queue дають пропуски або помилки копіювання.
- Помилки mmap/munmap або mq API можуть зупинити receiving path.

## Typical misuse
- Викликати `receive_data` повторно на тому ж receiver.
- Ігнорувати stop-path і життєвий цикл receiving thread.

## Open questions
- Чи потрібен explicit backpressure/overflow policy при високому frame rate.

## Connections
- implements: dp1.ipc.ipc_data_rc
- used_by: dp1.runtime.run_ipc_src
- produces: dp1.runtime.rc_ipc_raw_frame
