---
id: dp1.runtime.dp1_main_orchestration
title:
  uk: "dp1_main - orchestrator запуску DP1"
  en: "dp1_main - DP1 startup orchestrator"
tags: [dp1, runtime, main, orchestration]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_main.cpp"
  lines: "59-120"
status: "draft"
---

## Definition
`main` у DP1 виконує bootstrap runtime: парсить аргументи, ініціалізує logging/config/calibration/signal/network-to-DP2, обирає source runner (`run_ipc_src` або `run_uri_src`) і керує graceful stop.

## Assumptions
- Конфігураційні файли логування та програми доступні за переданими або дефолтними шляхами.
- `init_signal` коректно викликає `on_program_stop_request` при SIG-stop подіях.

## Theorem / Contract
- Перед стартом runner-ів ініціалізуються logger-канали та з'єднання DP1 -> DP2.
- Вибір runner-а робиться через `cfg.get_frame_src_type()`.
- Після завершення runner-а `program_stop = true`, далі short wait для завершення фонових thread.

## Interpretation
Це top-level orchestration contract модуля DP1, який зшиває runtime компоненти в єдиний execution path.

## Failure cases
- Помилки конфігурації/ініціалізації дають раннє завершення процесу.
- Невалідний source type завершує роботу з помилкою.

## Typical misuse
- Запускати процес без валідного `cam_index`/config та очікувати fallback behavior.
- Ігнорувати stop-сигнал і покладатися лише на аварійне завершення.

## Open questions
- Чи потрібен розділений shutdown timeout для різних runtime threads.

## Connections
- uses: dp1.runtime.run_ipc_src
- uses: dp1.runtime.run_uri_src
- uses: dp1.net.dp1_tr_res2dp2_connection
- uses: dp1.ipc.ipc_data_rc_impl
